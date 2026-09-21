/*
 * EXERCISE 01 - Complete the Fault Report
 *
 * decode_fault_snapshot() ham bir fault snapshot'i alir ve KARARLAR uretir.
 * Bu hedefte olusan fault: ayricaliksiz Thread'in korunan veriye dogrudan
 * yazmasi (ders 06'daki MemManage DACCVIOL).
 *
 * GOREV - asagidaki alanlari doldur:
 *
 *   d->active_exception  s->ipsr'den: 3 HardFault, 4 MemManage,
 *                        5 BusFault, 6 UsageFault
 *   d->escalated         HFSR.FORCED set mi?
 *   d->fault_class       CFSR'nin HANGI alt alani dolu? (MMFSR/BFSR/UFSR)
 *                        -> "MemManage" / "BusFault" / "UsageFault" / "none"
 *   d->primary_bit       o alt alandaki asil sebep biti (ornek "DACCVIOL")
 *                        VALID bitleri sebep degildir.
 *   d->address_valid     MMFAR / BFAR ancak ilgili VALID biti 1 ise kanittir
 *   d->fault_address     yalnizca address_valid ise doldur
 *   d->frame_safe        s->frame_valid (yigma basarili + desteklenen bicim)
 *   d->stacked_pc        yalnizca frame_safe ise doldur
 *
 * Kullanabilecegin maskeler (CMSIS, core_cm33.h):
 *   SCB_HFSR_FORCED_Msk
 *   SCB_CFSR_MEMFAULTSR_Msk  SCB_CFSR_BUSFAULTSR_Msk  SCB_CFSR_USGFAULTSR_Msk
 *   SCB_CFSR_DACCVIOL_Msk  SCB_CFSR_IACCVIOL_Msk  SCB_CFSR_MMARVALID_Msk
 *   SCB_CFSR_PRECISERR_Msk SCB_CFSR_BFARVALID_Msk SCB_CFSR_UNDEFINSTR_Msk
 *
 * KAPSAM: her CFSR bitini cozmen istenmiyor. Bu dersin uc fault turu yeter.
 *
 * Su an her alan bos; CHECK_D_* satirlari FAIL verir. BEKLENEN baslangic.
 * Referans cozum: solutions/01-fault-report/decode.c
 */
#include "platform/mps2_an505.h"
#include "fault_snapshot.h"

void decode_fault_snapshot(const fault_snapshot_t *s, fault_decision_t *d)
{
    (void)s;

    /* TODO(student) */
    d->active_exception = "?";
    d->escalated        = 1;
    d->fault_class      = "?";
    d->primary_bit      = "?";
    d->address_valid    = 0;
    d->fault_address    = 0u;
    d->frame_safe       = 0;
    d->stacked_pc       = 0u;
}
