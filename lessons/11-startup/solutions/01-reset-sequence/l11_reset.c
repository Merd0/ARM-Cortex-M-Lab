/*
 * SOLUTION 01 - Complete the Reset Sequence
 * Aciklama: solutions/01-reset-sequence/README.md
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

    l11_poison_ram();                    /* EGITIM AMACLI TEST ARACI */

    /* 1) .data: ilk degerler FLASH'ta (LMA = __etext), degiskenler RAM'de
     *    (VMA = __data_start .. __data_end). Linker script sinirlari 4'e
     *    hizaladigi icin word word kopyalamak yeterli ve tam. */
    {
        const uint32_t *src = &__etext;
        uint32_t *dst = &__data_start;
        while (dst < &__data_end) {
            *dst++ = *src++;
        }
    }
    phase = (phase << 4) | 1u;

    /* 2) .bss: [__bss_start, __bss_end) araligini sifirla */
    {
        uint32_t *dst = &__bss_start;
        while (dst < &__bss_end) {
            *dst++ = 0u;
        }
    }
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
