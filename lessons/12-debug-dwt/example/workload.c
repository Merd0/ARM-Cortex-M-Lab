#include <stdint.h>

/* Runtime input; noipa prevents call-site constant propagation and inlining.
 * A normal C function: the compiler owns AAPCS register preservation. */
__attribute__((noinline, noipa))
uint32_t l12_workload(const uint32_t *input, uint32_t count)
{
    uint32_t sum = 0U;
    for (uint32_t i = 0U; i < count; ++i) {
        sum += 3U * input[i] + 1U;
    }
    return sum;
}
