/*
 * mpu_store_word - KONTROLLU bir veri yazmasi. Ders 06 "Break It" tetikleyicisi.
 *
 *     void mpu_store_word(volatile uint32_t *addr, uint32_t value);
 *
 * Neden assembly?
 *   Hata ureten komutun TAM OLARAK hangisi oldugunu bilmek istiyoruz.
 *   C'de yazsaydik derleyici store'u baska bir komuta cevirebilir, baska
 *   bir sira uretebilir veya tamamen optimize edebilirdi. Burada tek bir
 *   "str" var ve adresi disassembly'de gosterebiliyoruz.
 *
 * AAPCS: r0 = addr, r1 = value. Yaprak fonksiyon; stack'e dokunmuyor.
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global mpu_store_word
    .type   mpu_store_word, %function
    .thumb_func
mpu_store_word:
    str     r1, [r0]            /* hata BURADA olusacak */
    bx      lr
    .size   mpu_store_word, . - mpu_store_word
