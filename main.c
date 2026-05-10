#define _YYMMDD "260510-1"
/*
[변수 이름]
    g_      global floating
    gi_     global integer
    gu_     global unsigned integer
    g_xxx_flag   global floating flag
    xxx_flag   global integer flag

[매크로 정의]
    숫자:     _XXX
    함수:     대문자
    action:  do_XXX
    value:   val_XXX
    HW 단자:   PIN_xxx, XXX_GPIOXX
*/



//=============================================================================
// 1. 사용자 정의 모듈 헤더 (기능별 초기화 함수 등)
//=============================================================================
// f28003x_lin_gpt.h removed — f280015x driverlib 방식에서 불필요
#include "init_EPWM123_PSM.h"
#include <key_print_digit_260507.h>
#include "init_DSP_ADC.h"
#include "init_SCIA_baudrate.h"



//=============================================================================
// 2. TI 제공 드라이버/장치 헤더
//=============================================================================
#include "driverlib.h"
#include "device.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "sw_prioritized_isr_levels.h"
#include <stdio.h>


float g_ModIndex  = 0.35f;
float g_ModIndex_LPF  = 0.0f;  // 초기값 = 0
float g_freq_kHz  = 200.f;   // PWM 주파수 (kHz) - 메인루프에서 매번 계산
float g_Per_usec  = 5.0f;    // PWM 주기 (usec)


// f28003x_device.h, f28003x_analogsubsys.h removed — f280015x driverlib.h로 통합


#define val_MAX(A,B)       ((A)>(B)?(A):(B))
#define val_MIN(A,B)       ((A)<(B)?(A):(B))
#define val_ABS(A)       ((A) >= 0 ? (A) : -(A))


#define PRINT_F_VAR1_nr(var)  print_s("\n\r  " #var " = "); print_f(var)
#define PRINT_F_VAR1(var)  print_s(#var " = "); print_f(var)
#define PRINT_F_VAR2(var)  print_s("\t| " #var " = "); print_f(var)
#define PRINT_I_VAR1(var)  print_s(#var " = "); print_i(var)
#define PRINT_I_VAR2(var)  print_s("\t| " #var " = "); print_i(var)
#define PRINT_I_VAR1_nr(var)  print_s("\n\r  " #var " = "); print_i(var)


uint32_t   gu32_timer1_int_watch_dog_cnt  = 0;  // increase at Timer1 interrupt (50kHz)
uint32_t   gu32_dsp_adc_int_watch_dog_cnt = 0;  // increase at ADC interrupt

#include "main_define.h"

#define  T_con      (20.e-6f)      // 50kHz Timer1 고정 = 20us
#define  T_1msec      (1.e-3f)      // 1kHz

// State 정의
typedef enum {
    _STANDBY = 0,
    _STOP,
    _RUN,
    _TRIP,
} Status_typedef;
Status_typedef gi_State = _STANDBY;

// Trip 정의
typedef enum {
    _NONE = 0,
    _TIMER1_INT_ERROR,
    _DSPADC_INT_ERROR,
    _OVER_CURR_DCCT1,
    _OVER_CURR_DCCT2
} Trip_no_typedef;
Trip_no_typedef gi_Trip_no = _NONE;
// Trip 사유 문자열 배열
#define TRIP_NAME_COUNT (sizeof(trip_names) / sizeof(trip_names[0]))
const char* trip_names[] = {
    " none             ",         // _NONE
    "_TIMER1_INT_ERROR ",         // _TIMER1_INT_ERROR
    "_DSPADC_INT_ERROR ",         // _DSPADC_INT_ERROR
    "_OVER_CURR_DCCT1 ",          // _OVER_CURR_DCCT1
    "_OVER_CURR_DCCT2 "           // _OVER_CURR_DCCT2
};


