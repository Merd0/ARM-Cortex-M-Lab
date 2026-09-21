/*
 * SVC_Handler sarmalayicisi - ders 07.
 *
 * Ders 05'teki surumden farki: exception GIRISINDEKI durumu kaydediyor.
 * Bunlar handler icinde C kodu calistiktan SONRA artik guvenilir sekilde
 * okunamaz:
 *
 *   lr  -> EXC_RETURN. Herhangi bir "bl" onu ezer.
 *   msp -> handler'in kendi prolog'u onu asagi iter.
 *
 * Bu yuzden ilk is onlari kaydetmek.
 *
 * DESTEKLENEN VARSAYIMLAR (bu ders):
 *   standart 8 word frame (soft-float build), tek guvenlik durumu,
 *   ek durum baglami yok (DCRS = 1).
 *   Genel bir exception cercevesi DEGILDIR.
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global SVC_Handler
    .type   SVC_Handler, %function
    .thumb_func
SVC_Handler:
    ldr     r2, =g_entry_lr
    str     lr, [r2]            /* EXC_RETURN - ilk is bu          */

    mrs     r3, msp
    ldr     r2, =g_entry_msp
    str     r3, [r2]            /* giristeki MSP                   */

    mrs     r3, psp
    ldr     r2, =g_entry_psp
    str     r3, [r2]            /* giristeki PSP                   */

    tst     lr, #4              /* EXC_RETURN bit2: 0 = MSP, 1 = PSP */
    ite     eq
    mrseq   r0, msp
    mrsne   r0, psp             /* r0 = frame adresi               */
    ldr     r2, =g_frame_addr
    str     r0, [r2]

    mov     r1, lr              /* r1 = EXC_RETURN                 */
    b       svc_dispatch        /* lr KORUNUR -> donusu o yapar    */
    .size   SVC_Handler, . - SVC_Handler
