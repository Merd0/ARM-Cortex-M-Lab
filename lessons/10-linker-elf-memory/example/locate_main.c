/*
 * Lesson 10 Exercise 01 - "Locate Your Data" surucusu.
 *
 * Bu dosyayi DEGISTIRME. Ogrencinin dosyasi: locate.c
 * (exercises/01-locate-your-data/ veya solutions/01-locate-your-data/).
 *
 * Kontroller sabit adreslere DAYANMAZ: beklenen degerler ya linker'in
 * kendi hesapladigi sembollerden (__flash_load_end) ya da bagimsiz bir
 * dogrulamadan (LMA'daki gercek ilk deger) gelir.
 */
#include "platform/mps2_an505.h"
#include "common/console.h"
#include "l10.h"
#include "locate.h"

static const char *pf(int ok) { return ok ? "PASS" : "FAIL"; }
static uint32_t A(const void *p) { return (uint32_t)(uintptr_t)p; }

/* ------------------------------------------------------------------ */
/* GUVENILMEYEN bir adresin, FLASH yuk goruntusunden 4 byte okunabilir  */
/* olup olmadigi. Ogrencinin dondurdugu HER deger icin guvenli olmali:  */
/* yanlis bir cevap bir fault'a degil, bir FAIL'e donusmeli.            */
/*                                                                      */
/* Aralik:  [ORIGIN(FLASH), __flash_load_end)   - bitis HARIC.          */
/*   __flash_load_end = LOADADDR(.data) + SIZEOF(.data): linker         */
/*   script'teki tanimi yuk goruntusunden SONRAKI ilk byte'tir.         */
/*                                                                      */
/* TASMA OLMAZ: "addr + 4 <= end" gibi bir toplama YAPILMIYOR. Once     */
/* addr'in [start, end] icinde oldugu gosteriliyor; ancak ondan sonra   */
/* "end - addr" hesaplaniyor ve bu fark negatif olamaz. 0xFFFFFFFC gibi */
/* bir deger ikinci kontrolde reddedilir; toplama hic yapilmadigi icin   */
/* sarip (wrap) araliga "geri donemez".                                  */
/* ------------------------------------------------------------------ */
static int flash_word_readable(uintptr_t addr)
{
    const uintptr_t start = (uintptr_t)L10_FLASH_ORIGIN;
    const uintptr_t end   = (uintptr_t)&__flash_load_end;   /* HARIC */

    if (addr < start) {
        return 0;
    }
    if (addr > end) {
        return 0;
    }
    if (sizeof(uint32_t) > (end - addr)) {   /* guvenli: addr <= end */
        return 0;
    }
    if ((addr & (sizeof(uint32_t) - 1u)) != 0u) {
        return 0;                             /* .data'daki uint32_t hizali */
    }
    return 1;
}

int main(void)
{
    console_line("");
#ifdef L10_SOLUTION
    console_line("===== LESSON 10 SOLUTION 01 =====");
#else
    console_line("===== LESSON 10 EXERCISE 01 =====");
#endif

    uint32_t flash = flash_used_bytes();
    uint32_t ram   = ram_static_bytes();
    uint32_t lma   = data_lma_of(&g_init);

    console_kv_u32("  FLASH_USED_BYTES", flash);
    console_kv_u32("  RAM_STATIC_BYTES", ram);
    console_kv_hex("  G_INIT_LMA", lma);

    /* Linker'in kendi hesabi: script'teki __flash_load_end. */
    uint32_t flash_expected = A(&__flash_load_end) - L10_FLASH_ORIGIN;
    uint32_t ram_expected   = (A(&__data_end) - A(&__data_start)) +
                              (A(&__bss_end)  - A(&__bss_start));

    console_kv_str("  CHECK_FLASH_USED", pf(flash == flash_expected));
    console_kv_str("  CHECK_RAM_STATIC", pf(ram == ram_expected));

    /* LMA ancak TUM kontroller gectikten SONRA okunur (flash_word_readable). */
    int lma_ok = 0;
    if (flash_word_readable((uintptr_t)lma)) {
        lma_ok = (*(const volatile uint32_t *)(uintptr_t)lma == 0xC0DEFACEu);
    }
    console_kv_str("  CHECK_LMA_HOLDS_INIT", pf(lma_ok));

    int regions_ok =
        in_region(k_table, L10_FLASH_ORIGIN, L10_FLASH_LENGTH) &&
        in_region(&g_init, L10_RAM_ORIGIN, L10_RAM_LENGTH) &&
        in_region(&g_zero, L10_RAM_ORIGIN, L10_RAM_LENGTH) &&
        !in_region(k_table, L10_RAM_ORIGIN, L10_RAM_LENGTH) &&
        !in_region(&g_init, L10_FLASH_ORIGIN, L10_FLASH_LENGTH);
    console_kv_str("  CHECK_REGIONS", pf(regions_ok));

    console_line("===== END =====");
    for (;;) {
        __WFI();
    }
}