// DSP_ADC 변수
float g_DCPT1_A1_adc, g_DCPT1_A1, g_DCPT1_A1_LPF = 0;
float g_DCPT2_A2_adc, g_DCPT2_A2, g_DCPT2_A2_LPF = 0;
float g_DCCT1_A3_adc, g_DCCT1_A3, g_DCCT1_A3_LPF = 0;
float g_DCCT2_A6_adc, g_DCCT2_A6, g_DCCT2_A6_LPF = 0;
float g_DCCT1_A3_peak = 0.0f;   // RUN 중 DCCT1 전류 최댓값
float g_DCCT2_A6_peak = 0.0f;   // RUN 중 DCCT2 전류 최댓값

// 과전류 보호 변수
float g_DCCT1_OC_ref = 5.0f;    // DCCT1 과전류 임계값 (A) - 초기값 5A
float g_DCCT2_OC_ref = 50.0f;   // DCCT2 과전류 임계값 (A) - 초기값 50A

// 상태/제어 변수
uint16_t gu_seq = 0;                  // 시퀀스 카운터

// 제어 시간 변수
float g_T_end = 1.0f;                 // 종료 시간 (sec)
float g_T_run = 0;

int g_test_Trip_no = 0;               // 테스트 Trip 번호

// 상태 문자열 배열
const char* state_names[] = {
    "STANDBY",      // _STANDBY = 0
    "STOP   ",         // _STOP = 1
    "RUN    ",          // _RUN = 2
    "TRIP   "          // _TRIP = 3
};

// 상태 판별 헬퍼 함수
//inline bool is_standby(void)      { return gi_State == _STANDBY; }
uint16_t gu_Per_ePWM1 = 0;


#define LED4R_GPIO20     20
#define LED5G_GPIO22     22
#define GEN_GPIO30          30
void init_GPIO(void)
{


    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_setDirectionMode(LED4R_GPIO20, GPIO_DIR_MODE_OUT);

    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_setDirectionMode(LED5G_GPIO22, GPIO_DIR_MODE_OUT);


    // GEN_GPIO30
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setDirectionMode(GEN_GPIO30, GPIO_DIR_MODE_OUT);
    GPIO_writePin(GEN_GPIO30, 0);   //0 = OFF
    /*
    // SCRGATE_active
    GPIO_setPinConfig(GPIO_7_GPIO7);
    GPIO_setDirectionMode(SCRGATE_GPIO7, GPIO_DIR_MODE_OUT);
    GPIO_writePin(SCRGATE_GPIO7, 1);   //1 = OFF

    // DOUT1_GPIO0
    GPIO_setPinConfig(GPIO_0_GPIO0);
    GPIO_setDirectionMode(DOUT1_GPIO0, GPIO_DIR_MODE_OUT);

    // DOUT2_GPIO1
    GPIO_setPinConfig(GPIO_1_GPIO1);
    GPIO_setDirectionMode(DOUT2_GPIO1, GPIO_DIR_MODE_OUT);

    // DOUT3_GPIO2
    GPIO_setPinConfig(GPIO_2_GPIO2);
    GPIO_setDirectionMode(DOUT3_GPIO2, GPIO_DIR_MODE_OUT);

    // DOUT4_GPIO3
    GPIO_setPinConfig(GPIO_3_GPIO3);
    GPIO_setDirectionMode(DOUT4_GPIO3, GPIO_DIR_MODE_OUT);

    // DOUT5_GPIO4
    GPIO_setPinConfig(GPIO_4_GPIO4);
    GPIO_setDirectionMode(DOUT5_GPIO4, GPIO_DIR_MODE_OUT);

    // PT_RELAY_GPIO5
    GPIO_setPinConfig(GPIO_5_GPIO5);
    GPIO_setDirectionMode(PT_RELAY_GPIO5, GPIO_DIR_MODE_OUT);

    // CT_TEST_GPIO23
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_setDirectionMode(CT_TEST_GPIO23, GPIO_DIR_MODE_OUT);

    // PT_TEST_GPIO25
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(PT_TEST_GPIO25, GPIO_DIR_MODE_OUT);

    // CANR1_GPIO9
    GPIO_setPinConfig(GPIO_9_GPIO9);
    GPIO_setDirectionMode(CANR1_GPIO9, GPIO_DIR_MODE_OUT);

    // CANR2_GPIO10
    GPIO_setPinConfig(GPIO_10_GPIO10);
    GPIO_setDirectionMode(CANR2_GPIO10, GPIO_DIR_MODE_OUT);

    // LED2_TP4_GPIO56
    GPIO_setPinConfig(GPIO_56_GPIO56);
    GPIO_setDirectionMode(LED2_TP4_GPIO56, GPIO_DIR_MODE_OUT);

    // LED3_TP5_GPIO57
    GPIO_setPinConfig(GPIO_57_GPIO57);
    GPIO_setDirectionMode(LED3_TP5_GPIO57, GPIO_DIR_MODE_OUT);



    // DIN1_GPIO40 input
    GPIO_setPinConfig(GPIO_40_GPIO40);
    GPIO_setDirectionMode(DIN1_GPIO40, GPIO_DIR_MODE_IN);

    // DIN2_GPIO41 input
    GPIO_setPinConfig(GPIO_41_GPIO41);
    GPIO_setDirectionMode(DIN2_GPIO41, GPIO_DIR_MODE_IN);

    // DIN3_GPIO44 input
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(DIN3_GPIO44, GPIO_DIR_MODE_IN);

    */

}

