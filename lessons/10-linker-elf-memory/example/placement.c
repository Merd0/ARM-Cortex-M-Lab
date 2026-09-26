/*
 * Ders 10 - ayri bir derleme birimindeki kod.
 *
 * main.c text_fn'i cagirdiginda derleyici hedefin adresini BILMEZ:
 * main.o icinde bir BL ve bir relocation kalir; adresi linker doldurur.
 */
#include "l10.h"

__attribute__((noinline))
uint32_t text_fn(uint32_t x)
{
    return (x * 3u) + 1u;
}

/* GC gozlemi: hicbir yerden cagrilmayan fonksiyon. */
uint32_t unused_function(uint32_t x);
uint32_t unused_function(uint32_t x)
{
    return x ^ 0xDEADDEADu;
}
