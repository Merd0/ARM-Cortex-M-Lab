/*
 * SOLUTION 01 - Complete the Fault Report
 * Exercise: exercises/01-fault-report/decode.c
 * Aciklama: solutions/01-fault-report/README.md
 */
#include "platform/mps2_an505.h"
#include "fault_snapshot.h"

static const char *name_of(uint32_t ipsr)
{
    switch (ipsr) {
    case 3u: return "HardFault";
    case 4u: return "MemManage";
    case 5u: return "BusFault";
    case 6u: return "UsageFault";
    default: return "other";
    }
}

void decode_fault_snapshot(const fault_snapshot_t *s, fault_decision_t *d)
{
    /* 1) Hangi exception calisiyor? (IPSR) */
    d->active_exception = name_of(s->ipsr & 0x1FFu);

    /* 2) Yukseltilmis mi? (HFSR.FORCED) - HardFault'ta KOK SEBEP degil */
    d->escalated = ((s->hfsr & SCB_HFSR_FORCED_Msk) != 0u);

    /* 3) Kanit CFSR'de: hangi alt alan dolu?  Aktif handler'dan bagimsiz. */
    d->fault_class = "none";
    d->primary_bit = "none";
    d->address_valid = 0;
    d->fault_address = 0u;

    if ((s->cfsr & SCB_CFSR_MEMFAULTSR_Msk) != 0u) {
        d->fault_class = "MemManage";
        if (s->cfsr & SCB_CFSR_DACCVIOL_Msk)      d->primary_bit = "DACCVIOL";
        else if (s->cfsr & SCB_CFSR_IACCVIOL_Msk) d->primary_bit = "IACCVIOL";
        else if (s->cfsr & SCB_CFSR_MSTKERR_Msk)  d->primary_bit = "MSTKERR";
        /* 4) Adres yalnizca VALID biti ile birlikte kanittir */
        if (s->cfsr & SCB_CFSR_MMARVALID_Msk) {
            d->address_valid = 1;
            d->fault_address = s->mmfar_raw;
        }
    } else if ((s->cfsr & SCB_CFSR_BUSFAULTSR_Msk) != 0u) {
        d->fault_class = "BusFault";
        if (s->cfsr & SCB_CFSR_PRECISERR_Msk)        d->primary_bit = "PRECISERR";
        else if (s->cfsr & SCB_CFSR_IMPRECISERR_Msk) d->primary_bit = "IMPRECISERR";
        else if (s->cfsr & SCB_CFSR_STKERR_Msk)      d->primary_bit = "STKERR";
        else if (s->cfsr & SCB_CFSR_IBUSERR_Msk)     d->primary_bit = "IBUSERR";
        if (s->cfsr & SCB_CFSR_BFARVALID_Msk) {
            d->address_valid = 1;
            d->fault_address = s->bfar_raw;
        }
    } else if ((s->cfsr & SCB_CFSR_USGFAULTSR_Msk) != 0u) {
        d->fault_class = "UsageFault";
        if (s->cfsr & SCB_CFSR_UNDEFINSTR_Msk)      d->primary_bit = "UNDEFINSTR";
        else if (s->cfsr & SCB_CFSR_STKOF_Msk)      d->primary_bit = "STKOF";
        else if (s->cfsr & SCB_CFSR_INVSTATE_Msk)   d->primary_bit = "INVSTATE";
        /* UsageFault'un adres register'i YOKTUR */
    }

    /* 5) Frame yalnizca yigma basarili ve bicim destekleniyorsa */
    d->frame_safe = s->frame_valid;
    d->stacked_pc = d->frame_safe ? s->frame.pc : 0u;
}
