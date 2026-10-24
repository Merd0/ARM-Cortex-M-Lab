#include "../api.h"
static volatile uint32_t initialized = 0x2468ACE0U;
static volatile uint32_t zeroed;
int main(void)
{
#ifdef L16_INVALID_ENTRY
    /* Imported entry points at SG; +4 skips that 32-bit instruction. */
    uint32_t (*bad)(uint32_t) =
        (uint32_t (*)(uint32_t))((uintptr_t)&l16_service + 4U);
    (void)bad(7U);
#else
    uint32_t observations = initialized == 0x2468ACE0U && zeroed == 0U ? 1U : 0U;
    uint32_t result0 = l16_service(0U);
    uint32_t result7 = l16_service(7U);
    uint32_t result15 = l16_service(15U);
    uint32_t denied16 = l16_service(16U);
    uint32_t denied_max = l16_service(0xFFFFFFFFU);
    if (result0 == 1U && result7 == 15U && result15 == 31U) observations |= 2U;
    if (denied16 == L16_DENIED && denied_max == L16_DENIED) observations |= 4U;
    l16_report(observations);
    /* Must fault in NS state: SAU/IDAU classify this as Secure data. */
    zeroed = *(volatile const uint32_t *)0x30000000U;
#endif
    l16_report(0x80000000U); /* Reaching here is a protection failure. */
    for (;;) __asm volatile("wfi");
}
