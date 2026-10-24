#include "../sync.h"
#include "platform/mps2_an505.h"
#include "common/console.h"

static void check(const char *name, uint32_t yes)
{
    console_kv_str(name, yes ? "PASS" : "FAIL");
}

__attribute__((noinline, noipa)) void l13_done(void) { __NOP(); }

int main(void)
{
    console_line("===== LESSON 13: SYNCHRONIZATION AND DSP =====");
    l13_reset();
    uint32_t delivered = l13_racy_increment(&g_l13_counter);
    console_kv_u32("RACY_TOTAL", g_l13_counter);
    check("CHECK_LOST_UPDATE", delivered && g_l13_irq_count == 1U && g_l13_counter == 1U);

    l13_reset();
#if defined(L13_NO_RETRY)
    uint32_t success = l13_exclusive_increment(&g_l13_counter, 1U);
#else
    uint32_t success = l13_exclusive_increment(&g_l13_counter, 4U);
#endif
    console_kv_u32("EXCLUSIVE_TOTAL", g_l13_counter);
    console_kv_u32("FIRST_STREX_STATUS", g_l13_first_status);
    console_kv_u32("RETRIES", g_l13_retries);
    check("CHECK_INTERRUPTED_RESERVATION", g_l13_first_status == 1U && g_l13_retries == 1U);
    check("CHECK_ATOMIC_TOTAL", success && g_l13_counter == 2U && g_l13_irq_count == 1U);
    check("CHECK_DSP", l13_dsp_checks());
    check("CHECK_BASELINE_SOFT", SCB->CPACR == 0U && (__get_CONTROL() & 4U) == 0U);
    check("CHECK_NO_FAULT", SCB->CFSR == 0U && SCB->HFSR == 0U);
    console_line("===== END =====");
    l13_done();
    for (;;) __WFI();
}
