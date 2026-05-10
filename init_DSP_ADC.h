#ifndef INIT_DSP_ADC_H
#define INIT_DSP_ADC_H

#include <stdint.h>
#include "device.h"
#include "driverlib.h"
#include "main_define.h"

//=============================================================================
// ADC 占쏙옙占� 占쏙옙占쏙옙 占쏙옙占쏙옙 (extern 占쏙옙占쏙옙)
//=============================================================================
extern uint16_t gu_DCPT1_A1;
extern uint16_t gu_DCPT2_A2;
extern uint16_t gu_DCCT1_A3;
extern uint16_t gu_DCCT2_A6;

//extern uint32_t gu_dsp_adc_int_watch_dog_cnt;
//extern int      LED_toggle_flag;

//=============================================================================
// 占쌉쇽옙 占쏙옙占쏙옙 占쏙옙占쏙옙
//=============================================================================
void initADC(void);
void initADCSOC(void);
void setupADCAInterrupt(void);

void initDAC_AOUT(void);
void setDAC_A_voltage(float v);

void initDAC_BOUT(void);
void setDAC_B_voltage(float v);

// ADCA1 占쏙옙占싶뤄옙트
__interrupt void adcA1ISR(void);

// LED GPIO 占쏙옙호
#define LED3_TP5_GPIO57   57

#endif  // INIT_DSP_ADC_H


/*
#ifndef INIT_DSP_ADC_H
#define INIT_DSP_ADC_H

#include <stdint.h>
#include "driverlib.h"
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

// ADC 占십깍옙화
void initADC(void);

// ADCSOC 占쏙옙占쏙옙
void initADCSOC(void);

// ADCA 占쏙옙占싶뤄옙트 占쏙옙占쏙옙
void setupADCAInterrupt(void);

// ADC 占쏙옙占� 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
extern uint16_t gu_DCCT1_A1;
extern uint16_t gu_DCCT2_A2;
extern uint16_t gu_DCCT3_A3;
extern uint16_t gu_DCPT1_A4;
extern uint16_t gu_NTC5K_A5;
extern uint16_t gu_TEST10A_A6;
extern uint16_t gu_TEST48V_A7;
extern uint16_t gu_DCPT2_A8;

extern uint16_t gu_P15V_B0;
extern uint16_t gu_N15V_B2;
extern uint16_t gu_A5V_B3;
extern uint16_t gu_P15V1_B4;
extern uint16_t gu_N15V1_B5;


// DAC_AOUT 占십깍옙화
void initDAC_AOUT(void);

// DAC_AOUT 占쏙옙占쏙옙 占쏙옙占�
 void setDAC_voltage(float v);

#ifdef __cplusplus
}
#endif

#endif // INIT_DSP_ADC_H
 */

