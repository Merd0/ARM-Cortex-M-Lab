#include "sync.h"
#include "platform/mps2_an505.h"

_Alignas(4) volatile uint32_t g_l13_counter;
volatile uint32_t g_l13_irq_count, g_l13_retries, g_l13_first_status;

void SysTick_Handler(void)
{
    ++g_l13_counter;
    ++g_l13_irq_count;
}

void l13_reset(void)
{
    /* Called only with PRIMASK=0 and no active/pending SysTick in this lab. */
    SysTick->CTRL = 0U;
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
    __DSB();
    __ISB();
    g_l13_counter = 0U;
    g_l13_irq_count = 0U;
    g_l13_retries = 0U;
    g_l13_first_status = 0xFFFFFFFFU;
}

void l13_pend_update(void)
{
    /* Explicit pending exception: no periodic timer or scheduler here. */
    SCB->ICSR = SCB_ICSR_PENDSTSET_Msk;
    __DSB();
    __ISB();
}

uint32_t l13_wait_update(void)
{
    for (uint32_t i = 0U; i < 1024U; ++i) {
        if (g_l13_irq_count != 0U) return 1U;
    }
    return 0U;
}
