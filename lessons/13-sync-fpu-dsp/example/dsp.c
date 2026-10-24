#include "platform/mps2_an505.h"
#include "common/console.h"
#include <limits.h>

static uint32_t pack(int16_t lo, int16_t hi)
{
    return (uint32_t)(uint16_t)lo | ((uint32_t)(uint16_t)hi << 16);
}

__attribute__((noinline, noipa)) int32_t l13_dot_c(const int16_t *a, const int16_t *b)
{
    /* Only four bounded samples in this example: no signed overflow. */
    int32_t sum = 0;
    for (uint32_t i = 0U; i < 4U; ++i) sum += (int32_t)a[i] * b[i];
    return sum;
}

__attribute__((noinline, noipa)) int32_t l13_dot_dsp(const int16_t *a, const int16_t *b)
{
    uint32_t sum = __SMLAD(pack(a[0], a[1]), pack(b[0], b[1]), 0U);
    return (int32_t)__SMLAD(pack(a[2], a[3]), pack(b[2], b[3]), sum);
}

static uint32_t saturate(int32_t a, int32_t b, int32_t expected)
{
    uint32_t saved = __get_APSR();
    uint32_t cleared = saved & ~(1U << 27);
    int32_t result;
    uint32_t flags;
    __asm volatile ("msr APSR_nzcvq, %2\n\tqadd %0, %3, %4\n\tmrs %1, APSR"
                    : "=&r"(result), "=&r"(flags)
                    : "r"(cleared), "r"(a), "r"(b) : "cc");
    __asm volatile ("msr APSR_nzcvq, %0" :: "r"(saved) : "cc");
    return result == expected && (flags & (1U << 27)) != 0U;
}

uint32_t l13_dsp_checks(void)
{
    static const int16_t a[4] = {1000, -2000, 3000, -4000};
    static const int16_t b[4] = {2, 3, -4, 5};
    int32_t plain = l13_dot_c(a, b);
    int32_t dsp = l13_dot_dsp(a, b);
    console_kv_hex("DOT_C", (uint32_t)plain);
    console_kv_hex("DOT_DSP", (uint32_t)dsp);
    uint32_t hi = saturate(INT32_MAX - 2, 10, INT32_MAX);
    uint32_t lo = saturate(INT32_MIN + 2, -10, INT32_MIN);
    console_kv_str("SATURATION_AND_Q", hi && lo ? "PASS" : "FAIL");
    return plain == -36000 && dsp == plain && hi && lo;
}
