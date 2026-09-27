/*
 * Ders 11 - ortak fikstur bildirimleri.
 *
 * Nesneler kasitli olarak ders-yerel ve taninabilir degerlerde:
 *   g_l11_init  baslatilmis global  -> .data  (ilk degeri FLASH'ta, LMA)
 *   g_l11_zero  sifir baslangicli   -> .bss
 *   g_l11_phase ders-yerel startup'in faz kaydi (.bss) - yalnizca ozel
 *               startup hedefleri kullanir; bkz. l11_reset.c
 */
#ifndef LESSON11_FIXTURES_H
#define LESSON11_FIXTURES_H

#include <stdint.h>

#define L11_INIT_VALUE   0x11C0FFEEu
#define L11_POISON       0xA5A5A5A5u   /* EGITIM AMACLI TEST ARACI; bkz. l11_vectors.c */
#define L11_PHASE_OK     0x123u        /* .data -> .bss -> SystemInit                   */

extern uint32_t g_l11_init;
extern uint32_t g_l11_zero;
extern uint32_t g_l11_phase;

/* Linker script sembolleri (platform/mps2-an505/linker/mps2-an505-secure.ld).
 * Depolamalari yok; yalnizca ADRESLERI anlamli. */
/* Vektor tablosunun basi: bir word DIZISI (boyutu linker'da). */
extern const uint32_t __vectors_start[];
extern uint32_t __etext;          /* = LOADADDR(.data) */
extern uint32_t __data_start;
extern uint32_t __data_end;
extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern uint32_t __stack_top;
extern uint32_t __stack_limit;

#endif
