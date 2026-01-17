#include "printk.h"
#include <stdarg.h>

static void sbi_console_putchar(int ch) {
    register unsigned long a0 asm("a0") = ch;
    register unsigned long a7 asm("a7") = 1;
    asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
}

static void putchar(char c) {
    sbi_console_putchar(c);
}

static void puts(const char *s) {
    while (*s) {
        putchar(*s++);
    }
}

static void put_uint(uint64_t n, int base) {
    char buf[32];
    int i = 0;
    const char *digits = "0123456789abcdef";
    
    if (n == 0) {
        putchar('0');
        return;
    }
    
    while (n > 0) {
        buf[i++] = digits[n % base];
        n /= base;
    }
    
    while (i > 0) {
        putchar(buf[--i]);
    }
}

void printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    while (*fmt) {
        if (*fmt == '%' && *(fmt + 1)) {
            fmt++;
            switch (*fmt) {
                case 'd': {
                    int val = va_arg(args, int);
                    if (val < 0) {
                        putchar('-');
                        val = -val;
                    }
                    put_uint(val, 10);
                    break;
                }
                case 'u':
                    put_uint(va_arg(args, unsigned int), 10);
                    break;
                case 'x':
                    put_uint(va_arg(args, unsigned int), 16);
                    break;
                case 'l': {
                    fmt++;
                    if (*fmt == 'x') {
                        put_uint(va_arg(args, unsigned long), 16);
                    } else if (*fmt == 'u') {
                        put_uint(va_arg(args, unsigned long), 10);
                    } else if (*fmt == 'd') {
                        long val = va_arg(args, long);
                        if (val < 0) {
                            putchar('-');
                            val = -val;
                        }
                        put_uint(val, 10);
                    }
                    break;
                }
                case 's':
                    puts(va_arg(args, char *));
                    break;
                case 'c':
                    putchar(va_arg(args, int));
                    break;
                case '%':
                    putchar('%');
                    break;
            }
        } else {
            putchar(*fmt);
        }
        fmt++;
    }
    
    va_end(args);
}
