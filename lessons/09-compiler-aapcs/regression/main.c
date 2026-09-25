/*
 * Lesson 09 - OLCUM ALETI REGRESYONU (ogrenciye yonelik degil)
 *
 * abi_check_callee'nin su durumlarda guvenle rapor verip normal
 * dondugunu kanitlar:
 *   - SP'yi +4 bozan callee           -> maske 0x100
 *   - SP'yi -8 bozan callee           -> maske 0x100
 *   - r4 VE SP'yi bozan callee        -> maske 0x101
 *   - pozitif kontrol (good_callee)   -> maske 0x000
 * Her durumda: sonuc (12) korunur, cagiranin r4-r11'i (0xDEAD0000..7)
 * ve SP'si geri gelir, END'e ulasilir. Eski harness bu hedefte
 * 0xDEAD0000'a yazmaya calisip BusFault uretiyordu.
 */
#include "platform/mps2_an505.h"
#include "common/console.h"
#include "abi.h"

volatile uint32_t g_harness_sp_at_call;

volatile uint32_t g_reg_result;
volatile uint32_t g_reg_sp_before;
volatile uint32_t g_reg_sp_after;
volatile uint32_t g_reg_after[8];

uint32_t sp_up_callee(uint32_t a, uint32_t b);
uint32_t sp_down_callee(uint32_t a, uint32_t b);
uint32_t sp_up_r4_callee(uint32_t a, uint32_t b);
uint32_t sp_regression_run(abi_fn2_t fn);

static const char *pf(int ok) { return ok ? "PASS" : "FAIL"; }

static void run_case(const char *name, abi_fn2_t fn, uint32_t expected_mask)
{
    g_reg_result = 0u;
    uint32_t mask = sp_regression_run(fn);

    int regs_ok = 1;
    for (unsigned i = 0u; i < 8u; i++) {
        if (g_reg_after[i] != (0xDEAD0000u + i)) {
            regs_ok = 0;
        }
    }

    console_line(name);
    console_kv_hex("  MASK", mask);
    console_kv_u32("  RESULT", g_reg_result);
    console_kv_hex("  CALLER_R4_AFTER", g_reg_after[0]);
    console_kv_hex("  CALLER_SP_BEFORE", g_reg_sp_before);
    console_kv_hex("  CALLER_SP_AFTER", g_reg_sp_after);
    console_kv_str("  CHECK_MASK", pf(mask == expected_mask));
    console_kv_str("  CHECK_RESULT", pf(g_reg_result == 12u));
    console_kv_str("  CHECK_CALLER_R4_R11_RESTORED", pf(regs_ok));
    console_kv_str("  CHECK_CALLER_SP_RESTORED", pf(g_reg_sp_before == g_reg_sp_after));
    console_kv_str("  CHECK_HARNESS_SP_ALIGNED", pf((g_harness_sp_at_call & 7u) == 0u));
}

int main(void)
{
    console_line("");
    console_line("===== LESSON 09: HARNESS SP REGRESSION =====");

    run_case("[1] good_callee (pozitif kontrol)", good_callee,     0x000u);
    run_case("[2] sp_up_callee (SP +4)",          sp_up_callee,    0x100u);
    run_case("[3] sp_down_callee (SP -8)",        sp_down_callee,  0x100u);
    run_case("[4] sp_up_r4_callee (r4 + SP +4)",  sp_up_r4_callee, 0x101u);

    console_line("===== END =====");
    for (;;) {
        __WFI();
    }
}
