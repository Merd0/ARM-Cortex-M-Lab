#include <stdint.h>

volatile uint32_t g_l12_watch;

/* Keep call boundaries visible in all presets. The post-call barrier prevents
 * sibling-call elimination; it is a compiler barrier, not a hardware DMB. */
__attribute__((noinline, noipa)) uint32_t l12_leaf(uint32_t value)
{
#if defined(L12_BROKEN_STORE)
    g_l12_watch = value + 1U;  /* BREAK IT: legal RAM write, wrong value. */
#else
    g_l12_watch = value;
#endif
    return g_l12_watch;
}

__attribute__((noinline, noipa)) uint32_t l12_middle(uint32_t value)
{
    uint32_t result = l12_leaf(value);
    __asm volatile ("" ::: "memory");
    return result;
}

__attribute__((noinline, noipa)) uint32_t l12_outer(uint32_t value)
{
    uint32_t result = l12_middle(value);
    __asm volatile ("" ::: "memory");
    return result;
}
