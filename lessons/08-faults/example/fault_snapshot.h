/*
 * Ders 08 - fault anlik goruntusu (snapshot) ve karar yapisi.
 * Egitim amacli kucuk bir yapi; uretim fault cercevesi DEGILDIR.
 */
#ifndef LESSON08_FAULT_SNAPSHOT_H
#define LESSON08_FAULT_SNAPSHOT_H

#include <stdint.h>

typedef struct {
    uint32_t r0, r1, r2, r3, r12, lr, pc, xpsr;
} exception_frame_t;

typedef struct {
    /* RAW: donanimdan okunan ham degerler */
    uint32_t ipsr;
    uint32_t exc_return;
    uint32_t control;
    uint32_t msp;
    uint32_t psp;
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t shcsr;
    uint32_t mmfar_raw;
    uint32_t bfar_raw;
    uint32_t frame_address;
    /* frame yalnizca frame_valid != 0 iken anlamli */
    int      frame_valid;
    exception_frame_t frame;
} fault_snapshot_t;

/* Exercise 01: ham snapshot'tan cikarilan KARARLAR */
typedef struct {
    const char *active_exception;   /* "MemManage" / "BusFault" / "UsageFault" / "HardFault" */
    int         escalated;          /* 1 / 0 */
    const char *fault_class;        /* CFSR'deki alt alan: "MemManage" / "BusFault" / "UsageFault" / "none" */
    const char *primary_bit;        /* ornek: "DACCVIOL" */
    int         address_valid;      /* MMFAR veya BFAR gecerli mi */
    uint32_t    fault_address;      /* yalnizca address_valid ise anlamli */
    int         frame_safe;         /* frame cozulebilir mi */
    uint32_t    stacked_pc;         /* yalnizca frame_safe ise anlamli */
} fault_decision_t;

void decode_fault_snapshot(const fault_snapshot_t *s, fault_decision_t *d);

#endif
