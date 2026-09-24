/*
 * SOLUTION 01 - Repair an ABI-Broken Assembly Function
 *
 *   uint32_t scale_and_sum(uint32_t a, uint32_t b)
 *       ->  helper_scale(a) + b
 *
 * Iki onarim (ayrinti: solutions/01-callee-save/README.md):
 *   1. r5 callee-saved: kullaniyorsak saklamak zorundayiz.
 *   2. BL aninda SP 8'e hizali olmali: yigilan toplam byte 8'in kati,
 *      ve hizalamayi bozan gereksiz "sub sp, #4" kaldirildi.
 * r3 burada yalnizca DOLGU: {r3, r4, r5, lr} = 16 byte.
 */
    .syntax unified
    .thumb
    .text

    .global scale_and_sum
    .type   scale_and_sum, %function
    .thumb_func
scale_and_sum:
    push    {r3, r4, r5, lr}    /* 16 byte: r4/r5 korunur, SP 8'e hizali */
    mov     r4, r0              /* a */
    mov     r5, r1              /* b - artik korunan bir register'da */

    ldr     r2, =g_ex_sp_at_call
    mov     r3, sp
    str     r3, [r2]            /* BL aninda SP */

    mov     r0, r4
    bl      helper_scale        /* r0 = helper_scale(a) */

    add     r0, r0, r5          /* + b */
    pop     {r3, r4, r5, pc}    /* r4/r5 geri; r3 dolgu; donus */
    .size   scale_and_sum, . - scale_and_sum
