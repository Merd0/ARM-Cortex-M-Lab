.syntax unified
.cpu cortex-m33
.thumb
.text
.macro fault name
.global \name
.type \name,%function
.thumb_func
\name:
    tst lr,#4
    ite eq
    mrseq r0,msp
    mrsne r0,psp
    mov r1,lr
    b fa_live_record
.size \name,.-\name
.endm
fault HardFault_Handler
fault MemManage_Handler
fault BusFault_Handler
fault UsageFault_Handler