inline void update_PWM(void)
{

}
inline void gap_det_CG();


// 조건 함수
inline bool is_over_current(float val, float ref) { return fabsf(val) > ref; }
inline bool is_over_voltage(float val, float ref) { return val > ref; }
inline bool is_under_voltage(float val, float ref) { return val < ref; }


// Trip 검사 함수
inline void check_Trip(void)
{


    //      if (is_over_current(g_CS_A, g_OC_ref)) gi_Trip_no = _OVER_CURR_A;
    //      else if (is_over_current(g_CS_B, g_OC_ref)) gi_Trip_no = _OVER_CURR_B;



    if (gi_Trip_no != _NONE) {
        //gi_State = _TRIP;
    }


}


inline void dsp_adc_scalling(void)
{
    //GPIO_togglePin(LED3_TP5_GPIO57);

    g_DCPT1_A1_adc = (float)gu_DCPT1_A1 * _DSP_ADC_SCALE_;
    //Vadc = A*((4.99f/4.f)/5000.f)*(10.f/3.57f)
    g_DCPT1_A1 = (g_DCPT1_A1_adc - 0.300f)*800.f/1.995f;
    do_LPF_GAMMA(g_DCPT1_A1_LPF, g_DCPT1_A1, _GAMMA_50ksample_10HzLPF);

    g_DCPT2_A2_adc = (float)gu_DCPT2_A2 * _DSP_ADC_SCALE_;
    //Vadc = A*((4.99f/4.f)/5000.f)*(10.f/3.57f)
    g_DCPT2_A2 = (g_DCPT2_A2_adc - 0.300f)*100.f/1.961f;
    do_LPF_GAMMA(g_DCPT2_A2_LPF, g_DCPT2_A2, _GAMMA_50ksample_10HzLPF);

    g_DCCT1_A3_adc = (float)gu_DCCT1_A3 * _DSP_ADC_SCALE_;
    //Vadc = A*((4.99f/4.f)/5000.f)*(10.f/3.57f)
    g_DCCT1_A3 = (g_DCCT1_A3_adc - 1.640f)/13.3e-3f;
    do_LPF_GAMMA(g_DCCT1_A3_LPF, g_DCCT1_A3, _GAMMA_50ksample_10HzLPF);

    g_DCCT2_A6_adc = (float)gu_DCCT2_A6 * _DSP_ADC_SCALE_;
    //Vadc = V*(0.05f/1000.f)*(309.f/3.f)*(10.f/15.f)
    g_DCCT2_A6 = (g_DCCT2_A6_adc - 0.296f)*300.f/2.0f;
    do_LPF_GAMMA(g_DCCT2_A6_LPF, g_DCCT2_A6, _GAMMA_50ksample_10HzLPF);


}


