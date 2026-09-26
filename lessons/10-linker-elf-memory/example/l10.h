/*
 * Ders 10 - ortak bildirimler.
 *
 * Nesneler BILEREK ayri derleme birimlerinde (objects.c, placement.c,
 * l10_asm.S) tanimli: boylece main.o bunlara ADIYLA baglanmak zorunda
 * kalir ve .o dosyasinda cozumlenmemis referanslar + relocation'lar
 * gozlenebilir.
 */
#ifndef LESSON10_L10_H
#define LESSON10_L10_H

#include <stdint.h>

/* objects.c */
extern const uint32_t k_table[4];        /* -> .rodata.k_table             */
extern const uint32_t k_custom[2];       /* -> .rodata.l10_table (ozel ad)  */
extern uint32_t       g_init;            /* -> .data.g_init                 */
extern uint32_t       g_zero;            /* -> .bss.g_zero                  */
uint32_t  counter_next(void);            /* static yerel -> .bss            */
uint32_t *counter_addr(void);

/* placement.c */
uint32_t text_fn(uint32_t x);            /* -> .text.text_fn                */

/* l10_asm.S */
uint32_t asm_fn(uint32_t x);             /* .global -> baska TU'dan gorunur */
extern uint32_t asm_word;                /* .data icinde, .global           */

/* Linker script'in tanimladigi semboller (platform/.../mps2-an505-secure.ld).
 * Bunlarin DEPOLAMASI yok; yalnizca ADRESLERI anlamli: &__data_start gibi
 * kullanilir. */
extern uint32_t __vectors_start;
extern uint32_t __vectors_end;
extern uint32_t __etext;                 /* = LOADADDR(.data)               */
extern uint32_t __data_start;
extern uint32_t __data_end;
extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern uint32_t __heap_start;
extern uint32_t __heap_end;
extern uint32_t __flash_load_end;
extern uint32_t __stack_top;

#define L10_FLASH_ORIGIN  0x10000000u    /* MEMORY { FLASH ... }            */
#define L10_FLASH_LENGTH  (512u * 1024u)
#define L10_RAM_ORIGIN    0x30000000u    /* MEMORY { RAM ... }              */
#define L10_RAM_LENGTH    (32u * 1024u)

#endif
