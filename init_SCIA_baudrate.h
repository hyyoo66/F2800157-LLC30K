#ifndef SCIA_DRIVER_H_
#define SCIA_DRIVER_H_

#include <stdint.h>

// =============================
// 상수 정의
// =============================
#define BUFFER_SIZE 64

// =============================
// 전역 변수 선언 (volatile)
// =============================
extern volatile char rxBuffer[BUFFER_SIZE];
extern volatile int rxWriteIndex;
extern volatile int rxReadIndex;
extern volatile int rxBufferCount;
extern char receivedChar;

// =============================
// 함수 프로토타입 선언
// =============================
void scia_init(uint16_t baud_rate);
/*
void scia_enable_rx_interrupt(void);
__interrupt void sciRX_ISR(void);
void clearKeyBuffer(void);
*/
int kbhit(void);
char getchar1(void);

#endif /* SCIA_DRIVER_H_ */
