/*
 * EXERCISE 01 - Repair an ABI-Broken Assembly Function
 *
 *   uint32_t scale_and_sum(uint32_t a, uint32_t b)
 *       ->  helper_scale(a) + b
 *
 * Bu fonksiyon DOGRU sonucu dondurur. Ama AAPCS sozlesmesini IKI yerden
 * bozuyor. Firmware'in kontrolleri:
 *
 *   CHECK_RESULT              PASS   <- sonuc dogru
 *   CHECK_CALLEE_SAVED        FAIL   <- harness bir register'in bozuldugunu buldu
 *   CHECK_SP_ALIGNED_AT_CALL  FAIL   <- BL aninda SP 8'e hizali degil
 *
 * GOREV: ikisini de onar. Sonuc dogru kalmali.
 *
 * Cevaplaman gereken sorular (README'de de var):
 *   - b cagri boyunca hangi register'da tutuluyor? O register kimin?
 *   - BL aninda SP'nin 8'in kati olmasi icin toplam kac byte yigilmali?
 *   - LR neden saklaniyor? Donus neden "pop {..., pc}" ile yapiliyor?
 *
 * BL'den hemen onceki SP kaydini (g_ex_sp_at_call) SILME: kontrol onu
 * kullaniyor.
 *
 * Referans cozum: solutions/01-callee-save/
 */
    .syntax unified
    .thumb
    .text

    .global scale_and_sum
    .type   scale_and_sum, %function
    .thumb_func
scale_and_sum:
    push    {r4, lr}
    mov     r4, r0              /* a */
    mov     r5, r1              /* b - cagri boyunca burada tutuluyor */

    sub     sp, sp, #4          /* yerel bir "calisma alani" */

    ldr     r2, =g_ex_sp_at_call
    mov     r3, sp
    str     r3, [r2]            /* BL aninda SP - SILME */

    mov     r0, r4
    bl      helper_scale        /* r0 = helper_scale(a) */

    add     sp, sp, #4
    add     r0, r0, r5          /* + b */
    pop     {r4, pc}
    .size   scale_and_sum, . - scale_and_sum
