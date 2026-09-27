/*
 * EXERCISE 01 - Complete the Reset Sequence
 *
 * Bu Reset_Handler'in iskeleti hazir: zehirleme (egitim araci), faz
 * kaydi, SystemInit ve main cagrilari. EKSIK olan iki adim:
 *
 *   TODO 1: .data'nin ilk degerlerini FLASH'tan (LMA) RAM'e (VMA) kopyala
 *   TODO 2: .bss'i sifirla
 *
 * Kullanabilecegin linker sembolleri (fixtures.h):
 *   __etext  __data_start  __data_end  __bss_start  __bss_end
 * Adres sabiti YAZMA; yalnizca bu sembolleri kullan.
 *
 * Baslangic durumunda firmware calisir ve sunu raporlar:
 *   CHECK_DATA_INIT FAIL, CHECK_BSS_ZERO FAIL, CHECK_PHASE_ORDER PASS
 *
 * Faz kayitlarinin YERINI ve SIRASINI degistirme: CHECK_PHASE_ORDER,
 * .data -> .bss -> SystemInit sirasini denetler.
 *
 * Referans cozum: solutions/01-reset-sequence/
 */
#include <stdint.h>
#include "platform/mps2_an505.h"
#include "fixtures.h"

extern int main(void);
extern void SystemInit(void);
void l11_poison_ram(void);

void Reset_Handler(void)
{
    uint32_t phase = 0u;

    l11_poison_ram();                    /* EGITIM AMACLI TEST ARACI - silme */

    /* 1) .data */
    /* TODO(student): .data kopyalama dongusu */
    phase = (phase << 4) | 1u;

    /* 2) .bss */
    /* TODO(student): .bss sifirlama dongusu */
    phase = (phase << 4) | 2u;

    /* 3) Sistem baslatma */
    SystemInit();
    phase = (phase << 4) | 3u;

    g_l11_phase = phase;

    /* 4) Uygulama */
    (void)main();

    for (;;) {
        __WFI();
    }
}
