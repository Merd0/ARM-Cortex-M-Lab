/*
 * EXERCISE 01 - Locate Your Data
 *
 * Dort fonksiyonu YALNIZCA linker sembollerinden ve nesne adreslerinden
 * hesapla. Map dosyasindan sayi KOPYALAMA: kodun baska bir build'de de
 * dogru kalmali.
 *
 * Kullanabilecegin semboller (l10.h, linker script tarafindan tanimli):
 *   __vectors_start  __vectors_end  __etext  __data_start  __data_end
 *   __bss_start  __bss_end  __heap_start  __heap_end  __stack_top
 * Hatirlatma: linker sembollerinin DEPOLAMASI yoktur; &__data_start gibi
 * ADRESLERI kullanilir.
 *
 * Sorular (README'de de var):
 *   - FLASH'ta hangi byte'lar var? Yalnizca .text mi?
 *   - .data FLASH'ta da yer kaplar mi? Neden?
 *   - __etext adi neyi soyluyor, gercekte neyi tutuyor? (linker script'e bak)
 *   - .bss FLASH'ta yer kaplar mi?
 *
 * Su an her fonksiyon 0 donduruyor; kontroller FAIL verir. BEKLENEN
 * baslangic durumu.
 *
 * Referans cozum: solutions/01-locate-your-data/
 */
#include "l10.h"
#include "locate.h"

uint32_t flash_used_bytes(void)
{
    /* TODO(student) */
    return 0u;
}

uint32_t ram_static_bytes(void)
{
    /* TODO(student) */
    return 0u;
}

uint32_t data_lma_of(const void *p)
{
    (void)p;
    /* TODO(student) */
    return 0u;
}

int in_region(const void *p, uint32_t origin, uint32_t length)
{
    (void)p;
    (void)origin;
    (void)length;
    /* TODO(student) */
    return 0;
}
