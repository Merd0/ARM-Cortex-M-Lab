#include "../diagnostic.h"
#include "platform/mps2_an505.h"
#include "common/console.h"
static void check(const char *key, int good) { console_kv_str(key, good ? "PASS" : "FAIL"); }
int main(void)
{
    _Alignas(8) exception_frame_t frame = {0};
    frame.pc = 0x10000100U; frame.xpsr = 0x01000000U;
    fault_snapshot_t s = {0}; fault_decision_t d;
    s.ipsr = 3U; s.exc_return = 0xFFFFFFFDU;
    s.hfsr = SCB_HFSR_FORCED_Msk; s.cfsr = 0x8200U; s.bfar_raw = 0x30100000U;
    fa_capture(&s, (uintptr_t)&frame, (uintptr_t)&frame, (uintptr_t)(&frame + 1));
    fa_analyze(&s, &d);
    check("CHECK_VALID_PSP", d.frame_safe && d.stacked_pc == frame.pc && d.escalated &&
          d.address_valid && d.fault_address == 0x30100000U);
    s.exc_return = 0xFFFFFFF9U;
    check("CHECK_VALID_MSP", fa_capture(&s, (uintptr_t)&frame, (uintptr_t)&frame, (uintptr_t)(&frame + 1)));
    s.exc_return = 0xFFFFFFBCU;
    check("CHECK_VALID_NS", fa_capture(&s, (uintptr_t)&frame, (uintptr_t)&frame, (uintptr_t)(&frame + 1)));
    const uint32_t faults[] = {SCB_CFSR_STKERR_Msk, SCB_CFSR_MSTKERR_Msk,
                              SCB_CFSR_STKOF_Msk, SCB_CFSR_UNSTKERR_Msk,
                              SCB_CFSR_MUNSTKERR_Msk, SCB_CFSR_MLSPERR_Msk,
                              SCB_CFSR_LSPERR_Msk};
    int rejected = 1;
    for (uint32_t i = 0U; i < sizeof(faults) / sizeof(faults[0]); ++i) {
        s.cfsr = faults[i];
        /* Bounds alone would reject the invalid pointer below. Also supply a
         * structurally valid frame to prove that each status bit vetoes it. */
        if (fa_capture(&s, (uintptr_t)&frame, (uintptr_t)&frame,
                       (uintptr_t)(&frame + 1))) rejected = 0;
        if (fa_capture(&s, 0xFFFFFFF8U, 0U, 0xFFFFFFFFU)) rejected = 0;
        s.frame_valid = 1U; /* Analyze must not trust a caller's validity flag. */
        fa_analyze(&s, &d);
        if (d.frame_safe) rejected = 0;
    }
    check("CHECK_STACK_ERROR_REJECT", rejected);
    s.cfsr = 0U; s.exc_return = 0xFFFFFFEDU; /* FP unsupported */
    check("CHECK_FP_REJECT", !fa_capture(&s, 0xFFFFFFF8U, 0U, 0xFFFFFFFFU));
    s.exc_return = 0xFFFFFFBDU; /* cross-security unsupported */
    check("CHECK_CROSS_SECURITY_REJECT", !fa_capture(&s, 0xFFFFFFF8U, 0U, 0xFFFFFFFFU));
    s.exc_return = 0xFFFFFFFDU;
    /* A word-aligned payload is not necessarily an M33 exception frame. */
    _Alignas(8) uint32_t offset_frame[9] = {0};
    offset_frame[7] = frame.pc;
    offset_frame[8] = frame.xpsr;
    check("CHECK_ALIGNMENT_REJECT", !fa_capture(&s, (uintptr_t)&offset_frame[1],
          (uintptr_t)offset_frame, (uintptr_t)(offset_frame + 9)));
    _Alignas(8) struct { exception_frame_t frame; uint32_t pad; } padded = {0};
    padded.frame.pc = frame.pc; padded.frame.xpsr = frame.xpsr | (1U << 9);
    uintptr_t start = (uintptr_t)&padded;
    check("CHECK_PADDING", !fa_capture(&s, start, start, start + 32U) &&
          fa_capture(&s, start, start, start + 36U));
    frame.xpsr |= 15U;
    check("CHECK_MODE_REJECT", !fa_capture(&s, (uintptr_t)&frame,
          (uintptr_t)&frame, (uintptr_t)(&frame + 1)));
    s.exc_return = 0xFFFFFFF1U;
    check("CHECK_VALID_HANDLER", fa_capture(&s, (uintptr_t)&frame,
          (uintptr_t)&frame, (uintptr_t)(&frame + 1)));
    frame.xpsr = 0x01000000U;
    check("CHECK_HANDLER_MODE_REJECT", !fa_capture(&s, (uintptr_t)&frame,
          (uintptr_t)&frame, (uintptr_t)(&frame + 1)));
    s.exc_return = 0xFFFFFFFDU;
    frame.pc |= 1U;
    check("CHECK_PC_REJECT", !fa_capture(&s, (uintptr_t)&frame,
          (uintptr_t)&frame, (uintptr_t)(&frame + 1)));
    frame.pc &= ~1U; frame.xpsr = 0U;
    check("CHECK_THUMB_REJECT", !fa_capture(&s, (uintptr_t)&frame,
          (uintptr_t)&frame, (uintptr_t)(&frame + 1)));
    frame.xpsr = 0x01000000U;
    check("CHECK_BOUNDS_REJECT", !fa_capture(&s, (uintptr_t)&frame, (uintptr_t)&frame + 4U,
                                           (uintptr_t)(&frame + 1)));
    s.cfsr = SCB_CFSR_PRECISERR_Msk; s.bfar_raw = 0xDEADBEEFU;
    fa_analyze(&s, &d); check("CHECK_INVALID_ADDRESS", !d.address_valid);
    s.cfsr = SCB_CFSR_DACCVIOL_Msk | SCB_CFSR_MMARVALID_Msk; s.mmfar_raw = 0x30000100U;
    fa_analyze(&s, &d); check("CHECK_MMFAR", d.address_valid && d.fault_address == s.mmfar_raw);
    console_line("===== END =====");
    for (;;) __WFI();
}
