/*
 * Lesson 10 - Linker, ELF & Memory
 *
 * Ayri derlenmis parcalar (objects.c, placement.c, l10_asm.S, main.c)
 * tek bir ELF'te nasil birlesiyor, ve kod/veri Cortex-M belleginde nereye
 * yerlesiyor?
 *
 * Bu program CALISMA ZAMANINDA gorulebilen kismi gosterir: adresler,
 * degerler ve linker sembolleri. Geri kalani (section'lar, semboller,
 * relocation'lar, map dosyasi) ELF ve .map uzerinden incelenir - bkz. README.
 */

#include "platform/mps2_an505.h"
#include "common/console.h"
#include "l10.h"

static const char *pf(int ok) { return ok ? "PASS" : "FAIL"; }

static uint32_t A(const void *p) { return (uint32_t)(uintptr_t)p; }

static int in_range(uint32_t a, const void *lo, const void *hi)
{
    return (a >= A(lo)) && (a < A(hi));
}

int main(void)
{
    console_line("");
    console_line("===== LESSON 10: LINKER, ELF & MEMORY =====");

    /* g_zero ve s_calls'i HENUZ yazmadan once okuyalim. */
    uint32_t zero_before  = g_zero;
    uint32_t calls_before = *counter_addr();

    /* ------------------------------------------------------------ */
    console_line("[1] Nesneler nereye yerlesti? (calisma zamani adresleri)");
    uint32_t fn_ptr = (uint32_t)(uintptr_t)&text_fn;
    console_kv_hex("  text_fn (fonksiyon isaretcisi)", fn_ptr);
    console_kv_hex("  k_table", A(k_table));
    console_kv_hex("  k_custom (.rodata.l10_table)", A(k_custom));
    console_kv_hex("  g_init", A(&g_init));
    console_kv_hex("  asm_word", A(&asm_word));
    console_kv_hex("  g_zero", A(&g_zero));
    console_kv_hex("  s_calls (static yerel)", A(counter_addr()));

    /* ------------------------------------------------------------ */
    console_line("[2] Linker sembolleri");
    console_kv_hex("  __vectors_start", A(&__vectors_start));
    console_kv_hex("  __vectors_end", A(&__vectors_end));
    console_kv_hex("  __etext (=LOADADDR(.data))", A(&__etext));
    console_kv_hex("  __data_start", A(&__data_start));
    console_kv_hex("  __data_end", A(&__data_end));
    console_kv_hex("  __bss_start", A(&__bss_start));
    console_kv_hex("  __bss_end", A(&__bss_end));
    console_kv_hex("  __flash_load_end", A(&__flash_load_end));
    console_kv_hex("  __stack_top", A(&__stack_top));

    /* ------------------------------------------------------------ */
    console_line("[3] .data: VMA (RAM) ve LMA (FLASH)");
    uint32_t off = A(&g_init) - A(&__data_start);
    uint32_t lma = A(&__etext) + off;
    console_kv_hex("  g_init VMA", A(&g_init));
    console_kv_hex("  g_init LMA", lma);
    console_kv_hex("  deger @ LMA (FLASH)", *(const volatile uint32_t *)lma);
    console_kv_hex("  deger @ VMA (RAM)", g_init);

    /* ------------------------------------------------------------ */
    console_line("[4] Kontroller");
    const uint32_t flash_end = L10_FLASH_ORIGIN + L10_FLASH_LENGTH;

    console_kv_str("  CHECK_TEXT_IN_FLASH",
        pf(((fn_ptr & ~1u) >= L10_FLASH_ORIGIN) && ((fn_ptr & ~1u) < flash_end)));
    console_kv_str("  CHECK_FN_PTR_THUMB_BIT", pf((fn_ptr & 1u) == 1u));
    console_kv_str("  CHECK_RODATA_IN_FLASH_RO",
        pf(in_range(A(k_table),  &__vectors_end, &__etext) &&
           in_range(A(k_custom), &__vectors_end, &__etext)));
    console_kv_str("  CHECK_DATA_IN_DATA",
        pf(in_range(A(&g_init),   &__data_start, &__data_end) &&
           in_range(A(&asm_word), &__data_start, &__data_end)));
    console_kv_str("  CHECK_BSS_IN_BSS",
        pf(in_range(A(&g_zero),         &__bss_start, &__bss_end) &&
           in_range(A(counter_addr()),  &__bss_start, &__bss_end)));
    console_kv_str("  CHECK_DATA_LMA_HOLDS_INIT",
        pf(*(const volatile uint32_t *)lma == 0xC0DEFACEu));
    console_kv_str("  CHECK_DATA_COPIED", pf(g_init == 0xC0DEFACEu));
    console_kv_str("  CHECK_BSS_ZERO", pf(zero_before == 0u && calls_before == 0u));
    console_kv_str("  CHECK_STATIC_LOCAL", pf(counter_next() == 1u && counter_next() == 2u));
    console_kv_str("  CHECK_FLASH_LOAD_END",
        pf(A(&__flash_load_end) ==
           A(&__etext) + (A(&__data_end) - A(&__data_start))));
    console_kv_str("  CHECK_VECTOR0_IS_STACK_TOP",
        pf(__vectors_start == A(&__stack_top)));
    console_kv_str("  CHECK_CROSS_TU_CALL", pf(text_fn(10u) == 31u));
    console_kv_str("  CHECK_ASM_LINKED",
        pf(asm_fn(5u) == 31u && asm_word == 0x5A5A5A5Au));

    console_line("===== END =====");
    for (;;) {
        __WFI();
    }
}
