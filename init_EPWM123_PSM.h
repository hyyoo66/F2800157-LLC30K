#ifndef INIT_EPWM123_PSM_H
#define INIT_EPWM123_PSM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// =============================
// 사용자 설정
// =============================
#define DEAD_TICKS   18

// =============================
// 외부 변수
// =============================
extern uint16_t gu_Per_ePWM1;

// =============================
// 함수 선언
// =============================
void initEPWMGpio_123(void);

void EPWM123_init(float per_usec);

void EPWM123_setModIndex(float per_usec, float modindex);

#ifdef __cplusplus
}
#endif

#endif
