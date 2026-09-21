/*
 * SOLUTION 02 - multiply_add (Experiment'in referans cozumu)
 *
 *     int multiply_add(int a, int b, int c);   ->  a * b + c
 *
 * AAPCS: uc tamsayi argument sirayla r0, r1, r2'ye gelir.
 * Donus degeri yine r0.
 *
 * Iki yol var:
 *
 *   1) muls r0, r0, r1 ;  adds r0, r0, r2      -> iki komut, 16-bit x2
 *   2) mla  r0, r0, r1, r2                     -> tek komut, 32-bit
 *
 * MLA (Multiply Accumulate) Armv7-M ve sonrasinda vardir, yani
 * Cortex-M33'te kullanilabilir. Ama Armv6-M'de (Cortex-M0/M0+) YOKTUR.
 * Burada bilerek MLA kullaniyoruz - tek komutta ayni isi yapmasi,
 * "hangi komutun hangi mimaride oldugu" sorusunu somutlastiriyor.
 *
 * MLA bayraklari GUNCELLEMEZ ve 's' sonekli bir surumu yoktur.
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global multiply_add
    .type   multiply_add, %function
    .thumb_func
multiply_add:
    mla     r0, r0, r1, r2      /* r0 = (a * b) + c */
    bx      lr
    .size   multiply_add, . - multiply_add
