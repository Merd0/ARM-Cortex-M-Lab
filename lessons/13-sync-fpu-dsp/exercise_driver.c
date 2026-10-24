#include "sync.h"
#include "platform/mps2_an505.h"
#include "common/console.h"

static uint32_t check(const char *name, uint32_t yes)
{
    console_kv_str(name, yes ? "PASS" : "FAIL");
    return !!yes;
}

int main(void)
{
    uint32_t passed = 0U;
    console_line("===== LESSON 13: CRITICAL UPDATE EXERCISE =====");
    l13_reset();
    l13_protected_increment(&g_l13_counter);
    passed += check("OPEN_MASK", __get_PRIMASK() == 0U);
    uint32_t delivered = l13_wait_update();
    passed += check("OPEN_IRQ", delivered && g_l13_irq_count == 1U);
    passed += check("OPEN_TOTAL", g_l13_counter == 2U);

    l13_reset();
    __disable_irq();
    l13_protected_increment(&g_l13_counter);
    passed += check("NESTED_MASK", __get_PRIMASK() == 1U && g_l13_irq_count == 0U);
    __enable_irq();
    __ISB();
    delivered = l13_wait_update();
    passed += check("NESTED_IRQ", delivered && g_l13_irq_count == 1U);
    passed += check("NESTED_TOTAL", g_l13_counter == 2U);
    console_kv_u32("CASES_PASSED", passed);
    console_line("===== END =====");
    for (;;) __WFI();
}
