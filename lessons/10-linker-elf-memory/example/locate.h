/* Ders 10 Exercise 01 - "Locate Your Data" arayuzu. */
#ifndef LESSON10_LOCATE_H
#define LESSON10_LOCATE_H
#include <stdint.h>

/* FLASH'ta kullanilan byte: yuk goruntusunun TAMAMI
 * (vektorler + kod + salt-okunur veri + .data'nin ilk degerleri). */
uint32_t flash_used_bytes(void);

/* RAM'de STATIK olarak kullanilan byte: .data + .bss (stack haric). */
uint32_t ram_static_bytes(void);

/* .data icindeki bir nesnenin ILK DEGERININ FLASH'taki adresi (LMA). */
uint32_t data_lma_of(const void *p);

/* p, [origin, origin + length) araliginda mi? */
int in_region(const void *p, uint32_t origin, uint32_t length);
#endif
