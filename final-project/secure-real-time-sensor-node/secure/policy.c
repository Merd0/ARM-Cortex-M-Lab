#include "platform/mps2_an505.h"
#include "../../../lessons/16-trustzone-advanced/api.h"

uint32_t l16_allowed(uint32_t value)
{
    /* FP belongs to the bounded privileged SVC service, never a Thread task.
     * Reject before any VFP instruction; no active Secure task context is moved. */
    return value <= 15U && __get_IPSR() == 11U;
}
