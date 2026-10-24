#include "platform/mps2_an505.h"
#include "common/console.h"

uint32_t l13_fp_run(uint32_t active);
uint32_t l13_abi_call_bits(uint32_t a, uint32_t b);
void l13_fp_set_environment(uint32_t fpscr);
uint32_t l13_fp_get_environment(void);
extern char l13_after_svc, l13_fp_first;

volatile uint32_t g_l13_before_sp, g_l13_entry_sp, g_l13_exc_return;
volatile uint32_t g_l13_thread_control, g_l13_fpccr_entry, g_l13_fpccr_after;
volatile uint32_t g_l13_fp_slot_before;
volatile uint32_t g_l13_fp_active;
volatile uint32_t g_l13_core[8], g_l13_fp_saved[17];
volatile uint32_t g_l13_return_bits, g_l13_cfsr, g_l13_hfsr;
volatile uint32_t g_l13_return_fpscr;

static void check(const char *name, uint32_t yes)
{
    console_kv_str(name, yes ? "PASS" : "FAIL");
}

__attribute__((noinline, noipa)) void l13_done(void) { __NOP(); }

/* Only our known Secure->Secure, MSP-origin frame; no generic frame decoder.
 * FPCCR.TS=0 fixes the extra FP payload to S0-S15/FPSCR/reserved. */
void l13_capture_frame(const uint32_t *frame)
{
    for (uint32_t i = 0U; i < 8U; ++i) g_l13_core[i] = frame[i];
    if ((g_l13_exc_return & 0x10U) == 0U) {
        for (uint32_t i = 0U; i < 17U; ++i) g_l13_fp_saved[i] = frame[8U + i];
    }
}

__attribute__((noreturn)) void l13_nocp_report(const uint32_t *frame, uint32_t exc_return)
{
    g_l13_cfsr = SCB->CFSR;
    g_l13_hfsr = SCB->HFSR;
    g_l13_entry_sp = (uint32_t)frame;
    g_l13_exc_return = exc_return;
    for (uint32_t i = 0U; i < 8U; ++i) g_l13_core[i] = frame[i];
    console_kv_hex("CFSR", g_l13_cfsr);
    console_kv_hex("HFSR", g_l13_hfsr);
    console_kv_hex("STACKED_PC", frame[6]);
    console_kv_hex("EXC_RETURN", exc_return);
    check("CHECK_NOCP", g_l13_cfsr == SCB_CFSR_NOCP_Msk && g_l13_hfsr == 0U);
    check("CHECK_USAGE_HANDLER", __get_IPSR() == 6U);
    check("CHECK_DENIED_PC", frame[6] == ((uint32_t)&l13_fp_first & ~1U));
    check("CHECK_DENIED_BASIC", (exc_return & 0x1CU) == 0x18U && SCB->CPACR == 0U);
    console_line("===== END =====");
    l13_done();
    for (;;) __WFI();
}

int main(void)
{
    console_line("===== LESSON 13: FPU PERMISSION AND CONTEXT =====");
    console_kv_hex("MVFR0", FPU->MVFR0);
    console_kv_hex("CPACR_BEFORE", SCB->CPACR);
    check("CHECK_RESET_PERMISSION", SCB->CPACR == 0U);
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;
#if defined(L13_FP_denied)
    (void)l13_fp_run(1U);          /* First FP instruction must NOCP-fault. */
    check("CHECK_UNEXPECTED_RETURN", 0U);
#else
    SCB->CPACR |= (3U << 20) | (3U << 22);
    __DSB();
    __ISB();
    uint32_t config = FPU->FPCCR;
    config &= ~(FPU_FPCCR_LSPEN_Msk | FPU_FPCCR_TS_Msk | FPU_FPCCR_CLRONRET_Msk);
    config |= FPU_FPCCR_ASPEN_Msk;
#if defined(L13_FP_lazy)
    config |= FPU_FPCCR_LSPEN_Msk;
#endif
    FPU->FPCCR = config;
    __DSB();
    __ISB();
#if defined(L13_FP_basic)
    g_l13_fp_active = 0U;
#else
    g_l13_fp_active = 1U;
    /* Caller explicitly selects FZ=1 through an FP environment support function.
     * The worker must preserve control state; all arithmetic uses normal values. */
    l13_fp_set_environment(1U << 24);
#endif
    g_l13_return_bits = l13_fp_run(g_l13_fp_active);
    if (g_l13_fp_active != 0U) g_l13_return_fpscr = l13_fp_get_environment();
    uint32_t bytes = g_l13_before_sp - g_l13_entry_sp;
    uint32_t extended = (g_l13_exc_return & 0x10U) == 0U;
    console_kv_hex("CPACR_AFTER", SCB->CPACR);
    console_kv_hex("THREAD_CONTROL", g_l13_thread_control);
    console_kv_hex("EXC_RETURN", g_l13_exc_return);
    console_kv_u32("FRAME_BYTES", bytes);
    console_kv_hex("FPCCR_ENTRY", g_l13_fpccr_entry);
    console_kv_hex("FPCCR_AFTER_FP", g_l13_fpccr_after);
    console_kv_hex("RAW_FP_SLOT_BEFORE", g_l13_fp_slot_before);
    console_kv_hex("RESTORED_S0", g_l13_return_bits);
    console_kv_hex("RESTORED_FPSCR", g_l13_return_fpscr);
    check("CHECK_PERMISSION", (SCB->CPACR & 0x00F00000U) == 0x00F00000U);
    check("CHECK_FRAME_TYPE", extended == g_l13_fp_active &&
          ((g_l13_thread_control >> 2) & 1U) == g_l13_fp_active);
    check("CHECK_STACK_LAYOUT", bytes == (extended ? 104U : 32U) &&
          (g_l13_entry_sp & 7U) == 0U && (g_l13_core[7] & 0x200U) == 0U &&
          g_l13_core[6] == ((uint32_t)&l13_after_svc & ~1U) &&
          (g_l13_exc_return & 0x2CU) == 0x28U);
#if defined(L13_FP_lazy)
    check("CHECK_LAZY_STATE", (g_l13_fpccr_entry & 1U) == 1U &&
          (g_l13_fpccr_after & 1U) == 0U && g_l13_fp_slot_before == 0xA5A5A5A5U);
#else
    check("CHECK_LAZY_STATE", (g_l13_fpccr_entry & 1U) == 0U &&
          (g_l13_fpccr_after & 1U) == 0U &&
          g_l13_fp_slot_before == (extended ? 0x3FC00000U : 0U));
#endif
    check("CHECK_FP_PAYLOAD", !extended || (g_l13_fp_saved[0] == 0x3FC00000U &&
          g_l13_fp_saved[1] == 0x40000000U && g_l13_fp_saved[2] == 0x40600000U));
    check("CHECK_FP_RESTORE", g_l13_return_bits == (extended ? 0x3FC00000U : 0U));
    check("CHECK_FP_CONTROLS", !extended || (g_l13_fp_saved[16] == (1U << 24) &&
          g_l13_return_fpscr == (1U << 24)));
    uint32_t result = l13_abi_call_bits(0x3FC00000U, 0x40000000U);
    console_kv_hex("ABI_ADD_BITS", result);
    check("CHECK_FLOAT_ABI", result == 0x40600000U);
    check("CHECK_NO_FAULT", SCB->CFSR == 0U && SCB->HFSR == 0U);
#endif
    console_line("===== END =====");
    l13_done();
    for (;;) __WFI();
}
