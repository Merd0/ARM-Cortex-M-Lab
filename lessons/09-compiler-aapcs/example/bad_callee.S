/*
 * Ders 09 - BREAK IT: bilerek AAPCS ihlali.
 *
 *   uint32_t bad_callee(uint32_t a, uint32_t b)  ->  a + b   (DOGRU sonuc)
 *
 * r4'u (callee-saved) SAKLAMADAN degistirir. Sonuc dogrudur; zarar
 * cagiranin r4'teki degerine, daha sonra, baskasinin kodunda cikar.
 */
    .syntax unified
    .thumb
    .text

    .global bad_callee
    .type   bad_callee, %function
    .thumb_func
bad_callee:
    movs    r4, #0              /* IHLAL: callee-saved r4 saklanmadan ezildi */
    adds    r0, r0, r1          /* sonuc yine de dogru */
    bx      lr
    .size   bad_callee, . - bad_callee
