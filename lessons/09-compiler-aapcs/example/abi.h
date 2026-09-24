/*
 * Ders 09 - AAPCS olcum araclarinin ortak prototipleri.
 *
 * Probe'lar ve harness EL YAZIMI assembly'dir; C tarafinin argumanlari
 * dogru yerlestirebilmesi icin TAM prototiplerin cagiran taraf tarafindan
 * gorulmesi gerekir. Bu baslik o yuzden var.
 */
#ifndef LESSON09_ABI_H
#define LESSON09_ABI_H

#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Arguman / donus probe'lari (example/abi_probes.S)                   */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t r0, r1, r2, r3;   /* +0  +4  +8  +12 : giristeki register'lar */
    uint32_t sp;               /* +16             : giristeki SP           */
    uint32_t stk0, stk1;       /* +20 +24         : [SP+0], [SP+4]         */
    uint32_t ret_r0, ret_r1;   /* +28 +32         : BL'den hemen sonra     */
} probe_capture_t;

extern volatile probe_capture_t g_probe;

void     probe_4u32(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
void     probe_5u32(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e);
void     probe_u32_u64(uint32_t a, uint64_t b);
void     probe_3u32_u64(uint32_t a, uint32_t b, uint32_t c, uint64_t d);
uint64_t probe_call_u64ret(void);

/* C -> asm -> C zinciri */
uint32_t abi_chain(uint32_t x);
extern volatile uint32_t g_chain_sp_at_call;

/* ------------------------------------------------------------------ */
/* Callee-saved harness (example/abi_harness.S)                        */
/* ------------------------------------------------------------------ */
typedef uint32_t (*abi_fn2_t)(uint32_t a, uint32_t b);

/* Donus: bozulma maskesi.  bit0..bit7 = r4..r11,  bit8 = SP geri gelmedi.
 * *out_result: test edilen fonksiyonun donus degeri. */
uint32_t abi_check_callee(abi_fn2_t fn, uint32_t a, uint32_t b,
                          uint32_t *out_result);
extern volatile uint32_t g_harness_sp_at_call;

#define ABI_MASK_SP   (1u << 8)

/* Test edilen callee'ler */
uint32_t good_callee(uint32_t a, uint32_t b);   /* example/good_callee.S */
uint32_t bad_callee(uint32_t a, uint32_t b);    /* example/bad_callee.S  */

/* C yardimcilari (example/helpers.c - AYRI derleme birimi) */
uint32_t helper_scale(uint32_t x);
uint64_t make_u64(void);
uint32_t helper_ext(uint32_t x);

#endif
