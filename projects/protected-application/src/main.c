#include "platform/mps2_an505.h"
#include "common/console.h"
#include "../../fault-analyzer/diagnostic.h"
typedef struct { uint32_t stack[256]; uint32_t results[8]; } app_t;
static app_t app __attribute__((aligned(32)));
static volatile uint32_t kernel_total;
void psp_start(uint32_t top, void (*entry)(void)) __attribute__((noreturn));
void mpu_store_word(volatile uint32_t *, uint32_t);
static void check(const char *key, uint32_t ok) { console_kv_str(key, ok ? "PASS" : "FAIL"); }
static uint32_t call(uint32_t arg)
{
    register uint32_t r0 __asm("r0") = arg;
    __asm volatile("svc #1" : "+r"(r0) :: "r1","r2","r3","r12","cc","memory");
    return r0;
}
static void finish(void)
{
    check("CHECK_UNPRIV_PSP", app.results[0] == 3U && app.results[1] == 0U);
    check("CHECK_SVC", app.results[2] == 1U && app.results[3] == 3U && app.results[4] == 6U);
    check("CHECK_REJECT", app.results[5] == 0xFFFFFFFFU && kernel_total == 6U);
    check("CHECK_HANDLER", __get_IPSR() != 0U);
}
void svc_dispatch(exception_frame_t *frame, uint32_t ret)
{
    fault_snapshot_t s = {0}; s.exc_return = ret;
    if (ret != 0xFFFFFFFDU || !fa_capture(&s,(uintptr_t)frame,(uintptr_t)app.stack,
                                       (uintptr_t)(app.stack+256)) ||
        frame->pc < 0x10000002U || frame->pc >= 0x10080000U) for (;;) __WFI();
    const uint16_t instruction = *(const uint16_t *)(uintptr_t)(frame->pc - 2U);
    if (instruction == 0xDF01U) {
        if (frame->r0 <= 31U) { kernel_total += frame->r0; frame->r0 = kernel_total; }
        else frame->r0 = 0xFFFFFFFFU;
    } else if (instruction == 0xDF02U) {
        finish();
        check("CHECK_NO_FAULT", SCB->CFSR == 0U && SCB->HFSR == 0U);
        console_line("===== END =====");
        for (;;) __WFI();
    } else frame->r0 = 0xFFFFFFFFU;
}
__attribute__((naked)) void MemManage_Handler(void)
{
    __asm volatile("mrs r0,psp\nmov r1,lr\nb protected_fault");
}
void protected_fault(exception_frame_t *frame, uint32_t ret)
{
    fault_snapshot_t s = {0}; fault_decision_t d;
    s.ipsr = __get_IPSR(); s.exc_return = ret; s.cfsr = SCB->CFSR;
    s.hfsr = SCB->HFSR; s.mmfar_raw = SCB->MMFAR;
    fa_capture(&s,(uintptr_t)frame,(uintptr_t)app.stack,(uintptr_t)(app.stack+256));
    fa_analyze(&s,&d);
    finish();
    check("CHECK_MPU_DENIAL", s.ipsr == 4U && s.cfsr == 0x82U && d.frame_safe &&
          d.address_valid && d.fault_address == (uint32_t)(uintptr_t)&kernel_total && kernel_total == 6U);
    check("CHECK_NOT_ESCALATED", s.hfsr == 0U);
    console_line("===== END =====");
    for (;;) __WFI();
}
static void thread(void)
{
    __set_CONTROL(3U); __ISB();
    app.results[0] = __get_CONTROL(); app.results[1] = __get_IPSR();
    app.results[2] = call(1U); app.results[3] = call(2U); app.results[4] = call(3U);
    app.results[5] = call(32U);
#ifdef APP_denied
    mpu_store_word(&kernel_total, 0xBADC0DEU);
#endif
    __asm volatile("svc #2" ::: "memory");
    for (;;) __NOP();
}
int main(void)
{
    console_line("===== PROJECT: PROTECTED COUNTER =====");
    ARM_MPU_Disable(); ARM_MPU_SetMemAttr(0U,0xFFU);
    ARM_MPU_SetRegion(0U, ARM_MPU_RBAR(0x10000000U,0U,1U,1U,0U), ARM_MPU_RLAR(0x1007FFFFU,0U));
    ARM_MPU_SetRegion(1U, ARM_MPU_RBAR((uint32_t)(uintptr_t)&app,0U,0U,1U,1U),
                      ARM_MPU_RLAR((uint32_t)(uintptr_t)&app+sizeof(app)-1U,0U));
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
    __set_PSPLIM((uint32_t)(uintptr_t)app.stack);
    psp_start((uint32_t)(uintptr_t)(app.stack+256),thread);
}
