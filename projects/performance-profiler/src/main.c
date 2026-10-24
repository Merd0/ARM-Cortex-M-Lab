#include "platform/mps2_an505.h"
#include "common/console.h"
#include "../../../lessons/12-debug-dwt/measurement.h"
uint32_t l12_workload(const uint32_t *, uint32_t);
static volatile uint32_t ticks, sink;
void SysTick_Handler(void) { ++ticks; }
static void check(const char *key,uint32_t ok) { console_kv_str(key,ok?"PASS":"FAIL"); }
int main(void)
{
    uint32_t data[16]; for (uint32_t i=0U;i<16U;++i) data[i]=i+1U;
    console_line("===== PROJECT: PERFORMANCE PROFILER =====");
    uint32_t demcr=DCB->DEMCR, ctrl=DWT->CTRL;
    DCB->DEMCR=demcr|DCB_DEMCR_TRCENA_Msk;
    if (!(ctrl&DWT_CTRL_NOCYCCNT_Msk)) { DWT->CYCCNT=0U; DWT->CTRL=ctrl|1U; }
    __DSB(); __ISB();
    struct l12_sample sample = {DWT->CTRL,DCB->DEMCR,DWT->CYCCNT,0U,0U};
    for (uint32_t i=0U;i<32U;++i) sink=l12_workload(data,16U);
    sample.end=DWT->CYCCNT;
    DWT->CTRL=ctrl; DCB->DEMCR=demcr;
    enum l12_kind kind=l12_classify(&sample);
    console_kv_str("DWT_RESULT",kind==L12_COUNTER_DELTA?"COUNTER_ADVANCED":"UNAVAILABLE");
    check("CHECK_CLASSIFICATION", kind==L12_COUNTER_DELTA || kind==L12_UNAVAILABLE);
    check("CHECK_WORKLOAD",sink==424U);
    uint32_t configured=SysTick_Config(SystemCoreClock/1000U)==0U;
    uint32_t iterations=0U;
    while(ticks<4U && iterations<200000U) { sink=l12_workload(data,16U); ++iterations; }
    SysTick->CTRL=0U; SCB->ICSR=SCB_ICSR_PENDSTCLR_Msk; __DSB(); __ISB();
    console_kv_u32("LOCAL_TICKS",ticks); console_kv_u32("ITERATIONS",iterations);
    check("CHECK_PROGRESS",configured && ticks>=4U && iterations>0U);
    check("CHECK_NO_FAULT",SCB->CFSR==0U && SCB->HFSR==0U);
    console_line("[SCOPE] Model-local progress, static size/instructions; no speedup or WCET.");
    console_line("===== END =====");
    for (;;) __WFI();
}
