/*
 * SOLUTION 01 - Locate Your Data
 * Aciklama: solutions/01-locate-your-data/README.md
 */
#include "l10.h"
#include "locate.h"

static uint32_t A(const void *p) { return (uint32_t)(uintptr_t)p; }

uint32_t flash_used_bytes(void)
{
    /* FLASH yuk goruntusu: vektorlerin basindan .data'nin LMA'sina kadar
     * (kod + salt-okunur veri + hizalama), ARTI .data'nin FLASH'taki kopyasi.
     * .bss FLASH'ta yer KAPLAMAZ. */
    uint32_t ro_part   = A(&__etext) - A(&__vectors_start);
    uint32_t data_copy = A(&__data_end) - A(&__data_start);
    return ro_part + data_copy;
}

uint32_t ram_static_bytes(void)
{
    /* .data RAM'de de yer kaplar (VMA), .bss yalnizca RAM'de. Stack burada
     * sayilmiyor: bir section degil, linker script'te bir muhasebe. */
    return (A(&__data_end) - A(&__data_start)) +
           (A(&__bss_end)  - A(&__bss_start));
}

uint32_t data_lma_of(const void *p)
{
    /* .data, RAM'deki (VMA) ile FLASH'taki (LMA) halinde AYNI sirada
     * yerlesir. Nesnenin .data icindeki ofseti iki yerde de aynidir.
     * __etext = LOADADDR(.data). */
    return A(&__etext) + (A(p) - A(&__data_start));
}

int in_region(const void *p, uint32_t origin, uint32_t length)
{
    uint32_t a = A(p);
    return (a >= origin) && ((a - origin) < length);
}
