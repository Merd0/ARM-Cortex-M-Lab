#include "platform/mps2_an505.h"
#include "platform/board.h"
#include "common/console.h"
#include <arm_cmse.h>
uint32_t node_calibrate(uint32_t value);
static uint32_t processed, bad_result, denied_diagnostics;
__attribute__((noinline,noipa)) void node_gateway_resumed(void) { __NOP(); }
void l16_platform_ready(void)
{
    SCB->AIRCR=(0x5FAU<<16)|SCB_AIRCR_PRIS_Msk|SCB_AIRCR_BFHFNMINS_Msk;
    SCB->CPACR |= (0xFU<<20); /* Secure only; NS FP access stays disabled. */
    SCB_NS->CPACR=0U;
    SCB->NSACR &= ~((1U<<10)|(1U<<11));
    __DSB(); __ISB();
}
uint32_t l16_transform(uint32_t value)
{
    /* This hook is reached only from a privileged SVC Handler. The C driver
     * and CMSE wrapper are general-register-only; the bounded ASM leaf owns
     * S0/S1, preserves FPSCR, erases its FP payload, then this hook restores
     * the pre-call FPCA/SFPA state before returning to the integer gateway. */
    uint32_t control=__get_CONTROL();
    uint32_t result=node_calibrate(value);
    __set_CONTROL(control);
    __ISB();
    ++processed;
    if(result!=(value*3U)/2U) bad_result=1U;
    return result;
}
static void done(void) __attribute__((noreturn));
static void done(void)
{
    console_line("===== END =====");
    for (;;) __WFI();
}
uint32_t l16_observation(uint32_t value, uint32_t caller_ns)
{
    /* CONTROL.nPRIV describes Thread, not Handler privilege. A Secure gateway
     * keeps IPSR; the caller flag itself was captured at the actual entry. */
    if(!caller_ns || (__get_IPSR()==0U && (__TZ_get_CONTROL_NS()&1U))) {
        ++denied_diagnostics;
#if NODE_TEST == 6
        /* Controlled exception fixture, independent of host timing/optimization:
         * a real NS SysTick/PendSV arrives while Secure Thread owns the call. */
        if(denied_diagnostics==1U) {
            SCB_NS->ICSR=SCB_ICSR_PENDSTSET_Msk;
            __DSB(); __ISB();
            node_gateway_resumed();
        }
#endif
        return 1U;
    }
    if((value&0xFFFFFF00U)==0x55000000U) {
        board_uart_putc((char)(value&0xFFU)); return 1U;
    }
    if(value==0x7E000000U) {
        SysTick_NS->CTRL=0U;
        console_kv_u32("SECURE_PROCESS_CALLS",processed);
        console_kv_str("CHECK_SECURE_PROCESSING",processed>0U && !bad_result?"PASS":"FAIL");
        console_kv_str("CHECK_NS_FPU_DISABLED",SCB_NS->CPACR==0U &&
            (SCB->NSACR&((1U<<10)|(1U<<11)))==0U?"PASS":"FAIL");
        console_kv_str("CHECK_SECURE_CALLER",caller_ns?"PASS":"FAIL");
#if NODE_TEST == 6
        console_kv_str("CHECK_DIAGNOSTIC_POLICY",denied_diagnostics>=2U?"PASS":"FAIL");
#endif
        console_line("[AUTHENTICATION] NONE: calibration service, no signature or cryptographic tag");
        done();
    }
    return 1U; /* Unknown diagnostic requests have no effect. */
}
void l16_fault_hook(uint32_t status)
{
    SysTick_NS->CTRL=0U;
    console_kv_hex("SFSR",status);
    if(status&(1U<<6)) console_kv_hex("SFAR",*(volatile uint32_t *)0xE000EDE8U);
    else console_kv_str("SFAR","INVALID");
    console_kv_str("CHECK_EXPECTED_SECURE_FAULT",NODE_TEST==3 &&
        (status&~0x40U)==8U && processed>0U?"PASS":"FAIL");
    console_kv_str("CHECK_SECURE_HANDLER",__get_IPSR()==7U?"PASS":"FAIL");
    console_kv_str("CHECK_NO_ESCALATION",SCB->HFSR==0U?"PASS":"FAIL");
    done();
}
void HardFault_Handler(void)
{
    SysTick_NS->CTRL=0U;
    console_kv_hex("SFSR",*(volatile uint32_t *)0xE000EDE4U);
    console_kv_hex("HFSR",SCB->HFSR);
    console_kv_str("FRAME","UNSUPPORTED: cross-security capture omitted");
    console_kv_str("CHECK_UNEXPECTED_SECURE_HARDFAULT","FAIL");
    done();
}
