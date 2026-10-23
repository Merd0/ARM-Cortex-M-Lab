#include "../../measurement.h"

enum l12_kind l12_classify(const struct l12_sample *sample)
{
    /* TODO(student): implement the contract in measurement.h. */
    (void)sample;
    return L12_UNANSWERED;
}

uint32_t l12_delta(uint32_t start, uint32_t end)
{
    /* TODO(student): unsigned counter difference, including one wrap. */
    (void)start;
    (void)end;
    return 0U;
}
