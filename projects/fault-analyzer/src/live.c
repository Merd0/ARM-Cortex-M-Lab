#include "../diagnostic.h"
#include "platform/mps2_an505.h"
#include "common/console.h"
static uint32_t app_stack[256] __attribute__((aligned(32)));
#ifdef FA_mpu
static volatile uint32_t protected_word = 0x12345678U;
#endif
void psp_start(uint32_t top, void (*entry)(void)) __attribute__((noreturn));
static void check(const char *key, uint32_t good) { console_kv_str(key, good ? "PASS" : "FAIL"); }
void fa_live_record(exception_frame_t *frame, uint32_t ret)
{
    fault_snapshot_t s = {0}; fault_decision_t d;
    s.ipsr = __get_IPSR(); s.exc_return = ret;
    s.cfsr = SCB->CFSR; s.hfsr = SCB->HFSR;
    s.mmfar_raw = SCB->MMFAR; s.bfar_raw = SCB->BFAR;
    uintptr_t low = ret & 4U ? (uintptr_t)app_stack : 0x30007000U;
    uintptr_t high = ret & 4U ? (uintptr_t)(app_stack + 256) : 0x30008000U;
    fa_capture(&s, (uintptr_t)frame, low, high); fa_analyze(&s, &d);
    console_kv_str("CLASS", d.fault_class); console_kv_str("CAUSE", d.primary_bit);
    console_kv_hex("EXC_RETURN", ret); console_kv_hex("CFSR", s.cfsr);
    console_kv_hex("HFSR", s.hfsr); console_kv_hex("PC", d.stacked_pc);
#if defined(FA_udf)
    check("CHECK_CAUSE", s.ipsr == 6U && s.cfsr == SCB_CFSR_UNDEFINSTR_Msk);
#elif defined(FA_unaligned)
    check("CHECK_CAUSE", s.ipsr == 6U && s.cfsr == SCB_CFSR_UNALIGNED_Msk);
#elif defined(FA_mpu)
    check("CHECK_CAUSE", s.ipsr == 4U && s.cfsr == 0x82U && d.address_valid &&
          d.fault_address == (uint32_t)(uintptr_t)&protected_word);
#elif defined(FA_escalated)
    check("CHECK_CAUSE", s.ipsr == 3U && s.cfsr == 0x8200U && d.escalated);
#else
    check("CHECK_CAUSE", s.ipsr == 5U && s.cfsr == 0x8200U && !d.escalated);
#endif
    check("CHECK_FRAME", d.frame_safe && d.stacked_pc >= 0x10000000U && d.stacked_pc < 0x10080000U);
    check("CHECK_PRIVILEGE", __get_IPSR() != 0U); /* Handler privilege is unconditional. */
    SCB->CFSR = s.cfsr;
    SCB->HFSR = SCB_HFSR_DEBUGEVT_Msk | (s.hfsr & (SCB_HFSR_FORCED_Msk | SCB_HFSR_VECTTBL_Msk));
    __DSB();
    check("CHECK_CLEAR", SCB->CFSR == 0U && SCB->HFSR == 0U);
    console_line("===== END =====");
    for (;;) __WFI();
}
#ifdef FA_mpu
static void denied(void)
{
    __set_CONTROL(3U); __ISB();
    protected_word = 7U;
    for (;;) __NOP();
}
#endif
int main(void)
{
    console_line("===== PROJECT: LIVE FAULT =====");
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_USGFAULTENA_Msk;
#if defined(FA_mpu)
    ARM_MPU_Disable();
    ARM_MPU_SetMemAttr(0U, 0xFFU);
    ARM_MPU_SetRegion(0U, ARM_MPU_RBAR(0x10000000U, 0U, 1U, 1U, 0U), ARM_MPU_RLAR(0x1007FFFFU,0U));
    ARM_MPU_SetRegion(1U, ARM_MPU_RBAR((uint32_t)(uintptr_t)app_stack,0U,0U,1U,1U),
                      ARM_MPU_RLAR((uint32_t)(uintptr_t)(app_stack+256)-1U,0U));
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
    psp_start((uint32_t)(uintptr_t)(app_stack+256), denied);
#elif defined(FA_udf)
    __asm volatile("udf #0");
#elif defined(FA_unaligned)
    SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
    uint32_t addr = 0x10000001U;
    __asm volatile("ldr r0,[%0]" :: "r"(addr) : "r0","memory");
#else
#if !defined(FA_escalated)
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
#endif
    uint32_t addr = 0x30100000U;
    __asm volatile("ldr r0,[%0]" :: "r"(addr) : "r0","memory");
#endif
    for (;;) __WFI();
}
