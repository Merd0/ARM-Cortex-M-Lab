/*
 * Ders 09 - C yardimcilari. BILEREK AYRI bir derleme birimi (TU).
 *
 * Ayni TU'da olsalardi, -O2/-Os'ta GCC hangi register'lari bozduklarini
 * bildigi icin (-fipa-ra) cagiranlarda AAPCS'in izin verdiginden daha
 * iyimser register kullanimi uretebilirdi. Ayri TU'da derleyici yalnizca
 * AAPCS sozlesmesine guvenebilir.
 */
#include "abi.h"

uint32_t helper_scale(uint32_t x)
{
    return (x * 3u) + 1u;
}

uint64_t make_u64(void)
{
    return 0x0123456789ABCDEFull;
}

uint32_t helper_ext(uint32_t x)
{
    return x ^ 0x5A5A5A5Au;
}
