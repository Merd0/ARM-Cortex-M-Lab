/*
 * Lesson 09 - Optimizasyon fikstuu: AYNI C kaynagi, UC preset.
 *
 *   debug   = -Og      release = -O2      minsize = -Os
 *
 * Bu dosyanin asil ciktisi calisma zamani DEGIL, her preset'in .lst
 * dosyasidir. Her fonksiyon TEK bir etkiyi gostermek icin var.
 *
 * Bazi fonksiyonlar YALNIZCA disassembly icindir ve ASLA cagrilmaz:
 *   poll_plain / poll_vol   sonsuz dongu olabilir
 *   signed_gt               isaretli tasma = tanimsiz davranis (UB)
 *   asm_clobber_bad         bilerek yanlis inline asm (AAPCS ihlali)
 * Bunlar --gc-sections'tan kurtulsun diye adresleri alinir (g_keep),
 * ama cagrilmazlar.
 *
 * Calisma zamani yalnizca tanimli davranisa sahip fonksiyonlari cagirir ve
 * bir OPT_CHECKSUM basar: bu deger UC preset'te de AYNI olmalidir.
 * Makine kodu degisir; anlam degismez.
 */

#include <stdint.h>
#include "platform/mps2_an505.h"
#include "common/console.h"
#include "abi.h"

#define NOINLINE __attribute__((noinline))

/* ------------------------------------------------------------------ */
/* O1  Yaprak, yalnizca register                                        */
/* ------------------------------------------------------------------ */
NOINLINE uint32_t leaf_regs(uint32_t a, uint32_t b)
{
    return (a ^ b) + (a >> 3);
}

/* ------------------------------------------------------------------ */
/* O2  Stack KULLANAN yaprak                                            */
/*                                                                      */
/* Hicbir fonksiyon CAGIRMAZ (gercek yaprak). Diziyi bellekte tutmaya   */
/* zorlayan sey bir cagri degil, bos bir inline asm: dizinin adresini   */
/* girdi olarak alir ve "memory" clobber'i bildirir. Derleyici asm'in   */
/* diziyi okuyabilecegini varsaymak zorunda kalir.                      */
/* ------------------------------------------------------------------ */
NOINLINE uint32_t leaf_with_stack(uint32_t seed)
{
    uint32_t buf[4];
    for (uint32_t i = 0u; i < 4u; i++) {
        buf[i] = seed + i;
    }
    __asm__ volatile ("" : : "r" (buf) : "memory");
    return buf[0] + buf[1] + buf[2] + buf[3];
}

/* ------------------------------------------------------------------ */
/* O3  Tail call adayi. helper_ext AYRI bir TU'da (helpers.c).          */
/* ------------------------------------------------------------------ */
NOINLINE uint32_t tail_call(uint32_t x)
{
    return helper_ext(x + 1u);
}

/* ------------------------------------------------------------------ */
/* O4  Inlining. sq() iki ayri yerden cagriliyor (tek cagri, -Og'da     */
/*     bile "called once" inlining'e yol acabilirdi).                   */
/* ------------------------------------------------------------------ */
static uint32_t sq(uint32_t x)
{
    return x * x;
}

NOINLINE uint32_t use_sq_twice(uint32_t a, uint32_t b)
{
    return sq(a) + sq(b);
}

/* ------------------------------------------------------------------ */
/* O5  Olu yazma (dead store) ve volatile                               */
/* ------------------------------------------------------------------ */
uint32_t          g_plain;
volatile uint32_t g_vol;

NOINLINE void dead_store(void)
{
    g_plain = 1u;
    g_plain = 2u;
}

NOINLINE void vol_store_twice(void)
{
    g_vol = 1u;
    g_vol = 2u;
}

/* ------------------------------------------------------------------ */
/* O6  Yoklama (polling) - YALNIZCA DISASSEMBLY, ASLA CAGRILMAZ          */
/* ------------------------------------------------------------------ */
uint32_t          g_flag_plain;
volatile uint32_t g_flag_vol;

NOINLINE void poll_plain(void)
{
    while (g_flag_plain == 0u) {
    }
}

NOINLINE void poll_vol(void)
{
    while (g_flag_vol == 0u) {
    }
}

/* ------------------------------------------------------------------ */
/* O7  Isaretli / isaretsiz tasma                                       */
/*     signed_gt: YALNIZCA DISASSEMBLY (x = INT_MAX icin UB).           */
/*     unsigned_gt: her girdi icin tanimli; calisma zamaninda cagrilir. */
/* ------------------------------------------------------------------ */
NOINLINE int signed_gt(int x)
{
    return (x + 1) > x;
}

NOINLINE int unsigned_gt(unsigned x)
{
    return (x + 1u) > x;
}

/* ------------------------------------------------------------------ */
/* O8  Inline asm'in IKI sozlesmesi                                     */
/*     _bad: r4'u yazar ama derleyiciye SOYLEMEZ -> YALNIZCA DISASSEMBLY */
/*     _good: "r4" clobber'ini bildirir -> derleyici r4'u saklar        */
/* ------------------------------------------------------------------ */
NOINLINE void asm_clobber_bad(uint32_t v)
{
    __asm__ volatile ("mov r4, %0" : : "r" (v));
}

NOINLINE void asm_clobber_good(uint32_t v)
{
    __asm__ volatile ("mov r4, %0" : : "r" (v) : "r4");
}

/* ------------------------------------------------------------------ */
/* O9  r9 gozlemi: "r9" clobber'i derleyicinin r9'u saklayip           */
/*     saklamadigini gosterir (M6). Cagrilmasi guvenli.                */
/* ------------------------------------------------------------------ */
NOINLINE void clobber_r9(void)
{
    __asm__ volatile ("mov r9, #1" : : : "r9");
}

/* ------------------------------------------------------------------ */
/* Yalnizca-disassembly fonksiyonlarini GC'den korumak icin: adresleri  */
/* alinir, ASLA cagrilmaz.                                              */
/* ------------------------------------------------------------------ */
volatile uintptr_t g_keep;

int main(void)
{
    console_line("");
    console_line("===== LESSON 09: OPTIMIZATION =====");

    g_keep = (uintptr_t)poll_plain ^ (uintptr_t)poll_vol ^
             (uintptr_t)signed_gt  ^ (uintptr_t)asm_clobber_bad;

    uint32_t sum = 0u;
    sum += leaf_regs(0x12345678u, 0x0F0F0F0Fu);
    sum += leaf_with_stack(100u);
    sum += tail_call(7u);
    sum += use_sq_twice(3u, 4u);
    dead_store();
    sum += g_plain;
    vol_store_twice();
    sum += g_vol;
    sum += (uint32_t)unsigned_gt(5u);
    sum += (uint32_t)unsigned_gt(0xFFFFFFFFu);   /* tanimli: sarar -> 0 */
    asm_clobber_good(0xC0FFEEu);
    clobber_r9();

    console_kv_hex("  OPT_CHECKSUM", sum);
    console_kv_str("  CHECK_OPT_CHECKSUM", (sum == L09_EXPECTED_CHECKSUM) ? "PASS" : "FAIL");
    console_line("===== END =====");
    for (;;) {
        __WFI();
    }
}
