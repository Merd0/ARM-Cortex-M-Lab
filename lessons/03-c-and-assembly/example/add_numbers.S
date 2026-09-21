/*
 * add_numbers - elle yazilmis (handwritten) assembly fonksiyonu.
 *
 * C tarafindaki bildirim:
 *     int add_numbers(int a, int b);
 *
 * AAPCS geregi:
 *     r0 = birinci argument (a)
 *     r1 = ikinci  argument (b)
 *     r0 = donus degeri
 *     lr = cagiranin donus adresi
 *
 * Bu fonksiyon yaprak (leaf) bir fonksiyondur: baska fonksiyon cagirmaz,
 * bu yuzden lr'yi stack'e itmeye gerek yoktur.
 */

    .syntax unified              /* Tek, modern assembler sozdizimi   */
    .thumb                       /* Cortex-M yalnizca Thumb calistirir */
    .text
    .align  2

    .global add_numbers
    .type   add_numbers, %function
    .thumb_func                  /* Sembolun bit0'i 1 olarak isaretlenir */

add_numbers:
    adds    r0, r0, r1           /* r0 = a + b  (bayraklari da gunceller) */
    bx      lr                   /* lr'deki adrese don                    */

    .size   add_numbers, . - add_numbers
