/*
 * Cok kucuk libc parcalari.
 * -nostdlib ile link ettigimiz icin, GCC'nin urettigi memset/memcpy
 * cagrilarini kendimiz karsilamak zorundayiz. Boylece C runtime'in
 * sihirli olmadigi da gorulmus olur.
 */
#include <stddef.h>

void *memset(void *dst, int value, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    while (n--) {
        *d++ = (unsigned char)value;
    }
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}
