/*
 * MPS2-AN505 startup code (calisan platform startup'i).
 *
 * DIKKAT: Bu dosya "hazir calisan" altyapidir.
 * Startup'i sifirdan yazma alistirmasi ayri bir derste yapilir:
 *   lessons/11-startup/
 *
 * Reset akisi:
 *   RESET -> initial MSP (vector[0]) -> Reset_Handler (vector[1])
 *         -> .data kopyala -> .bss sifirla -> SystemInit -> main
 */
#include <stdint.h>
#include "platform/mps2_an505.h"
#include "platform/board.h"

/* Linker script'ten gelen semboller (degerleri degil, ADRESLERI onemli) */
extern uint32_t __etext;
extern uint32_t __data_start;
extern uint32_t __data_end;
extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern uint32_t __stack_top;

extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

/* Zayif (weak) tanimlar: uygulama isterse ayni isimle kendi handler'ini yazar
   ve linker otomatik olarak onu secer. */
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

__attribute__((weak)) void SystemInit(void) { }

typedef void (*vector_entry_t)(void);

/* Vector table.
   [0] = initial MSP degeri (fonksiyon degil, adres!)
   [1] = Reset_Handler
   Armv8-M ARM (DDI 0553), "The vector table" bolumu. */
__attribute__((section(".vectors"), used))
const vector_entry_t __vector_table[] = {
    (vector_entry_t)(&__stack_top),  /*  0: Initial MSP              */
    Reset_Handler,                   /*  1: Reset                    */
    NMI_Handler,                     /*  2: NMI                      */
    HardFault_Handler,               /*  3: HardFault                */
    MemManage_Handler,               /*  4: MemManage                */
    BusFault_Handler,                /*  5: BusFault                 */
    UsageFault_Handler,              /*  6: UsageFault               */
    SecureFault_Handler,             /*  7: SecureFault (Armv8-M)    */
    0, 0, 0,                         /*  8-10: Reserved              */
    SVC_Handler,                     /* 11: SVCall                   */
    DebugMon_Handler,                /* 12: DebugMonitor             */
    0,                               /* 13: Reserved                 */
    PendSV_Handler,                  /* 14: PendSV                   */
    SysTick_Handler,                 /* 15: SysTick                  */
    /* 16+ : cihaza ozel kesmeler (IRQ0...) ileride eklenir */
};

void Reset_Handler(void)
{
    /* 1) .data: FLASH'taki kopyayi RAM'e tasi */
    uint32_t *src = &__etext;
    uint32_t *dst = &__data_start;
    while (dst < &__data_end) {
        *dst++ = *src++;
    }

    /* 2) .bss: sifirla */
    dst = &__bss_start;
    while (dst < &__bss_end) {
        *dst++ = 0U;
    }

    /* 3) Cekirdek/sistem baslatma (saat, FPU, vb.) */
    SystemInit();

    /* 4) Uygulama */
    (void)main();

    /* main donerse: burada kal (gercek urunlerde reset veya hata) */
    for (;;) {
        __WFI();
    }
}

void Default_Handler(void)
{
    /* Beklenmeyen exception. IPSR hangi exception oldugunu soyler.
       GDB'de:  p/x $xpsr   ya da  info registers xpsr */
    for (;;) {
        __asm volatile ("nop");
    }
}
