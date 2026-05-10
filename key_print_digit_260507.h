#ifndef KEY_PRINT_DIGIT_2506_H
#define KEY_PRINT_DIGIT_2506_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// ===== �ܺ� UART �Է� �Լ� (���δ� �ý��ۿ� ���� �ʿ�) =====
extern char getchar1(void);
//extern void putchar(char c);

// ===== �Լ� ���� =====
float input_f(void);           // UART �Է� �� float ��ȯ
void print_i(int n);            // ���� ���
void print_u32(uint32_t n);     // 32bit unsigned integer ���
void print_s(const char *str);   // ���ڿ� ���
void print_f(float num);     // �����Ҽ��� ��� (�ݿø� ����)
void print_f6(float num);

// ��Ÿ �ʿ��� �Լ� �Ǵ� ��ũ�� �߰� ����

#ifdef __cplusplus
}
#endif

#endif  // KEY_PRINT_DIGIT_2506_H
