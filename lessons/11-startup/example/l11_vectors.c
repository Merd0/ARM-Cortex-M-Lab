/*
 * Ders 11 - DERS-YEREL vektor tablosu ve varsayilan handler'lar.
 *
 * Yalnizca OZEL STARTUP hedefleri (11-startup-broken, 11-exercise-startup,
 * 11-solution-startup) kullanir. Bu hedefler paylasilan
 * startup_mps2_an505.c'yi LINK ETMEZ (bkz. CMakeLists.txt); yoksa iki
 * Reset_Handler / iki vektor tablosu olurdu.
 *
 * Yapi paylasilan startup ile ayni: 16 giris, vector[0] = ilk MSP,
 * vector[1] = Reset_Handler, handler'lar WEAK alias, 8-10 ve 13 sabit 0.
 *
 * Reset_Handler bu dosyada DEGIL: her hedef kendi l11_reset.c'sini getirir.
 */
#include <stdint.h>
#include "platform/mps2_an505.h"
#include "fixtures.h"

void Reset_Handler(void);
void Default_Handler(void);

#define WEAK_ALIAS __attribute__((weak, alias("Default_Handler")))

void NMI_Handler(void)            WEAK_ALIAS;
void HardFault_Handler(void)      WEAK_ALIAS;
void MemManage_Handler(void)      WEAK_ALIAS;
void BusFault_Handler(void)       WEAK_ALIAS;
void UsageFault_Handler(void)     WEAK_ALIAS;
void SecureFault_Handler(void)    WEAK_ALIAS;
void SVC_Handler(void)            WEAK_ALIAS;
void DebugMon_Handler(void)       WEAK_ALIAS;
void PendSV_Handler(void)         WEAK_ALIAS;
void SysTick_Handler(void)        WEAK_ALIAS;

typedef void (*vector_entry_t)(void);

__attribute__((section(".vectors"), used))
const vector_entry_t __vector_table[] = {
    (vector_entry_t)(&__stack_top),  /*  0: ilk MSP (bir adres DEGERI) */
    Reset_Handler,                   /*  1: Reset                      */
    NMI_Handler,                     /*  2 */
    HardFault_Handler,               /*  3 */
    MemManage_Handler,               /*  4 */
    BusFault_Handler,                /*  5 */
    UsageFault_Handler,              /*  6 */
    SecureFault_Handler,             /*  7 */
    0, 0, 0,                         /*  8-10: ayrilmis */
    SVC_Handler,                     /* 11 */
    DebugMon_Handler,                /* 12 */
    0,                               /* 13: ayrilmis */
    PendSV_Handler,                  /* 14 */
    SysTick_Handler,                 /* 15 */
};

void Default_Handler(void)
{
    for (;;) {
        __asm volatile ("nop");
    }
}

/* ------------------------------------------------------------------ */
/* EGITIM AMACLI TEST ARACI - uretim startup politikasi DEGILDIR.        */
/*                                                                      */
/* QEMU RAM'i sifirla baslatir (olculdu). Fiziksel donanimda reset      */
/* sonrasi RAM icerigi tanimsizdir. Bu fonksiyon .data ve .bss'in VMA   */
/* araligini taninabilir bir desenle (0xA5A5A5A5) doldurarak "tanimsiz  */
/* RAM"i deterministik olarak taklit eder. Boylece eksik bir .data      */
/* kopyasi veya eksik bir .bss sifirlamasi QEMU'da GORUNUR hale gelir.  */
/*                                                                      */
/* Paylasilan startup_mps2_an505.c bunu YAPMAZ ve yapmamalidir.          */
/* Yalnizca [__data_start, __bss_end) araligina yazar; stack RAM'in     */
/* tepesinde, bu araligin disindadir.                                   */
/* ------------------------------------------------------------------ */
void l11_poison_ram(void)
{
    for (uint32_t *p = &__data_start; p < &__bss_end; p++) {
        *p = L11_POISON;
    }
}
