#include "platform/mps2_an505.h"
#include "common/console.h"

extern volatile uint32_t g_l12_watch;
uint32_t l12_outer(uint32_t value);
uint32_t l12_workload(const uint32_t *input, uint32_t count);

volatile uint32_t g_l12_seed = 1U;
volatile uint32_t g_l12_sink;
volatile uint32_t g_l12_ticks;
volatile uint32_t g_l12_iterations;
volatile uint32_t g_l12_io_calls;
volatile uint32_t g_l12_dwt_start, g_l12_dwt_end;
volatile uint32_t g_l12_dwt_ctrl, g_l12_demcr;

void SysTick_Handler(void) { ++g_l12_ticks; }

static void check(const char *name, uint32_t yes)
{
    console_kv_str(name, yes ? "PASS" : "FAIL");
}

/* Stable breakpoint after UART output and timer shutdown. */
__attribute__((noinline, noipa)) void l12_done(void)
{
    __asm volatile ("nop" ::: "memory");
}

int main(void)
{
    uint32_t input[16];
    uint32_t seed = g_l12_seed;
    for (uint32_t i = 0U; i < 16U; ++i) input[i] = i + seed;
    console_line("===== LESSON 12: DEBUG AND MEASUREMENT =====");

    (void)l12_outer(42U);
    console_kv_u32("WATCH_VALUE", g_l12_watch);
    check("CHECK_WATCH_VALUE", g_l12_watch == 42U);

    /* This example is Secure/privileged and target-specific. Register reads
     * alone cannot prove implemented timing behavior. Preserve prior config. */
    uint32_t saved_demcr = DCB->DEMCR;
    uint32_t saved_ctrl = DWT->CTRL;
    DCB->DEMCR = saved_demcr | DCB_DEMCR_TRCENA_Msk;
    if ((saved_ctrl & DWT_CTRL_NOCYCCNT_Msk) == 0U) {
        DWT->CYCCNT = 0U;
        DWT->CTRL = saved_ctrl | DWT_CTRL_CYCCNTENA_Msk;
    }
    __DSB();
    __ISB();
    g_l12_demcr = DCB->DEMCR;
    g_l12_dwt_ctrl = DWT->CTRL;
    g_l12_dwt_start = DWT->CYCCNT;
    for (uint32_t i = 0U; i < 32U; ++i)
        g_l12_sink = l12_workload(input, 16U);
    g_l12_dwt_end = DWT->CYCCNT;
    DWT->CTRL = saved_ctrl;
    DCB->DEMCR = saved_demcr;
    console_kv_hex("DEMCR_READBACK", g_l12_demcr);
    console_kv_hex("DWT_CTRL_READBACK", g_l12_dwt_ctrl);
    console_kv_u32("DWT_T0", g_l12_dwt_start);
    console_kv_u32("DWT_T1", g_l12_dwt_end);
    console_kv_str("DWT_CHANGED", g_l12_dwt_start != g_l12_dwt_end ? "YES" : "NO");
    console_line("[CYCLE_BENCHMARK] NOT VALIDATED ON QEMU");
    console_kv_u32("CHECKSUM", g_l12_sink);
    check("CHECK_WORKLOAD", g_l12_sink == 424U);

    /* Local timer progress experiment, NOT CPU cycles / hardware throughput.
     * No WFI between condition and work: bounded polling also detects a stopped
     * timer. No exact iteration count or speed ranking is asserted. */
    g_l12_ticks = 0U;
    uint32_t configured = SysTick_Config(SystemCoreClock / 1000U) == 0U;
    if (configured) {
        while (g_l12_ticks < 4U && g_l12_iterations < 200000U) {
            g_l12_sink = l12_workload(input, 16U);
#if defined(L12_IO_IN_WINDOW)
            if ((g_l12_iterations & 255U) == 0U) {
                ++g_l12_io_calls;
                console_putc('.');
            }
#endif
            ++g_l12_iterations;
        }
    }
    SysTick->CTRL = 0U;
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
    __DSB();
    __ISB();
    console_line("");
    check("CHECK_TIMER_CONFIG", configured);
    console_kv_u32("LOCAL_TICKS", g_l12_ticks);
    console_kv_u32("LOCAL_ITERATIONS", g_l12_iterations);
    console_kv_u32("IO_CALLS_IN_WINDOW", g_l12_io_calls);
    check("CHECK_TIMER_PROGRESS", g_l12_ticks >= 4U && g_l12_iterations > 0U);
    check("CHECK_CLEAN_WINDOW", g_l12_io_calls == 0U);
    check("CHECK_NO_FAULT", SCB->CFSR == 0U && SCB->HFSR == 0U);
    console_line("[TIMING_SCOPE] QEMU local ticks; no cycles, speed ranking or WCET");
    console_line("===== END =====");
    l12_done();
    for (;;) __WFI();
}
