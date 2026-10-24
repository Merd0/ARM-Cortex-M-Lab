#include "diagnostic.h"
#include "platform/mps2_an505.h"

static int supported(uint32_t value)
{
    /* ES/S agree; DCRS=1, basic frame. Reject cross-security and FP frames. */
    return value == 0xFFFFFFF1U || value == 0xFFFFFFF9U || value == 0xFFFFFFFDU ||
           value == 0xFFFFFFB0U || value == 0xFFFFFFB8U || value == 0xFFFFFFBCU;
}
static int status_safe(const fault_snapshot_t *s)
{
    uint32_t stack_errors = SCB_CFSR_MSTKERR_Msk | SCB_CFSR_MUNSTKERR_Msk |
        SCB_CFSR_MLSPERR_Msk | SCB_CFSR_STKERR_Msk | SCB_CFSR_UNSTKERR_Msk |
        SCB_CFSR_LSPERR_Msk | SCB_CFSR_STKOF_Msk;
    return supported(s->exc_return) && !(s->cfsr & stack_errors);
}
int fa_capture(fault_snapshot_t *s, uintptr_t address, uintptr_t low, uintptr_t high)
{
    s->frame_valid = 0;
    s->frame_address = (uint32_t)address;
    if (!status_safe(s) || high < low || high - low < sizeof(exception_frame_t) ||
        (address & 3U) || address < low || address > high - sizeof(exception_frame_t)) return 0;
    const exception_frame_t *frame = (const exception_frame_t *)address;
    uint32_t xpsr = frame->xpsr;
    uint32_t pad = xpsr & (1U << 9) ? 4U : 0U;
    if (!(xpsr & (1U << 24)) || (frame->pc & 1U) ||
        ((s->exc_return & 8U) ? (xpsr & 0x1FFU) != 0U : (xpsr & 0x1FFU) == 0U) ||
        high - address < sizeof(*frame) + pad) return 0;
    s->frame = *frame;
    s->frame_valid = 1;
    return 1;
}
void fa_analyze(const fault_snapshot_t *s, fault_decision_t *d)
{
    fault_snapshot_t safe = *s;
    if (!status_safe(&safe)) safe.frame_valid = 0;
    decode_fault_snapshot(&safe, d);
    if ((s->cfsr & SCB_CFSR_UNALIGNED_Msk) && !(s->cfsr & 0xFFFFU))
        d->primary_bit = "UNALIGNED";
}
