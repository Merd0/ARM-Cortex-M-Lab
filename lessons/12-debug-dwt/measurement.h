#ifndef L12_MEASUREMENT_H
#define L12_MEASUREMENT_H
#include <stdint.h>
enum l12_kind { L12_UNANSWERED, L12_UNAVAILABLE, L12_CONTAMINATED, L12_COUNTER_DELTA };
struct l12_sample {
    uint32_t ctrl, demcr, start, end, io_calls;
};
/* Secure-state sample. These are synthetic inputs, not a claim about QEMU.
 * COUNTER_DELTA requires NOCYCCNT=0, CYCDISS=0, CYCCNTENA=1, TRCENA=1,
 * and unequal endpoints. Any I/O makes it CONTAMINATED first.
 * A delta is modulo 2^32; the caller must independently bound the interval to
 * less than 2^32 counter increments. It is not a WCET proof. */
enum l12_kind l12_classify(const struct l12_sample *sample);
uint32_t l12_delta(uint32_t start, uint32_t end);
#endif
