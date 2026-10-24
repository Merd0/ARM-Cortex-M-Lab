#include "../diagnostic.h"
#include "platform/mps2_an505.h"
#include "common/console.h"
static void check(const char *key, int good) { console_kv_str(key, good ? "PASS" : "FAIL"); }
int main(void)
{
    exception_frame_t frame = {0};
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
                              SCB_CFSR_STKOF_Msk, SCB_CFSR_UNSTKERR_Msk};
    int rejected = 1;
    for (uint32_t i = 0U; i < 4U; ++i) {
        s.cfsr = faults[i];
        if (fa_capture(&s, 0xFFFFFFF8U, 0U, 0xFFFFFFFFU)) rejected = 0;
        fa_analyze(&s, &d);
        if (d.frame_safe) rejected = 0;
    }
    check("CHECK_STACK_ERROR_REJECT", rejected);
    s.cfsr = 0U; s.exc_return = 0xFFFFFFEDU; /* FP unsupported */
    check("CHECK_FP_REJECT", !fa_capture(&s, 0xFFFFFFF8U, 0U, 0xFFFFFFFFU));
    s.exc_return = 0xFFFFFFBDU; /* cross-security unsupported */
    check("CHECK_CROSS_SECURITY_REJECT", !fa_capture(&s, 0xFFFFFFF8U, 0U, 0xFFFFFFFFU));
    s.exc_return = 0xFFFFFFFDU;
    check("CHECK_BOUNDS_REJECT", !fa_capture(&s, (uintptr_t)&frame, (uintptr_t)&frame + 4U,
                                           (uintptr_t)(&frame + 1)));
    s.cfsr = SCB_CFSR_PRECISERR_Msk; s.bfar_raw = 0xDEADBEEFU;
    fa_analyze(&s, &d); check("CHECK_INVALID_ADDRESS", !d.address_valid);
    s.cfsr = SCB_CFSR_DACCVIOL_Msk | SCB_CFSR_MMARVALID_Msk; s.mmfar_raw = 0x30000100U;
    fa_analyze(&s, &d); check("CHECK_MMFAR", d.address_valid && d.fault_address == s.mmfar_raw);
    console_line("===== END =====");
    for (;;) __WFI();
}
