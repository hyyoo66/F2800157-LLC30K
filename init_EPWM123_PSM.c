#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"
#include "device.h"

#ifndef EPWM_TBCTL_PHSDIR
#define EPWM_TBCTL_PHSDIR   (1U << 13)
#endif

// =============================
// 사용자 설정
// =============================
#define DEAD_TICKS   18 //150nsec

extern uint16_t gu_Per_ePWM1;

//==================================================
// GPIO
//==================================================
void initEPWMGpio_123(void)
{
    uint16_t pins[] = {0,1,2,3,4,5};
    int i;

    for(i = 0; i < 6; i++)
    {
        GPIO_setPadConfig(pins[i], GPIO_PIN_TYPE_STD);
        GPIO_setDirectionMode(pins[i], GPIO_DIR_MODE_OUT);
    }

    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setPinConfig(GPIO_4_EPWM3_A);
    GPIO_setPinConfig(GPIO_5_EPWM3_B);
}


//==================================================
// ePWM 1개 설정
// - UP-DOWN
// - A/B 완전 보완 50%
// - phase는 TBPHS + PHSDIR로만 처리
//==================================================
void init_EPWMx_PSM(uint32_t base,
                    uint16_t period,
                    uint16_t tbphs,
                    bool phsdir,
                    bool is_master)
{
    EPWM_setClockPrescaler(base,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);

    EPWM_setTimeBasePeriod(base, period);
    EPWM_setTimeBaseCounter(base, 0);
    EPWM_setTimeBaseCounterMode(base, EPWM_COUNTER_MODE_UP_DOWN);

    if(is_master)
    {
        EPWM_disablePhaseShiftLoad(base);

        EPWM_enableSyncOutPulseSource(base,
            EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
    }
    else
    {
        EPWM_setPhaseShift(base, tbphs);
        EPWM_enablePhaseShiftLoad(base);

        EPWM_setSyncInPulseSource(base,
            EPWM_SYNC_IN_PULSE_SRC_SYNCOUT_EPWM1);

        if(phsdir)
            HWREGH(base + EPWM_O_TBCTL) |= EPWM_TBCTL_PHSDIR;
        else
            HWREGH(base + EPWM_O_TBCTL) &= ~EPWM_TBCTL_PHSDIR;
    }

    EPWM_setCounterCompareShadowLoadMode(base,
        EPWM_COUNTER_COMPARE_A,
        EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);

    EPWM_setCounterCompareShadowLoadMode(base,
        EPWM_COUNTER_COMPARE_B,
        EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);

    EPWM_setCounterCompareValue(base,
        EPWM_COUNTER_COMPARE_A,
        DEAD_TICKS);

    EPWM_setCounterCompareValue(base,
        EPWM_COUNTER_COMPARE_B,
        period - DEAD_TICKS);

    // -------------------------------------------------
    // A 출력: 기준 50%
    // ZERO   -> HIGH
    // PERIOD -> LOW
    // -------------------------------------------------
    EPWM_setActionQualifierAction(base,
        EPWM_AQ_OUTPUT_A,
        EPWM_AQ_OUTPUT_HIGH,
        EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

    EPWM_setActionQualifierAction(base,
        EPWM_AQ_OUTPUT_A,
        EPWM_AQ_OUTPUT_LOW,
        EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);

    // -------------------------------------------------
    // B 출력: 50% 유지 + modindex로 phase shift
    //
    // UP_CMPA   -> HIGH
    // DOWN_CMPB -> LOW
    //
    // CMPA = shift
    // CMPB = period - shift
    // -------------------------------------------------
    EPWM_setActionQualifierAction(base,
        EPWM_AQ_OUTPUT_B,
        EPWM_AQ_OUTPUT_HIGH,
        EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    EPWM_setActionQualifierAction(base,
        EPWM_AQ_OUTPUT_B,
        EPWM_AQ_OUTPUT_LOW,
        EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);

    // Dead-band OFF
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, false);
    EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, false);
}


//==================================================
// 최초 초기화
//==================================================
void EPWM123_init(float per_usec)
{
    uint16_t period;
    uint16_t ph120;

    // UP-DOWN 모드: 실제 주기 = 2 × TBPRD
    // g_Per_usec = 원하는 실제 주기
    // TBPRD = g_Per_usec × 60 (120 ÷ 2)
    period = (uint16_t)(per_usec * 60.0f);
    gu_Per_ePWM1 = period;

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM2);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM3);

    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    initEPWMGpio_123();

    // UP-DOWN 기준 120도
    ph120 = (uint16_t)(((uint32_t)period * 2U) / 3U);

    // ePWM1 = 0도
    init_EPWMx_PSM(EPWM1_BASE,
                   period,
                   0,
                   false,
                   true);

    // ePWM2 = 120도
    init_EPWMx_PSM(EPWM2_BASE,
                   period,
                   ph120,
                   false,
                   false);

    // ePWM3 = 240도
    // ph120 + PHSDIR 반전으로 240도 생성
    init_EPWMx_PSM(EPWM3_BASE,
                   period,
                   ph120,
                   true,
                   false);

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // =========================================
    // ADC Trigger 설정
    // =========================================
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 2);
}


