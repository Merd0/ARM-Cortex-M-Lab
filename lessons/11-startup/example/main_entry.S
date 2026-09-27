/*
 * Ders 11 - main icin kucuk bir assembly giris kapisi (shim).
 *
 * Neden? C'de yazilmis bir main'in ILK satirina geldigimizde derleyici
 * prologu SP'yi coktan degistirmis olur. Bu shim, Reset_Handler'in
 * "bl main" komutunun biraktigi durumu HICBIR SEYE dokunmadan yakalar:
 *
 *   r0 <- SP  (bl main anindaki SP; BL, SP'yi degistirmez)
 *   r1 <- LR  (Reset_Handler'a donus adresi, Thumb biti dahil)
 *
 * ve C'deki l11_main(sp, lr)'ye KUYRUK DALLANMASI (b.w) yapar.
 *
 * AAPCS: argumanlar r0/r1'de; hicbir callee-saved register'a dokunulmuyor;
 * SP degismiyor; LR degismiyor. l11_main dondugunde dogrudan
 * Reset_Handler'a doner - yani "main dondu" davranisinin aynisi.
 */
    .syntax unified
    .thumb
    .text

    .global main
    .type   main, %function
    .thumb_func
main:
    mov     r0, sp
    mov     r1, lr
    b.w     l11_main
    .size   main, . - main