// 50kHz 빠른 과전류 판단 및 즉시 차단
inline void check_OC_fast(void)
{
    // RUN 중일 때만 과전류 판단
    if (gi_State == _RUN) {
        // peak 갱신 (항상 + 이므로 fabsf 불필요)
        g_DCCT1_A3_peak = val_MAX(g_DCCT1_A3_peak, g_DCCT1_A3);
        g_DCCT2_A6_peak = val_MAX(g_DCCT2_A6_peak, g_DCCT2_A6);

        if (g_DCCT1_A3 > g_DCCT1_OC_ref) {
            gi_Trip_no = _OVER_CURR_DCCT1;
            GPIO_writePin(GEN_GPIO30, 0);  // 즉시 Gen OFF
            gi_State = _TRIP;
            return;
        }
        if (g_DCCT2_A6 > g_DCCT2_OC_ref) {
            gi_Trip_no = _OVER_CURR_DCCT2;
            GPIO_writePin(GEN_GPIO30, 0);  // 즉시 Gen OFF
            gi_State = _TRIP;
            return;
        }
    }
}



__interrupt void timer1ISR(void)
{
    // EINT 없음: self-preemption 방지, ISR을 atomic하게 유지
    GPIO_writePin(LED5G_GPIO22, 1);

    if (gu32_timer1_int_watch_dog_cnt < 1000000) gu32_timer1_int_watch_dog_cnt++;

    // RUN 중: 실행 시간 업데이트
    if (gi_State == _RUN) {
        if (g_T_run < g_T_end) g_T_run += T_1msec;
        else gi_State = _STOP;
    }

    // LPF만 계산 (EPWM 업데이트는 main루프에서)
    do_LPF_GAMMA(g_ModIndex_LPF, g_ModIndex, _GAMMA_1ksample_1HzLPF);

    GPIO_writePin(GEN_GPIO30, (gi_State == _RUN));  // Gen ON/OFF

    GPIO_writePin(LED5G_GPIO22, 0);

    CPUTimer_clearOverflowFlag(CPUTIMER1_BASE);
    // Timer1 = INT13 (PIE 외부) → clearACKGroup 불필요
}


__interrupt void adcA1ISR(void)
{
    GPIO_writePin(LED4R_GPIO20, 1);

    // ADC 결과 저장 후 즉시 리턴
    gu_DCPT1_A1 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);
    gu_DCPT2_A2 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    gu_DCCT1_A3 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);
    gu_DCCT2_A6 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER6);

    dsp_adc_scalling();          // 전류값 계산 및 LPF 필터 적용
    check_OC_fast();             // 과전류 빠른 판단 & 즉시 차단 ★중요★

    if(gu32_dsp_adc_int_watch_dog_cnt < 1000000) gu32_dsp_adc_int_watch_dog_cnt++;

    GPIO_writePin(LED4R_GPIO20, 0);

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


// 타이머 설정 함수
void initTimer1(float T_per)
{
    // 타이머 초기화
    CPUTimer_setPeriod(CPUTIMER1_BASE, DEVICE_SYSCLK_FREQ*T_per);
    CPUTimer_stopTimer(CPUTIMER1_BASE);
    CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);
    CPUTimer_startTimer(CPUTIMER1_BASE);

    // 인터럽트 설정
    Interrupt_register(INT_TIMER1, &timer1ISR);
    Interrupt_enable(INT_TIMER1);
    CPUTimer_enableInterrupt(CPUTIMER1_BASE);
}


//=============================================================================
// 공통 화면 제어 함수
//=============================================================================
void clear_screen(void)
{
    print_s("\033[2J");   // 전체 화면 지우기
}

void hide_cursor(void)
{
    print_s("\033[?25l"); // 커서 숨김
}

void show_cursor(void)
{
    print_s("\033[?25h"); // 커서 보임
}

void cursor_home(void)
{
    print_s("\033[H");    // 커서 홈 위치로 이동
}




