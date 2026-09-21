/*
 * MemManage ve HardFault yakalayicilari - ders 07 "Break It" icin.
 *
 * Her ikisi de giristeki EXC_RETURN / MSP / PSP degerlerini kaydedip
 * ortak bir C rapor fonksiyonuna dallanir. Kayit ilk is olmali: handler
 * icinde C calisinca lr ve msp artik giris degerleri degildir.
 */

    .syntax unified
    .thumb
    .text
    .align  2

.macro CAPTURE_AND_BRANCH target
    ldr     r2, =g_entry_lr
    str     lr, [r2]
    mrs     r3, msp
    ldr     r2, =g_entry_msp
    str     r3, [r2]
    mrs     r3, psp
    ldr     r2, =g_entry_psp
    str     r3, [r2]
    tst     lr, #4
    ite     eq
    mrseq   r0, msp
    mrsne   r0, psp
    ldr     r2, =g_frame_addr
    str     r0, [r2]
    mov     r1, lr
    b       \target
.endm

    .global MemManage_Handler
    .type   MemManage_Handler, %function
    .thumb_func
MemManage_Handler:
    CAPTURE_AND_BRANCH fault_report
    .size   MemManage_Handler, . - MemManage_Handler

    .global HardFault_Handler
    .type   HardFault_Handler, %function
    .thumb_func
HardFault_Handler:
    CAPTURE_AND_BRANCH fault_report
    .size   HardFault_Handler, . - HardFault_Handler
