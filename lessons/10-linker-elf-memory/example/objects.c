/*
 * Ders 10 - her yerlesim sinifindan bir nesne.
 *
 * -ffunction-sections / -fdata-sections acik oldugu icin her nesne KENDI
 * input section'ina gider (.rodata.k_table, .data.g_init, ...). Linker
 * script'teki *(.rodata*) / *(.data*) / *(.bss*) desenleri onlari cikis
 * section'larina toplar.
 */
#include "l10.h"

/* const -> salt-okunur veri */
const uint32_t k_table[4] = { 0x11111111u, 0x22222222u, 0x33333333u, 0x44444444u };

/* OZEL input section adi. Linker script'e dokunmadan, mevcut *(.rodata*)
 * deseni tarafindan .rodata cikis section'ina alinir. */
__attribute__((section(".rodata.l10_table")))
const uint32_t k_custom[2] = { 0xAAAA0001u, 0xAAAA0002u };

/* baslatilmis global -> .data (RAM'de calisir, ilk degeri FLASH'ta durur) */
uint32_t g_init = 0xC0DEFACEu;

/* sifirla baslatilan global -> .bss */
uint32_t g_zero;

/* static YEREL -> yine .bss. Adi, derleyicinin urettigi LOCAL bir sembol
 * olur (ornegin "s_calls.0"); baska bir TU onu adiyla goremez. Adresini
 * disari vermek icin bu fonksiyon var. */
uint32_t *counter_addr(void)
{
    static uint32_t s_calls;
    return &s_calls;
}

uint32_t counter_next(void)
{
    uint32_t *p = counter_addr();
    *p = *p + 1u;
    return *p;
}

/* ---------------------------------------------------------------- */
/* GC gozlemi: HICBIR yerden referans verilmeyen nesneler.            */
/* ---------------------------------------------------------------- */

/* Duz, kullanilmayan veri -> --gc-sections atmali. */
uint32_t g_unused_data = 0x0BADBEEFu;

/* "used": DERLEYICIYE "bunu silme" der. LINKER'a bir sey soyler mi?
 * Olculecek (README, "used vs KEEP"). */
__attribute__((used))
const uint32_t k_unused_used[2] = { 0x05ED0001u, 0x05ED0002u };
