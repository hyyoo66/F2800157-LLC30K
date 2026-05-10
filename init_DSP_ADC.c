#include "init_DSP_ADC.h"
#include "main_define.h"

// ADC 결과 저장 전역 변수
uint16_t gu_DCPT1_A1;
uint16_t gu_DCPT2_A2;
uint16_t gu_DCCT1_A3;
uint16_t gu_DCCT2_A6;


void initADCSOC(void)
{
    // 샘플링 시간: 14 SYSCLK (약 117ns @120MHz)
    uint16_t acqps = 14;

#define _ADC_TRIGGER_SOC    ADC_TRIGGER_EPWM1_SOCA

    // === ADCA 설정 ===
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, _ADC_TRIGGER_SOC, ADC_CH_ADCIN1, acqps);  // DCPT1_A1
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, _ADC_TRIGGER_SOC, ADC_CH_ADCIN2, acqps);  // DCPT2_A2
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER3, _ADC_TRIGGER_SOC, ADC_CH_ADCIN3, acqps);  // DCCT1_A3
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER6, _ADC_TRIGGER_SOC, ADC_CH_ADCIN6, acqps);  // DCCT2_A6

    // ADCA 인터럽트: SOC6 완료 시점
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER6);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
}

extern __interrupt void adcA1ISR(void);  // defined in main.c

 void setupADCAInterrupt(void)
 {
     Interrupt_register(INT_ADCA1, &adcA1ISR);  // ISR 연결
     Interrupt_enable(INT_ADCA1);               // PIE에서 허용
     Interrupt_enableMaster();                  // CPU 전체 인터럽트 허용
 }

 void initADC(void)
 {
     // ADCA, ADCB 클럭 분주 설정 (e.g., SYSCLK/4)
     ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);

     // 기준전압 설정 + OTP offset trim 로드
     ADC_setVREF(ADCA_BASE, ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
     ADC_setOffsetTrim(ADCA_BASE);

     // 인터럽트 발생 위치: 샘플 완료 후
     ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);

     // 전원 켜기
     ADC_enableConverter(ADCA_BASE);

     DEVICE_DELAY_US(1000);  // 안정화 대기
 }



