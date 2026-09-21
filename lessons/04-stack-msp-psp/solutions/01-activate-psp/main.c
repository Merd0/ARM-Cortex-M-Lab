/*
 * SOLUTION 01 - Activate PSP Correctly
 *
 * Hazir gelenler:
 *   - task_stack_block[] : hizalanmis Thread Mode stack alani + guard word'ler
 *   - psp_start() cagrisi ve thread_entry() giris noktasi
 *   - self-check         : dogru yaptiysan PASS basar
 *
 * Asil cozum bu dosyada degil: solutions/01-activate-psp/psp_start.S icinde.
 *
 * Neden C'de degil de assembly'de?
 *   CONTROL.SPSEL'i siradan bir C fonksiyonunun ortasinda degistirmek
 *   gecersizdir; ayrinti psp_start.S'in basindaki aciklamada.
 *
 * DOGRULAMA
 *   Uc CHECK satiri da PASS olmali. Ayrica GDB ile dogrula:
 *     p/x $sp   p/x $msp   p/x $psp   p/x $control
 *   ($sp hangisiyle ayni cikiyor? Neden?)
 *
 * Exercise: exercises/01-activate-psp/
 */

#include "platform/mps2_an505.h"
#include "common/console.h"

#define TASK_STACK_WORDS   128u
#define GUARD_WORDS        2u
#define GUARD_PATTERN      0xDEADBEEFu

static uint32_t task_stack_block[TASK_STACK_WORDS + GUARD_WORDS]
    __attribute__((aligned(8)));

#define TASK_STACK_BASE   ((uint32_t)&task_stack_block[0])
#define TASK_STACK_LIMIT  ((uint32_t)&task_stack_block[TASK_STACK_WORDS])

/* psp_start.S icinde. DONMEZ. */
void psp_start(uint32_t psp_top, void (*entry)(void));

static int guards_intact(void)
{
    for (unsigned i = 0u; i < GUARD_WORDS; i++) {
        if (task_stack_block[TASK_STACK_WORDS + i] != GUARD_PATTERN) {
            return 0;
        }
    }
    return 1;
}

/* ------------------------------------------------------------------ */
static void __attribute__((noreturn)) thread_entry(void)
{
    /* 1) Aktif stack gercekten PSP mi?
     *    Once mod: Handler Mode'da SPSEL'in degeri ne olursa olsun MSP
     *    kullanilir. Burada Thread Mode'da olmaliyiz. */
    int spsel_ok = (__get_IPSR() == 0u) && ((__get_CONTROL() & 0x2U) != 0u);

    /* 2) Yerel degiskenler artik task_stack icinde mi yasiyor? */
    volatile uint32_t probe = 0u;
    uint32_t probe_addr = (uint32_t)&probe;
    int region_ok = (probe_addr >= TASK_STACK_BASE) &&
                    (probe_addr <  TASK_STACK_LIMIT);

    /* 3) Tahsis edilen alanin disina yazildi mi? */
    int guard_ok = guards_intact();

    console_kv_hex("MSP", __get_MSP());
    console_kv_hex("PSP", __get_PSP());
    console_kv_hex("CONTROL", __get_CONTROL());
    console_kv_hex("LOCAL_ADDR", probe_addr);
    console_kv_hex("GUARD_VALUE", task_stack_block[TASK_STACK_WORDS]);

    console_kv_str("CHECK_SPSEL", spsel_ok  ? "PASS" : "FAIL");
    console_kv_str("CHECK_REGION", region_ok ? "PASS" : "FAIL");
    console_kv_str("CHECK_STACK_GUARD", guard_ok ? "PASS" : "FAIL");

    console_line("===== END =====");
    for (;;) { __WFI(); }
}

int main(void)
{
    console_line("");
    console_line("===== LESSON 04 SOLUTION 01 =====");

    for (unsigned i = 0u; i < GUARD_WORDS; i++) {
        task_stack_block[TASK_STACK_WORDS + i] = GUARD_PATTERN;
    }

    console_kv_hex("TASK_STACK_BASE", TASK_STACK_BASE);
    console_kv_hex("TASK_STACK_LIMIT", TASK_STACK_LIMIT);
    console_kv_u32("TOP_MOD_8", TASK_STACK_LIMIT % 8u);

    psp_start(TASK_STACK_LIMIT, thread_entry);   /* donmez */
}
