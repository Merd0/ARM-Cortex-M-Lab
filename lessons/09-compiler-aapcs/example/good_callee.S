/*
 * Ders 09 - harness icin POZITIF KONTROL.
 *
 *   uint32_t good_callee(uint32_t a, uint32_t b)  ->  a + b
 *
 * r4 ve r5'i kullanir, bu yuzden onlari saklar ve geri yukler.
 * Harness bu fonksiyonda maske = 0 vermiyorsa harness'e GUVENILMEZ.
 * Yaprak fonksiyon: LR'yi saklamaz. push {r4, r5} 8 byte - SP hizali kalir
 * (yaprakta zorunlu da degil; AAPCS 8 byte kuralini cagri sinirinda ister).
 */
    .syntax unified
    .thumb
    .text

    .global good_callee
    .type   good_callee, %function
    .thumb_func
good_callee:
    push    {r4, r5}
    mov     r4, r0
    mov     r5, r1
    adds    r0, r4, r5
    pop     {r4, r5}
    bx      lr
    .size   good_callee, . - good_callee
