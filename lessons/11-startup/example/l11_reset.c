/*
 * Ders 11 - DERS-YEREL Reset_Handler (ayarlanabilir, egitim amacli).
 *
 * Paylasilan startup_mps2_an505.c'deki sirayi izler:
 *   .data kopyala -> .bss sifirla -> SystemInit -> main -> WFI dongusu
 * Tek ek: en basta l11_poison_ram() (EGITIM AMACLI TEST ARACI).
 *
 * Derleme anahtarlari (yalnizca bu ders icin):
 *   L11_BROKEN_SKIP_BSS  .bss sifirlamasini ATLA   (11-startup-broken)
 *   L11_SKIP_DATA        .data kopyasini ATLA      (yalnizca build probe'lari)
 *   L11_NO_POISON        zehirlemeyi ATLA          (QEMU maskeleme deneyi)
 *
 * Faz kaydi: yerel bir degiskende biriktirilir ve g_l11_phase'e (.bss)
 * ancak TUM RAM baslatma adimlarindan SONRA yazilir. Boylece ne
 * zehirleme ne .bss sifirlamasi onu silebilir.
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

#ifndef L11_NO_POISON
    l11_poison_ram();                    /* EGITIM AMACLI TEST ARACI */
#endif

    /* 1) .data: FLASH'taki ilk degerleri (LMA) RAM'e (VMA) kopyala */
#ifndef L11_SKIP_DATA
    {
        const uint32_t *src = &__etext;
        uint32_t *dst = &__data_start;
        while (dst < &__data_end) {
            *dst++ = *src++;
        }
    }
#endif
    phase = (phase << 4) | 1u;

    /* 2) .bss: sifirla */
#if !defined(L11_BROKEN_SKIP_BSS)
    {
        uint32_t *dst = &__bss_start;
        while (dst < &__bss_end) {
            *dst++ = 0u;
        }
    }
#endif
    phase = (phase << 4) | 2u;

    /* 3) Sistem baslatma (paylasilan system_init.c'deki guclu SystemInit) */
    SystemInit();
    phase = (phase << 4) | 3u;

    /* Faz kaydi RAM baslatmadan SONRA yaziliyor */
    g_l11_phase = phase;

    /* 4) Uygulama */
    (void)main();

    for (;;) {
        __WFI();
    }
}
