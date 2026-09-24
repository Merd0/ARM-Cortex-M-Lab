/*
 * Lesson 09 SOLUTION 01 - Repair an ABI-Broken Assembly Function
 *
 * scale_and_sum() (bu klasordeki .S) harness uzerinden cagrilir:
 *   - sonuc dogru mu?                    CHECK_RESULT
 *   - callee-saved register'lar korundu mu? CHECK_CALLEE_SAVED
 *   - BL aninda SP 8'e hizali miydi?     CHECK_SP_ALIGNED_AT_CALL
 *
 * Bu dosyayi DEGISTIRME; yalnizca scale_and_sum.S'i duzenle.
 */
#include "platform/mps2_an505.h"
#include "common/console.h"
#include "abi.h"

volatile uint32_t g_harness_sp_at_call;
volatile uint32_t g_ex_sp_at_call;

uint32_t scale_and_sum(uint32_t a, uint32_t b);

static const char *pf(int ok) { return ok ? "PASS" : "FAIL"; }

static void print_mask_regs(uint32_t mask)
{
    static const char *names[9] = {
        "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "SP"
    };
    console_puts("[  CORRUPTED]");
    if (mask == 0u) {
        console_puts(" (yok)");
    }
    for (unsigned i = 0u; i < 9u; i++) {
        if ((mask & (1u << i)) != 0u) {
            console_puts(" ");
            console_puts(names[i]);
        }
    }
    console_line("");
}

int main(void)
{
    console_line("");
    console_line("===== LESSON 09 SOLUTION 01 =====");

    /* Pozitif kontrol: olcum aletinin kendisi temiz mi? */
    uint32_t res = 0u;
    uint32_t mask = abi_check_callee(good_callee, 7u, 5u, &res);
    console_kv_str("  CHECK_HARNESS_POSITIVE_CONTROL", pf(mask == 0u && res == 12u));

    /* scale_and_sum(10, 5) = helper_scale(10) + 5 = 31 + 5 = 36 */
    res = 0u;
    g_ex_sp_at_call = 0xFFFFFFFFu;
    mask = abi_check_callee(scale_and_sum, 10u, 5u, &res);

    console_kv_u32("  RESULT", res);
    console_kv_hex("  MASK", mask);
    print_mask_regs(mask);
    console_kv_hex("  SP_AT_BL", g_ex_sp_at_call);

    console_kv_str("  CHECK_RESULT", pf(res == 36u));
    console_kv_str("  CHECK_CALLEE_SAVED", pf(mask == 0u));
    console_kv_str("  CHECK_SP_ALIGNED_AT_CALL", pf((g_ex_sp_at_call & 7u) == 0u));

    console_line("===== END =====");
    for (;;) {
        __WFI();
    }
}
