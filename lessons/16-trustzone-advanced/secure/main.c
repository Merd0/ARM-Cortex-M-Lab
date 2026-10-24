#include "../api.h"
#include "platform/mps2_an505.h"
#include "common/console.h"
#include <arm_cmse.h>

#define REG(address) (*(volatile uint32_t *)(address))
#define SFSR REG(0xE000EDE4U)
#define SFAR REG(0xE000EDE8U)
static volatile uint32_t secret __attribute__((section(".secret"))) = 0x13579BDFU;
static volatile uint32_t calls, from_ns, reported;
extern char __nsc_start, __nsc_end;
typedef void (*ns_entry_t)(void) __attribute__((cmse_nonsecure_call));

static void check(const char *name, uint32_t ok)
{
    console_kv_str(name, ok ? "PASS" : "FAIL");
}
static void end(void) __attribute__((noreturn));
static void end(void)
{
    console_line("===== END =====");
    for (;;) __WFI();
}
__attribute__((cmse_nonsecure_entry))
uint32_t l16_service(uint32_t value)
{
    ++calls;
    if (cmse_nonsecure_caller()) ++from_ns;
    if (!l16_allowed(value)) return L16_DENIED;
    return value * 2U + 1U;
}
__attribute__((cmse_nonsecure_entry))
void l16_report(uint32_t observations)
{
    if (observations & 0x80000000U) {
        check("CHECK_SECURE_REJECT", 0U);
        end();
    }
    reported = 1U;
    check("CHECK_NS_STARTUP", (observations & 1U) != 0U);
    check("CHECK_SERVICE", (observations & 2U) != 0U);
    check("CHECK_POLICY_DENY", (observations & 4U) != 0U);
    check("CHECK_CALLER_NS", cmse_nonsecure_caller() && calls == 5U && from_ns == 5U);
    /* Return through the compiler-generated BXNS before testing denial. */
}
void SecureFault_Handler(void)
{
    uint32_t status = SFSR;
    console_kv_hex("SFSR", status);
    uint32_t address_valid = (status & (1U << 6)) != 0U;
    if (address_valid) console_kv_hex("SFAR", SFAR);
    else console_kv_str("SFAR", "INVALID");
#ifdef L16_INVALID_ENTRY
    check("CHECK_INVALID_ENTRY", (status & 1U) != 0U && calls == 0U);
#else
    check("CHECK_SECURE_REJECT", (status & ~0x40U) == 0x08U &&
          (!address_valid || SFAR == 0x30000000U) && reported == 1U);
#endif
    check("CHECK_SECRET", secret == 0x13579BDFU);
    check("CHECK_SECURE_HANDLER", __get_IPSR() == 7U);
    check("CHECK_NO_ESCALATION", SCB->HFSR == 0U && SCB->CFSR == 0U);
    end();
}

/* Configure only complete blocks; discover MPC granularity, never assume the
 * QEMU page-size choice equals the FPGA's block size. Offsets are within RAM. */
static uint32_t mpc_ns(uintptr_t base, uint32_t first, uint32_t limit)
{
    uint32_t cfg = REG(base + 0x14U);
    if (cfg > 26U) return 0U;
    uint32_t block = 1UL << (cfg + 5U);
    if ((first & (block - 1U)) || (limit & (block - 1U))) return 0U;
    REG(base) = 1U << 4; /* error response; no auto-increment */
    uint32_t max = REG(base + 0x10U);
    for (uint32_t i = first / block; i < limit / block; ++i) {
        if (i / 32U > max) return 0U;
        REG(base + 0x18U) = i / 32U;
        REG(base + 0x1CU) |= 1UL << (i % 32U);
    }
    return 1U;
}
static void region(uint32_t number, uint32_t first, uint32_t last, uint32_t nsc)
{
    SAU->RNR = number;
    SAU->RBAR = first & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = (last & SAU_RLAR_LADDR_Msk) | (nsc ? SAU_RLAR_NSC_Msk : 0U) |
                SAU_RLAR_ENABLE_Msk;
}
int main(void)
{
    console_line("===== LESSON 16: SECURE AND NONSECURE IMAGES =====");
    __TZ_set_MSPLIM_NS(0x20007000U);
    __set_MSPLIM(0x30003000U);
    SAU->CTRL = 0U;
    for (uint32_t i = 0U; i < (SAU->TYPE & 0xFFU); ++i) {
        SAU->RNR = i;
        SAU->RLAR = 0U;
    }
    region(0U, 0x00080000U, 0x0009FFFFU, 0U);
    region(1U, 0x20004000U, 0x20007FFFU, 0U);
    region(2U, (uint32_t)(uintptr_t)&__nsc_start,
           (uint32_t)(uintptr_t)&__nsc_end - 1U, 1U);
    REG(0x50080014U) = 1U; /* IoT Kit IDAU permits Code NSC, not all code NS. */
    uint32_t configured = mpc_ns(0x58007000U, 0x80000U, 0xA0000U) &&
                          mpc_ns(0x50083000U, 0x4000U, 0x8000U);
    SAU->CTRL = SAU_CTRL_ENABLE_Msk;
    SCB->SHCSR |= SCB_SHCSR_SECUREFAULTENA_Msk;
    __DSB(); __ISB();
    check("CHECK_PARTITION", configured && (SAU->TYPE & 0xFFU) >= 3U);
    if (!configured) end();
    uint32_t *vectors = (uint32_t *)0x00080000U;
    uint32_t stack = vectors[0], reset = vectors[1];
    uint32_t valid = stack == 0x20008000U && (reset & 1U) &&
                     reset >= 0x00080001U && reset < 0x000A0000U;
    check("CHECK_NS_VECTORS", valid);
    if (!valid) end();
    SCB_NS->VTOR = 0x00080000U;
    __TZ_set_CONTROL_NS(0U);
    __TZ_set_MSP_NS(stack);
    __DSB(); __ISB();
    ns_entry_t entry = (ns_entry_t)cmse_nsfptr_create((void *)(uintptr_t)reset);
    entry();
    check("CHECK_NS_RETURNED", 0U);
    end();
}
