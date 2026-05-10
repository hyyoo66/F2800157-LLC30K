#include <key_print_digit_260507.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "driverlib.h"
#include "device.h"

// 외부 UART 입력 함수 (주인님 코드에 맞춰 정의되어 있어야 함)
extern char getchar1(void);

float input_f(void)
{
    char ch;
    char string[20];
    int i = 0;
    float number = 0;
    int flag_dot = 0;

    do {
        SysCtl_serviceWatchdog();   // 타이핑 중 와치독 갱신
        ch = getchar1();

        if (ch == '-' && i == 0) {  // 음수 부호는 첫 글자만 허용
            putchar(ch);
            string[i++] = ch;
        }
        else if (ch == '.' && flag_dot == 0) {
            flag_dot = 1;
            putchar(ch);
            string[i++] = ch;
        }
        else if (ch >= '0' && ch <= '9') {
            putchar(ch);
            string[i++] = ch;
        }
    } while (ch != 0x0D && ch != 0x0A && i < sizeof(string) - 1);  // Enter 처리 (CR or LF)

    string[i] = '\0';
    sscanf(string, "%f", &number);
    return number;
}


void print_s(const char *str)
{
    while (*str)
        putchar(*str++);
}


// 클로드-3
void print_integer(int n) {
    if (n == 0) {
        putchar('0');
        return;
    }
    char buf[10];
    int idx = 0;
    while (n > 0) {
        buf[idx++] = '0' + (n % 10);
        n /= 10;
    }
    while (idx--)
        putchar(buf[idx]);
}

void print_u32(uint32_t num) {
    char buf[11];   // uint32_t 최대 10자리 + null
    int idx = 0;
    if (num == 0) {
        putchar('0');
        return;
    }
    while (num > 0) {
        buf[idx++] = '0' + (num % 10);
        num /= 10;
    }
    while (idx--)
        putchar(buf[idx]);
}

void print_i(int num) {
    if (num > 999 || num < -999) {
        // 범위를 벗어나면 그냥 전체 숫자 출력
        if (num < 0) {
            putchar('-');
            num = -num;
        } else {
            putchar(' ');
        }
        print_integer(num);
        return;
    }
    // -999 ~ 999인 경우 항상 4자리 고정 출력
    if (num < 0) {
        num = -num;
        // 음수: 부호 뒤에 바로 숫자, 전체를 우측 정렬
        if (num >= 100) {
            putchar('-');
            print_integer(num);  // -999, -100
        } else if (num >= 10) {
            putchar(' ');
            putchar('-');
            print_integer(num);  // -24, -10
        } else {
            putchar(' ');
            putchar(' ');
            putchar('-');
            print_integer(num);  // -5, -1
        }
    } else {
        // 양수: 4자리 우측 정렬 (앞쪽 공백)
        if (num >= 100) {
            putchar(' ');
            print_integer(num);
        } else if (num >= 10) {
            putchar(' ');
            putchar(' ');
            print_integer(num);
        } else {
            putchar(' ');
            putchar(' ');
            putchar(' ');
            print_integer(num);
        }
    }
}

void print_f(float num) {
    int i;
    // 부호 처리
    int is_negative = (num < 0);
    if (is_negative) num = -num;

    // 정수부와 소수부 분리
    int integer_part = (int)num;
    float decimal_part = num - integer_part;

    // 정수부 자릿수 계산
    int int_digits = (integer_part == 0) ? 1 :
                     (integer_part >= 1000) ? 4 :
                     (integer_part >= 100) ? 3 :
                     (integer_part >= 10) ? 2 : 1;

    // 유효숫자 4자리에 맞는 소수부 자릿수
    int frac_digits = (4 - int_digits > 0) ? 4 - int_digits : 0;

    // 소수부 반올림 처리 (한 자리씩 추출 방식)
    float temp_decimal = decimal_part;

    // 반올림을 위해 다음 자릿수까지 고려
    if (frac_digits > 0) {
        for (i = 0; i < frac_digits; i++) {
            temp_decimal *= 10;
        }
        // 다음 자릿수 확인을 위해 한 번 더
        temp_decimal *= 10;
        int next_digit = (int)temp_decimal % 10;
        temp_decimal = (int)(temp_decimal / 10);

        // 반올림 처리
        if (next_digit >= 5) {
            temp_decimal += 1;
        }

        // 반올림으로 인한 자릿수 증가 처리
        int scale = 1;
        for (i = 0; i < frac_digits; i++) scale *= 10;

        if ((int)temp_decimal >= scale) {
            integer_part++;
            temp_decimal = 0;
            // 정수부가 증가했으므로 자릿수 재계산
            int_digits = (integer_part >= 1000) ? 4 :
                         (integer_part >= 100) ? 3 :
                         (integer_part >= 10) ? 2 : 1;
            frac_digits = (4 - int_digits > 0) ? 4 - int_digits : 0;
            if (frac_digits == 0) temp_decimal = 0;
        }
    }

    // 전체 길이 계산
    int content_len = int_digits + (frac_digits > 0 ? 1 + frac_digits : 0);
    int total_len = (is_negative ? 1 : 0) + content_len;

    // 우측 정렬을 위한 앞쪽 공백 출력
    for (i = 0; i < 7 - total_len; i++) {
        putchar(' ');
    }

    // 부호 출력
    if (is_negative) {
        putchar('-');
    }

    // 정수부 출력
    print_integer(integer_part);

    // 소수부 출력
    if (frac_digits > 0) {
        putchar('.');

        // 소수부를 한 자리씩 출력
        int frac_value = (int)temp_decimal;
        if (frac_value == 0) {
            for (i = 0; i < frac_digits; i++) {
                putchar('0');
            }
        } else {
            // frac_value의 자릿수 계산
            int temp = frac_value;
            int actual_digits = 0;
            while (temp > 0) {
                temp /= 10;
                actual_digits++;
            }

            // 필요한 만큼 0 채우기
            for (i = 0; i < frac_digits - actual_digits; i++) {
                putchar('0');
            }
            print_integer(frac_value);
        }
    }
}

void print_f6(float num) {
    int i;
    if (num < 0) {
        putchar('-');
        num = -num;
    } else {
        putchar(' ');
    }

    int integer_part = (int)num;
    float decimal_part = num - integer_part;

    print_integer(integer_part);
    putchar('.');

    // 소수부를 한 자리씩 추출하여 4자리 출력
    for (i = 0; i < 6; i++) {
        decimal_part *= 10;
        int digit = (int)decimal_part;
        putchar('0' + digit);
        decimal_part -= digit;  // 정수부 제거
    }
}