void ADC_DIN_display(void)
{

    print_s("\n\n\rVariables ------------------------------------");

    // 이제 (32비트 완전 출력)
    print_s("\n\r  gu32_timer1_int_watch_dog_cnt = ");
    print_u32(gu32_timer1_int_watch_dog_cnt);
    gu32_timer1_int_watch_dog_cnt  = 0;
    print_s("\n\r  gu32_dsp_adc_int_watch_dog_cnt = ");
    print_u32(gu32_dsp_adc_int_watch_dog_cnt);
    gu32_dsp_adc_int_watch_dog_cnt = 0;

    print_s("\n\r  T_con = ");print_f(T_con*1e6f);print_s("usec");
    print_s("\n\r  PWM period = ");print_f(g_Per_usec);print_s("usec");
    print_s("\n\r  PWM freq   = ");print_f(g_freq_kHz);print_s("kHz");
    PRINT_I_VAR1_nr(gu_Per_ePWM1);
        print_s("\n\r");


    print_s("\n\rDSP_ADC(12 bit) ------------------------------");
    print_s("\n\r g_DCPT1_A1_adc(Tp2) = ");
        print_f(g_DCPT1_A1_adc);print_s(", ");
        print_f(g_DCPT1_A1);print_s("V  ");
        print_f(g_DCPT1_A1_LPF);print_s("Vavg(LPF)");

    print_s("\n\r g_DCPT2_A2_adc(Tp3) = ");
        print_f(g_DCPT2_A2_adc);print_s(", ");
        print_f(g_DCPT2_A2);print_s("V  ");
        print_f(g_DCPT2_A2_LPF);print_s("V(LPF)  ");

    print_s("\n\r g_DCCT1_A3_adc(Tp4) = ");
        print_f(g_DCCT1_A3_adc);print_s(", ");
        print_f(g_DCCT1_A3);print_s("A, ");
        print_f(g_DCCT1_A3_LPF);print_s("A(LPF), ");
        print_f(g_DCCT1_A3_peak);print_s("A(pk)  ");

    print_s("\n\r g_DCCT2_A6_adc(Tp6) = ");
        print_f(g_DCCT2_A6_adc);print_s(", ");
        print_f(g_DCCT2_A6);print_s("A, ");
        print_f(g_DCCT2_A6_LPF);print_s("A(LPF), ");
        print_f(g_DCCT2_A6_peak);print_s("A(pk)  ");



    print_s("\r\n\n");
    static int cur_cnt = 0, i;
    cur_cnt = cur_cnt%10 + 1;
    for(i=0;i<cur_cnt;i++) putchar('>');
    for(;i<10;i++) putchar(' ');
}




#define read_gpio(var)  read_gpio_binary("\r>> " #var " = ", &(var))
void Key_process(char key)
{
    float ftmp;
    int i;

    print_s("\r");
    show_cursor();

    // 대문자는 소문자로 변환
    if (key >= 'A' && key <= 'Z') key = key - 'A' + 'a';

    if(key == '\r') clear_screen();


    if (key == 'r'){
        if (gi_Trip_no == _NONE && (gi_State == _STANDBY || gi_State == _STOP)) {
            g_ModIndex_LPF =   0;
            g_T_run = 0;
            g_DCCT1_A3_peak = 0.0f;   // RUN 시작 시 peak 리셋
            g_DCCT2_A6_peak = 0.0f;
            SysCtl_setWatchdogPrescaler(SYSCTL_WD_PRESCALE_64);  // 타임아웃 ~838ms
            SysCtl_serviceWatchdog();   // 카운터 0 리셋 후 활성화 (즉시리셋 방지)
            SysCtl_enableWatchdog();
            gi_State = _RUN;
        }
    }
    else if (key == 'w'){
        print_s("\r\n !!! WDG RESET TEST: will reset in ~838ms !!!\r\n");
        while(1) { }   // 서비스 안 함 → 타임아웃 → CPU 리셋
    }
    else if (key == 's'){
        gi_State = _STOP;
    }
    else if (key == 'e'){
        if (gi_Trip_no != _NONE) {
            gi_Trip_no = _NONE;
            gi_State = _STANDBY;
            g_T_run = 0;
        }
    }
    else if (key == 't'){
        print_s("\n\rg_T_end = ");
        ftmp = input_f();
        if (ftmp >= 0.1f && ftmp <= 1000.f)g_T_end = ftmp;
    }
    else if (key == 'a'){
        print_s("\n\rg_DCCT1_OC_ref = ");
        ftmp = input_f();
        if (ftmp >= 5.f && ftmp <= 40.f)g_DCCT1_OC_ref = ftmp;
    }
    else if (key == 'b'){
        print_s("\n\rg_DCC2_OC_ref = ");
        ftmp = input_f();
        if (ftmp >= 50.f && ftmp <= 350.f)g_DCCT2_OC_ref = ftmp;

    }
    else if (key == 'm'){
        print_s("\n\rg_ModIndex = ");
        ftmp = input_f();
        if (ftmp >= 0 && ftmp <= 1.f){
            g_ModIndex = ftmp;
        }
    }
    else if (key == 'p'){
        print_s("\n\rPWM period(usec) = ");
        ftmp = input_f();
        if (ftmp >= 5.0f && ftmp <= 11.11f){
            g_Per_usec = ftmp;
        }
    }



    hide_cursor();
    putchar('\r');
    for(i=0;i<120;i++) putchar(' ');
}


