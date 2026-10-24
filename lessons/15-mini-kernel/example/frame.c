#include "../kernel.h"

uint32_t *l15_initial_frame(uint32_t *top, uint32_t id, uintptr_t entry, uintptr_t exit)
{
    /* Caller supplies a checked, 8-byte-aligned stack with room for 17 words. */
    uint32_t *sp = top - L15_CONTEXT_WORDS;
    for (uint32_t i = 0; i < L15_CONTEXT_WORDS; ++i) sp[i] = 0U;
    sp[8] = L15_EXC_RETURN; /* software R4-R11, EXC_RETURN */
    sp[9] = id;             /* hardware R0-R3, R12, LR, PC, xPSR */
    sp[14] = (uint32_t)exit | 1U;
    sp[15] = (uint32_t)entry & ~1U;
    sp[16] = 0x01000000U;
    return sp;
}
