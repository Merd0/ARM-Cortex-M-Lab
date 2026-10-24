#include "../../measurement.h"
#include "platform/mps2_an505.h"

enum l12_kind l12_classify(const struct l12_sample *sample)
{
    if (sample->io_calls != 0U) return L12_CONTAMINATED;
    if ((sample->ctrl & (DWT_CTRL_NOCYCCNT_Msk | DWT_CTRL_CYCDISS_Msk)) != 0U ||
        (sample->ctrl & DWT_CTRL_CYCCNTENA_Msk) == 0U ||
        (sample->demcr & DCB_DEMCR_TRCENA_Msk) == 0U ||
        sample->start == sample->end)
        return L12_UNAVAILABLE;
    return L12_COUNTER_DELTA;
}

uint32_t l12_delta(uint32_t start, uint32_t end)
{
    return end - start;
}
