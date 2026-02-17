// kernel/lib/utils.c
#include "utils.h"

void int_to_string(int num, char* str) {
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    char temp[16];
    int j = 0;

    while (num > 0) {
        temp[j++] = '0' + (num % 10);
        num /= 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    while (j > 0) {
        str[i++] = temp[--j];
    }

    str[i] = '\0';
}

// Verbesserte Hex zu String Konvertierung
void hex_to_string(uint32_t num, char* str) {
    const char* hex_digits = "0123456789ABCDEF";
    int i = 0;
    int leading = 1;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    for (int shift = 28; shift >= 0; shift -= 4) {
        uint8_t digit = (num >> shift) & 0xF;

        if (digit != 0 || !leading || shift == 0) {
            if (leading && digit == 0 && shift > 0) {
                continue;
            }
            leading = 0;
            str[i++] = hex_digits[digit];
        }
    }

    str[i] = '\0';
}

void hex_to_string_byte(unsigned char value, char* buffer) {
    const char hex_chars[] = "0123456789ABCDEF";

    buffer[0] = hex_chars[(value >> 4) & 0xF];
    buffer[1] = hex_chars[value & 0xF];
    buffer[2] = '\0';
}
