#include "../kernel.h"
#include "platform/mps2_an505.h"
#include "common/console.h"

#define GUARD 0xC0DEF00DU
typedef struct {
    uint32_t lower[2];
    uint32_t words[L15_WORDS];
    uint32_t upper[2];
} stack_t;
static stack_t stacks[2] __attribute__((aligned(8)));
static task_t tasks[2];
task_t *g_l15_current;
volatile uint32_t g_l15_ticks, g_l15_switches, g_l15_yields, g_l15_async;
volatile uint32_t g_l15_runs[2], g_l15_bad[2], g_l15_booted;
static volatile uint32_t tick_pending;
extern uint32_t __stack_top;

static void check(const char *key, uint32_t ok)
{
    console_kv_str(key, ok ? "PASS" : "FAIL");
}
static uint32_t guards(void)
{
    for (uint32_t i = 0; i < 2U; ++i)
        if (stacks[i].lower[0] != GUARD || stacks[i].lower[1] != GUARD ||
            stacks[i].upper[0] != GUARD || stacks[i].upper[1] != GUARD) return 0U;
    return 1U;
}
static uint32_t valid(const task_t *t)
{
    uintptr_t sp = (uintptr_t)t->sp;
    if (sp < (uintptr_t)t->low || sp > (uintptr_t)t->high - L15_CONTEXT_WORDS * 4U)
        return 0U;
    if (((sp + L15_SAVED_WORDS * 4U) & 7U) != 0U) return 0U;
    return t->sp[8] == L15_EXC_RETURN && (t->sp[16] & 0x01000000U) != 0U;
}

void SysTick_Handler(void)
{
    ++g_l15_ticks;
    tick_pending = 1U;
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}
void l15_yield(void)
{
    ++g_l15_yields;
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    __DSB(); __ISB();
}
task_t *l15_schedule(void)
{
    task_t *old = g_l15_current;
    task_t *next = old == &tasks[0] ? &tasks[1] : &tasks[0];
    if (!guards() || !valid(old) || !valid(next)) l15_panic();
    if (tick_pending && old->sp[15] != ((uint32_t)(uintptr_t)&l15_after_yield & ~1U))
        ++g_l15_async;
    tick_pending = 0U;
    ++g_l15_switches;
    g_l15_current = next;
    return next;
}
uint32_t l15_observe(uint32_t id, uint32_t registers_ok)
{
    /* ASM masks interrupts only across this C call. The broken-save experiment
     * therefore corrupts canaries in ASM, not a suspended C pointer/frame. */
    if (id > 1U) l15_panic();
    ++g_l15_runs[id];
    if (!registers_ok) g_l15_bad[id] = 1U;
    if ((g_l15_runs[0] >= 12U && g_l15_runs[1] >= 12U &&
         g_l15_ticks >= 4U && g_l15_async >= 2U) ||
        g_l15_ticks >= 16U || g_l15_switches >= 50000U) return 1U;
    return g_l15_runs[id] <= 8U ? 2U : 0U;
}
void l15_task_exit(void) { l15_panic(); }

void l15_panic(void)
{
    __disable_irq();
    SysTick->CTRL = 0U;
    console_kv_str("CHECK_KERNEL_STATE", "FAIL");
    console_line("===== END =====");
    for (;;) __WFI();
}
void l15_finish(void)
{
    __disable_irq();
    SysTick->CTRL = 0U;
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
    console_kv_u32("TASK0_RUNS", g_l15_runs[0]);
    console_kv_u32("TASK1_RUNS", g_l15_runs[1]);
    console_kv_u32("TICKS", g_l15_ticks);
    console_kv_u32("SWITCHES", g_l15_switches);
    console_kv_u32("ASYNC_SWITCHES", g_l15_async);
    check("CHECK_INITIAL_FRAME", 1U);
    check("CHECK_TASK_PROGRESS", g_l15_runs[0] >= 12U && g_l15_runs[1] >= 12U);
    check("CHECK_REGISTERS", g_l15_bad[0] == 0U && g_l15_bad[1] == 0U);
    check("CHECK_STACK_GUARDS", guards());
    check("CHECK_PREEMPTION", g_l15_ticks >= 4U && g_l15_async >= 2U);
    check("CHECK_YIELD", g_l15_yields >= 16U);
    check("CHECK_HANDLER_MSP", __get_IPSR() == 11U &&
          __get_MSP() <= (uint32_t)(uintptr_t)&__stack_top &&
          __get_MSP() > (uint32_t)(uintptr_t)&__stack_top - 4096U);
    check("CHECK_NO_FAULT", SCB->CFSR == 0U && SCB->HFSR == 0U);
    console_line("===== END =====");
    for (;;) __WFI();
}

int main(void)
{
    console_line("===== LESSON 15: TWO TASK PSP KERNEL =====");
    for (uint32_t i = 0; i < 2U; ++i) {
        stacks[i].lower[0] = stacks[i].lower[1] = GUARD;
        stacks[i].upper[0] = stacks[i].upper[1] = GUARD;
        for (uint32_t j = 0; j < L15_WORDS; ++j) stacks[i].words[j] = 0xCCCCCCCCU;
        tasks[i].low = stacks[i].words;
        tasks[i].high = stacks[i].words + L15_WORDS;
    }
    for (uint32_t i = 0; i < 2U; ++i) {
        tasks[i].sp = l15_initial_frame(tasks[i].high, i,
            (uintptr_t)&l15_task_entry, (uintptr_t)&l15_task_exit);
        if (!valid(&tasks[i]) || tasks[i].sp[9] != i ||
            tasks[i].sp[15] != ((uint32_t)(uintptr_t)&l15_task_entry & ~1U) ||
            tasks[i].sp[14] != ((uint32_t)(uintptr_t)&l15_task_exit | 1U)) {
            check("CHECK_INITIAL_FRAME", 0U);
            check("CHECK_NO_LAUNCH", g_l15_booted == 0U);
            check("CHECK_STACK_GUARDS", guards());
            check("CHECK_NO_FAULT", SCB->CFSR == 0U && SCB->HFSR == 0U);
            console_line("===== END =====");
            for (;;) __WFI();
        }
    }
    g_l15_current = &tasks[0];
    NVIC_SetPriority(SVCall_IRQn, 0U);
    NVIC_SetPriority(SysTick_IRQn, 64U);
    NVIC_SetPriority(PendSV_IRQn, 255U);
    SysTick->LOAD = SystemCoreClock / 1000U - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = 0U; /* Enabled only after bootstrap has installed PSP. */
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk |
                  SCB_SHCSR_MEMFAULTENA_Msk;
    l15_start();
}
