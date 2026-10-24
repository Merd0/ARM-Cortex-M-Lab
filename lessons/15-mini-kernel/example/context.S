.syntax unified
.cpu cortex-m33
.thumb
.text
.align 2

.global l15_start
.type l15_start,%function
.thumb_func
l15_start:
    svc #1
    b l15_panic
.size l15_start,.-l15_start

.global SVC_Handler
.type SVC_Handler,%function
.thumb_func
SVC_Handler:
    tst lr,#4
    ite eq
    mrseq r0,msp
    mrsne r0,psp
    ldr r1,[r0,#24]
    ldrb r1,[r1,#-2]
    cmp r1,#0
    beq l15_yield
    cmp r1,#2
    beq l15_finish
    cmp r1,#1
    bne l15_panic
    ldr r0,=g_l15_booted
    ldr r1,[r0]
    cbnz r1,1f
    movs r1,#1
    str r1,[r0]
    ldr r0,=g_l15_current
    ldr r0,[r0]
    ldr r1,[r0,#4]
    msr psplim,r1
    ldr r0,[r0]
    movs r1,#2
    msr control,r1
    isb
    ldmia r0!,{r4-r11,lr}
    msr psp,r0
    /* No C frame crosses this boundary. Old main/MSP frame is abandoned. */
    ldr r0,=__stack_top
    msr msp,r0
    ldr r0,=0xE000E010
    movs r1,#7
    str r1,[r0]
    dsb
    isb
    bx lr
1:  b l15_panic
.size SVC_Handler,.-SVC_Handler

.global PendSV_Handler
.type PendSV_Handler,%function
.thumb_func
PendSV_Handler:
    ldr r1,=0xFFFFFFFD
    cmp lr,r1
    bne l15_panic
    mrs r0,psp
    tst r0,#7
    bne l15_panic
    ldr r3,=g_l15_current
    ldr r3,[r3]
    /* PSPLIM does not protect an indirect STM: check before the first store. */
    subs r1,r0,#36
    ldr r2,[r3,#4]
    cmp r1,r2
    blo l15_panic
    adds r1,r0,#32
    ldr r2,[r3,#8]
    cmp r1,r2
    bhi l15_panic
#ifdef L15_BROKEN_SAVE
    sub r0,r0,#36
    str lr,[r0,#32]  /* Deliberate defect: old R4-R11 slots remain stale. */
#else
    stmdb r0!,{r4-r11,lr}
#endif
    str r0,[r3]
    mrs r2,primask
    cpsid i
    push {r2,lr}     /* 8-byte-aligned public C interface on MSP. */
    bl l15_schedule
    pop {r2,lr}
    mov r12,r2
    ldr r1,[r0,#4]
    msr psplim,r1
    ldr r0,[r0]
    ldmia r0!,{r4-r11,lr}
    msr psp,r0
    msr primask,r12
    isb
    bx lr
.size PendSV_Handler,.-PendSV_Handler

.global l15_task_entry
.type l15_task_entry,%function
.thumb_func
l15_task_entry:
    sub sp,sp,#8
    str r0,[sp]
    add r4,r0,#0x40
    add r5,r0,#0x50
    add r6,r0,#0x60
    add r7,r0,#0x70
    add r8,r0,#0x80
    add r9,r0,#0x90
    add r10,r0,#0xA0
    add r11,r0,#0xB0
2:
    ldr r0,[sp]
    movs r1,#1
.macro check_reg reg,offset
    add r2,r0,#\offset
    cmp \reg,r2
    it ne
    movne r1,#0
.endm
    check_reg r4,0x40
    check_reg r5,0x50
    check_reg r6,0x60
    check_reg r7,0x70
    check_reg r8,0x80
    check_reg r9,0x90
    check_reg r10,0xA0
    check_reg r11,0xB0
    mrs r2,primask
    str r2,[sp,#4]
    cpsid i
    bl l15_observe
    ldr r2,[sp,#4]
    msr primask,r2
    isb
    cmp r0,#1
    beq 4f
    cmp r0,#2
    bne 3f
    svc #0
.global l15_after_yield
l15_after_yield:
    b 2b
3:  movw r0,#2000
5:  subs r0,r0,#1
    bne 5b
    b 2b
4:  svc #2
    b l15_panic
.size l15_task_entry,.-l15_task_entry
.section .note.GNU-stack,"",%progbits