//==================================================
// period 변경용
// modindex는 현재 구조에서는 A/B 대칭을 위해 사용하지 않음
//==================================================
void EPWM123_setModIndex(float per_usec, float modindex)
{
    uint16_t period;
    uint16_t period_old;
    uint16_t ph120;
    uint16_t shift;
    uint16_t cmpA;
    uint16_t cmpB;
    uint32_t timeout;

    // UP-DOWN 모드: 실제 주기 = 2 × TBPRD
    // g_Per_usec = 원하는 실제 주기
    // TBPRD = g_Per_usec × 60 (120 ÷ 2)
    period = (uint16_t)(per_usec * 60.0f);
    period_old = gu_Per_ePWM1;
    gu_Per_ePWM1 = period;

    if(modindex < 0.0f)
        modindex = 0.0f;
    else if(modindex > 1.0f)
        modindex = 1.0f;

    // -------------------------------------------------
    // 큰 주기 변화 감지 시 counter zero 대기
    // 변화량 > 기존값의 15% 또는 절댓값 > 50 카운트
    // -------------------------------------------------
    if( (period_old > 0) &&
        ((period > (period_old * 115U / 100U)) || (period < (period_old * 85U / 100U))) )
    {
        // 큰 변화: TBCTR_ZERO까지 대기 (타임아웃: 1ms = 120,000 사이클)
        timeout = 120000;

        while( (EPWM_getTimeBaseCounterValue(EPWM1_BASE) != 0) && (timeout > 0) )
            timeout--;
    }

    // -------------------------------------------------
    // period 변경
    // -------------------------------------------------
    EPWM_setTimeBasePeriod(EPWM1_BASE, period);
    EPWM_setTimeBasePeriod(EPWM2_BASE, period);
    EPWM_setTimeBasePeriod(EPWM3_BASE, period);

    // -------------------------------------------------
    // 3상 phase 갱신
    // -------------------------------------------------
    ph120 = (uint16_t)(((uint32_t)period * 2U) / 3U);

    EPWM_setPhaseShift(EPWM2_BASE, ph120);
    EPWM_setPhaseShift(EPWM3_BASE, ph120);

    EPWM_enablePhaseShiftLoad(EPWM2_BASE);
    EPWM_enablePhaseShiftLoad(EPWM3_BASE);

    HWREGH(EPWM2_BASE + EPWM_O_TBCTL) &= ~EPWM_TBCTL_PHSDIR;
    HWREGH(EPWM3_BASE + EPWM_O_TBCTL) |=  EPWM_TBCTL_PHSDIR;

    // -------------------------------------------------
    // modindex -> A/B phase shift
    //
    // modindex = 0   : A와 B 거의 동상
    // modindex = 1   : A와 B 거의 180도
    //
    // B는 항상 50%
    // A도 항상 50%
    // -------------------------------------------------
    shift = (uint16_t)((float)period * modindex);

    if(shift < DEAD_TICKS)
        shift = DEAD_TICKS;
    else if(shift > (period - DEAD_TICKS))
        shift = period - DEAD_TICKS;

    cmpA = shift;
    cmpB = period - shift;

    if(cmpB >= period)
        cmpB = period - 1;

    // -------------------------------------------------
    // 모든 상에 같은 local A-B phase 적용
    // 3상 위상은 TBPHS가 이미 담당하므로
    // 여기서 ph120/ph240을 CMP에 더하면 안 됨
    // -------------------------------------------------
    EPWM_setCounterCompareValue(EPWM1_BASE,
        EPWM_COUNTER_COMPARE_A,
        cmpA);

    EPWM_setCounterCompareValue(EPWM1_BASE,
        EPWM_COUNTER_COMPARE_B,
        cmpB);

    EPWM_setCounterCompareValue(EPWM2_BASE,
        EPWM_COUNTER_COMPARE_A,
        cmpA);

    EPWM_setCounterCompareValue(EPWM2_BASE,
        EPWM_COUNTER_COMPARE_B,
        cmpB);

    EPWM_setCounterCompareValue(EPWM3_BASE,
        EPWM_COUNTER_COMPARE_A,
        cmpA);

    EPWM_setCounterCompareValue(EPWM3_BASE,
        EPWM_COUNTER_COMPARE_B,
        cmpB);
}
