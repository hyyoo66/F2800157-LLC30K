#include <stdint.h>
#include <stdio.h>
#include <file.h>
#include <launchxl_ex1_sci_io_driverlib.h>
#include "launchxl_ex1_ti_ascii.h"
#include "driverlib.h"
#include "device.h"
#include "driverlib/sysctl.h"  // sysctl 관련 헤더 파일 포함
#include "init_SCIA_baudrate.h"

// SCIA 초기화 함수
    // SCIA 초기화

// 115200, 38400
void scia_init(uint16_t baud_rate)
{
    // GPIO 설정
    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCIRXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCIRXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCITXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCITXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_QUAL_ASYNC);

    // SCIA 설정
    SCI_performSoftwareReset(SCIA_BASE);
    SCI_setConfig(SCIA_BASE, DEVICE_LSPCLK_FREQ, baud_rate,
                  (SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE));
    SCI_enableModule(SCIA_BASE);

    // STDOUT -> SCIA 연결
    add_device("scia", _SSA, SCI_open, SCI_close, SCI_read, SCI_write,
               SCI_lseek, SCI_unlink, SCI_rename);
    freopen("scia:", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);
}

/*
** in main()
**   scia_enable_rx_interrupt();    // 수신 인터럽트만 별도 활성화
**
#define BUFFER_SIZE 64
volatile char rxBuffer[BUFFER_SIZE];  // 수신된 데이터를 저장할 라운드 버퍼
volatile int rxWriteIndex = 0;        // 버퍼에 쓸 위치
volatile int rxReadIndex = 0;         // 버퍼에서 읽을 위치
volatile int rxBufferCount = 0;       // 버퍼에 저장된 데이터 개수

char receivedChar = 0;

// SCI 인터럽트 서비스 루틴
__interrupt void sciRX_ISR(void)
{

    // SCIRXBUF 레지스터에서 데이터 읽기
    receivedChar = HWREGH(SCIA_BASE + SCI_O_RXBUF);


    // 라운드 버퍼에 데이터 저장
    rxBuffer[rxWriteIndex] = receivedChar;
    rxWriteIndex = (rxWriteIndex + 1) % BUFFER_SIZE;
    if (rxBufferCount < BUFFER_SIZE)
    {
        rxBufferCount++;
    }

    // 인터럽트 플래그 클리어
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXRDY_BRKDT);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);  // 그룹 9의 인터럽트 ACK
}


void scia_enable_rx_interrupt(void)
{
    SCI_disableFIFO(SCIA_BASE);
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXRDY_BRKDT);
    SCI_enableInterrupt(SCIA_BASE, SCI_INT_RXRDY_BRKDT);

    Interrupt_register(INT_SCIA_RX, &sciRX_ISR);
    Interrupt_enable(INT_SCIA_RX);
}
*/



/* 버퍼에서 데이터를 읽는 함수
int getchar1(void)
{
    char ch = 0;

    // 데이터가 입력될 때까지 대기
    while (rxBufferCount == 0)
    {
        // 데이터가 들어올 때까지 기다림
    }

    // 버퍼에서 데이터 읽기
    ch = rxBuffer[rxReadIndex];
    rxReadIndex = (rxReadIndex + 1) % BUFFER_SIZE;
    rxBufferCount--;

    return ch;
}

// 버퍼에 새로운 데이터가 있는지 확인하는 함수
int kbhit(void)
{
    return rxBufferCount > 0;  // 버퍼에 데이터가 있으면 1을 반환
}

void clearKeyBuffer(void)
{
    for (; kbhit(); )getchar1();  // 읽어서 버리기
}
*/


// polling 방식
int kbhit(void)
{
    return (HWREGH(SCIA_BASE + SCI_O_RXST) & SCI_RXST_RXRDY) ? 1 : 0;
}

// polling 방식
char getchar1(void)
{
    while (!(HWREGH(SCIA_BASE + SCI_O_RXST) & SCI_RXST_RXRDY));
    return (char)(HWREGH(SCIA_BASE + SCI_O_RXBUF));
}

