/*
 * Minimal console.
 * Bilerek printf yok: libc karmasasi olmadan, her satirin ne yaptigi gorunur.
 */
#ifndef COMMON_CONSOLE_H
#define COMMON_CONSOLE_H

#include <stdint.h>

void console_putc(char c);
void console_puts(const char *s);       /* satir sonu EKLEMEZ */
void console_line(const char *s);       /* satir sonu ekler   */
void console_hex32(uint32_t v);         /* 0xXXXXXXXX          */
void console_u32(uint32_t v);           /* ondalik             */

/* "[KEY] 0xXXXXXXXX" seklinde tek satir */
void console_kv_hex(const char *key, uint32_t v);
/* "[KEY] 1234" seklinde tek satir */
void console_kv_u32(const char *key, uint32_t v);
/* "[KEY] text" seklinde tek satir */
void console_kv_str(const char *key, const char *text);

#endif /* COMMON_CONSOLE_H */
