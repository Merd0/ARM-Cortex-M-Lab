/*
 * EXERCISE 01 - sub_numbers
 *
 * C tarafindaki bildirim:
 *     int sub_numbers(int a, int b);     ->  a - b
 *
 * GOREV
 *   Asagidaki fonksiyonu doldur. Kurallar:
 *     - argumentler AAPCS'e gore hangi register'lardan gelir? (bul)
 *     - sonuc hangi register'da donmeli? (bul)
 *     - donus nasil yapilir?
 *
 * Su an fonksiyon sabit 0 donuyor; yani derleniyor ama YANLIS.
 *
 * DOGRULAMA
 *   sub_numbers(12, 5)  ->  7
 *   sub_numbers(5, 12)  ->  -7   (konsol isaretsiz basar: 4294967289 = 0xFFFFFFF9)
 *
 * IPUCU
 *   Tek bir aritmetik komut ve tek bir donus komutu yeterli.
 *   Cikarma komutunun ismi icin: Armv8-M ARM, "SUB (register)".
 *
 * Referans cozum: solutions/01-sub-numbers/
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global sub_numbers
    .type   sub_numbers, %function
    .thumb_func
sub_numbers:
    /* TODO(student): a - b hesapla ve dondur. */
    movs    r0, #0
    bx      lr
    .size   sub_numbers, . - sub_numbers
