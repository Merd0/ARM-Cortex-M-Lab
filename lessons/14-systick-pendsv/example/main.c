#include "platform/mps2_an505.h"
#include "common/console.h"

void l14_priorities(void);
volatile uint32_t g_l14_phase, g_l14_inside_tick, g_l14_nested;
volatile uint32_t g_l14_ticks, g_l14_dispatches, g_l14_events[3], g_l14_used;

static void event(uint32_t id)
{
    uint32_t index = g_l14_used;
    if (index < 3U) g_l14_events[index] = id;
    g_l14_used = index + 1U;
}

void PendSV_Handler(void)
{
    if (g_l14_phase == 1U) {
        g_l14_nested = g_l14_inside_tick;
        event(3U);
    }
    ++g_l14_dispatches;
}

void SysTick_Handler(void)
{
    if (g_l14_phase == 1U) {
        g_l14_inside_tick = 1U;
        event(1U);
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
        __DSB();
        __ISB();
        event(2U);
        g_l14_inside_tick = 0U;
    } else if (g_l14_phase == 3U) {
        ++g_l14_ticks;
        if (g_l14_ticks >= 4U) SysTick->CTRL = 0U;
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
        __SEV(); /* Sticky event closes the condition-check / sleep race. */
    }
}

static void check(const char *key, uint32_t good)
{
    console_kv_str(key, good ? "PASS" : "FAIL");
}

int main(void)
{
    console_line("===== LESSON 14: TICK AND DEFERRED WORK =====");
    l14_priorities();
    check("CHECK_PRIORITY", NVIC_GetPriority(SysTick_IRQn) == 64U &&
          NVIC_GetPriority(PendSV_IRQn) == 255U);
    g_l14_phase = 1U;
    SCB->ICSR = SCB_ICSR_PENDSTSET_Msk;
    __DSB(); __ISB();
    for (uint32_t i = 0U; i < 10000U && g_l14_dispatches == 0U; ++i) __NOP();
    console_kv_u32("EVENT0", g_l14_events[0]);
    console_kv_u32("EVENT1", g_l14_events[1]);
    console_kv_u32("EVENT2", g_l14_events[2]);
    check("CHECK_DEFERRED", g_l14_used == 3U && g_l14_events[0] == 1U &&
          g_l14_events[1] == 2U && g_l14_events[2] == 3U && g_l14_nested == 0U);

    g_l14_phase = 2U;
    uint32_t before = g_l14_dispatches;
    uint32_t mask = __get_PRIMASK();
    __disable_irq();
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    __set_PRIMASK(mask);
    __DSB(); __ISB();
    for (uint32_t i = 0U; i < 10000U && g_l14_dispatches == before; ++i) __NOP();
    check("CHECK_PENDING_COALESCES", g_l14_dispatches == before + 1U);

    g_l14_phase = 3U;
    before = g_l14_dispatches;
    uint32_t configured = SysTick_Config(SystemCoreClock / 1000U) == 0U;
    l14_priorities(); /* SysTick_Config itself writes the SysTick priority. */
    /* Iteration budgets depend on optimization/host speed, not timer time.
     * SEV in the ISR also covers a final tick between this check and WFE.
     * The external smoke runner bounds a genuinely stalled timer. */
    while (g_l14_ticks < 4U) __WFE();
    SysTick->CTRL = 0U;
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
    __DSB(); __ISB();
    console_kv_u32("TICKS", g_l14_ticks);
    check("CHECK_TICK_RELOAD", configured && SysTick->LOAD == 19999U);
    check("CHECK_TICK_PROGRESS", g_l14_ticks == 4U && g_l14_dispatches > before &&
          g_l14_dispatches - before <= g_l14_ticks);
    check("CHECK_THREAD_MSP", __get_IPSR() == 0U && (__get_CONTROL() & 7U) == 0U);
    check("CHECK_NO_FAULT", SCB->CFSR == 0U && SCB->HFSR == 0U);
    console_line("===== END =====");
    for (;;) __WFI();
}
