/*
 * SOLUTION 01 - sub_numbers
 *
 *     int sub_numbers(int a, int b);   ->  a - b
 *
 * AAPCS: r0 = a, r1 = b, donus degeri r0, donus adresi lr.
 * Yaprak fonksiyon oldugu icin stack'e hicbir sey itilmez.
 *
 * Neden "subs" ve "sub" degil?
 *   Ikisi de dogru sonucu uretir; fark bayraklari (APSR N/Z/C/V)
 *   guncelleyip guncellememeleridir. Unified syntax'ta 's' soneki
 *   "bayraklari guncelle" demektir. Burada bayraklari kimse okumadigi
 *   icin ikisi de calisir; "subs" secildi cunku dusuk register'larla
 *   16-bit kodlamaya (2 byte) sigar. Uretilen kodlamayi kendi gozunle
 *   gormek icin README'deki objdump komutunu calistir.
 *
 * Exercise: exercises/01-sub-numbers/
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global sub_numbers
    .type   sub_numbers, %function
    .thumb_func
sub_numbers:
    subs    r0, r0, r1          /* r0 = a - b */
    bx      lr                  /* lr'ye don  */
    .size   sub_numbers, . - sub_numbers
