#include "drivers/uart.h"
#include <stdarg.h>

enum { HEXADECIMAL = 16, DECIMAL = 10 };

char int_to_char(int value) {
  if (value < 10) {
    return (char)('0' + value);
  }
  return (char)('a' + value - 10);
}

static void print_non_zero_uint(unsigned value, unsigned base) {
    if (value == 0) {
        return;
    }
    print_non_zero_uint(value / base, base);
    uartputc(int_to_char(value % base));
}

static void print_uint(unsigned value, unsigned base) {
    if (value == 0) {
        uartputc('0');
        return;
    }
    print_non_zero_uint(value, base);
}

void kprintf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            ++fmt;
            switch (*fmt)
            {
            case 'u':
                print_uint(va_arg(ap, unsigned), DECIMAL);
                break;
            case 'x':
                print_uint(va_arg(ap, unsigned), HEXADECIMAL);
                break;
            default:
                --fmt;
                break;
            }
        } else {
            uartputc(*fmt);
        }
        ++fmt;
    } 

    va_end(ap);
}
