/*
 * push_pop_probe - PUSH/POP'un SP uzerindeki etkisini OLCER.
 *
 *     void push_pop_probe(uint32_t out[3]);
 *
 *     out[0] = PUSH'tan once SP
 *     out[1] = PUSH'tan sonra SP
 *     out[2] = POP'tan sonra SP
 *
 * AAPCS: r0 = out (tek argument). Fonksiyon baska fonksiyon cagirmadigi icin
 * (leaf) lr'yi saklamaya gerek yok.
 *
 * r1'i serbestce kullaniyoruz cunku r0-r3 caller'in sorumlulugundadir.
 * Dokundugumuz r4-r7 ise callee-saved'dir - zaten push/pop ile geri
 * veriliyor, yani kural ihlal edilmiyor (bkz. ders 03).
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global push_pop_probe
    .type   push_pop_probe, %function
    .thumb_func
push_pop_probe:
    mov     r1, sp
    str     r1, [r0, #0]        /* out[0] = SP (once) */

    push    {r4, r5, r6, r7}    /* 4 register x 4 byte = 16 byte */

    mov     r1, sp
    str     r1, [r0, #4]        /* out[1] = SP (push sonrasi) */

    pop     {r4, r5, r6, r7}

    mov     r1, sp
    str     r1, [r0, #8]        /* out[2] = SP (pop sonrasi) */

    bx      lr
    .size   push_pop_probe, . - push_pop_probe
