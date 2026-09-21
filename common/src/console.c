#include "common/console.h"
#include "platform/board.h"

void console_putc(char c)
{
    if (c == '\n') {
        board_uart_putc('\r');
    }
    board_uart_putc(c);
}

void console_puts(const char *s)
{
    while (*s) {
        console_putc(*s++);
    }
}

void console_line(const char *s)
{
    console_puts(s);
    console_putc('\n');
}

void console_hex32(uint32_t v)
{
    static const char digits[] = "0123456789ABCDEF";
    console_puts("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        console_putc(digits[(v >> shift) & 0xFU]);
    }
}

void console_u32(uint32_t v)
{
    char buf[11];
    int i = 0;
    if (v == 0U) {
        console_putc('0');
        return;
    }
    while (v > 0U) {
        buf[i++] = (char)('0' + (v % 10U));
        v /= 10U;
    }
    while (i-- > 0) {
        console_putc(buf[i]);
    }
}

void console_kv_hex(const char *key, uint32_t v)
{
    console_putc('[');
    console_puts(key);
    console_puts("] ");
    console_hex32(v);
    console_putc('\n');
}

void console_kv_u32(const char *key, uint32_t v)
{
    console_putc('[');
    console_puts(key);
    console_puts("] ");
    console_u32(v);
    console_putc('\n');
}

void console_kv_str(const char *key, const char *text)
{
    console_putc('[');
    console_puts(key);
    console_puts("] ");
    console_puts(text);
    console_putc('\n');
}