void Status_display(void)
{
    int i;
    cursor_home();
    hide_cursor();

    print_s(   "======================================");
    print_s("\n\r=    PACTECH LLC30K ");print_s(_YYMMDD);
    print_s("\n\r======================================");

    print_s("\n\r[R]run, [S]stop, [E]reset");
    print_s("\n\r gi_State = ");
    print_s(state_names[gi_State]);

    if (gi_State == _TRIP) {
        print_s("TRIP: ");
        if (gi_Trip_no < TRIP_NAME_COUNT) print_s(trip_names[gi_Trip_no]);
    }
    else{
        for(i=0;i<100;i++) putchar(' ');
    }

    print_s("\n\n\r[t]");PRINT_F_VAR1(g_T_end);print_s(" sec, ");
        PRINT_F_VAR2(g_T_run);print_s(" sec     ");

    print_s("\n\r[a]");PRINT_F_VAR1(g_DCCT1_OC_ref);print_s("A");
    print_s("\n\r[b]");PRINT_F_VAR1(g_DCCT2_OC_ref);print_s("A");

    print_s("\n\r[m]");PRINT_F_VAR1(g_ModIndex);
    print_s("\n\r[p]");PRINT_F_VAR1(g_Per_usec);print_s("usec");

    print_s("\n\r[w] WDG = ");
    print_s((HWREGH(WD_BASE + SYSCTL_O_WDCR) & SYSCTL_WDCR_WDDIS) ? "OFF" : "ON ");

}


//================================================================================
void main(void)
{

    // 시스템 초기화
    Device_init();
    Device_initGPIO();
    Interrupt_initModule();
    Interrupt_initVectorTable();


    // GPIO 초기화
    init_GPIO();

    // HW 초기화
    gi_State = _STANDBY;
    //GPIO_writePin(GEN1_GPIO14, !is_run());   //0= ON, 1= OFF


    // 톱니파 삼각파, ePWM 및 인터럽트 초기화
    EPWM123_init(g_Per_usec);
    EPWM123_setModIndex(g_Per_usec, 0);


    // SCIA 초기화
    scia_init(9600);    // SCIA 초기화

    // ADCA 초기화
    initADC();
    initADCSOC();
    setupADCAInterrupt();// 인터럽트 설정

    initDAC_AOUT();

    // 타이머1 초기화
    initTimer1(T_1msec);  // 1kHz


    EINT;  // Enable Global Interrupt
    ERTM;  // Enable Realtime Interrupt


    while(1)
    {


        g_freq_kHz = 1e3f / g_Per_usec;

        EPWM123_setModIndex(g_Per_usec, g_ModIndex_LPF);   // ISR에서 이동 (1Hz LPF → main루프 업데이트 충분)

        SysCtl_serviceWatchdog();   // display 시작 전
        Status_display();
        SysCtl_serviceWatchdog();   // display 사이
        ADC_DIN_display();

        if(kbhit()) Key_process(getchar1());

        SysCtl_serviceWatchdog();   // 루프 끝

    }
}


