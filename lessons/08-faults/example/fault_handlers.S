/*
 * Ders 08 - fault handler sarmalayicilari.
 *
 * DORT handler ayni seyi yapar: giris anindaki mimari durumu KAYDEDIP
 * ortak C kaydediciye (fault_record) dallanir.
 *
 * Neden once kaydediyoruz?
 *   lr  -> EXC_RETURN. Herhangi bir "bl" onu ezer.
 *   msp -> handler'in C prologu onu asagi iter.
 * Canli handler $sp'sinden kesilen frame'i CIKARMAYA calismak ders 04'teki
 * debugger hatasinin ta kendisiydi.
 *
 * DESTEKLENEN VARSAYIMLAR: standart 8 word frame (soft-float), tek guvenlik
 * durumu. Bu bir uretim fault cercevesi DEGILDIR.
 */

    .syntax unified
    .thumb
    .text
    .align  2

.macro CAPTURE_AND_RECORD
    ldr     r2, =g_entry_lr
    str     lr, [r2]
    mrs     r3, msp
    ldr     r2, =g_entry_msp
    str     r3, [r2]
    mrs     r3, psp
    ldr     r2, =g_entry_psp
    str     r3, [r2]
    tst     lr, #4              /* EXC_RETURN bit2: frame hangi stack'te */
    ite     eq
    mrseq   r0, msp
    mrsne   r0, psp
    mov     r1, lr
    b       fault_record        /* donmez */
.endm

    .global MemManage_Handler
    .type   MemManage_Handler, %function
    .thumb_func
MemManage_Handler:
    CAPTURE_AND_RECORD
    .size   MemManage_Handler, . - MemManage_Handler

    .global BusFault_Handler
    .type   BusFault_Handler, %function
    .thumb_func
BusFault_Handler:
    CAPTURE_AND_RECORD
    .size   BusFault_Handler, . - BusFault_Handler

    .global UsageFault_Handler
    .type   UsageFault_Handler, %function
    .thumb_func
UsageFault_Handler:
    CAPTURE_AND_RECORD
    .size   UsageFault_Handler, . - UsageFault_Handler

    .global HardFault_Handler
    .type   HardFault_Handler, %function
    .thumb_func
HardFault_Handler:
    CAPTURE_AND_RECORD
    .size   HardFault_Handler, . - HardFault_Handler
