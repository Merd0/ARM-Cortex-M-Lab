.syntax unified
.cpu cortex-m33
.thumb
.text
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
1:
    ldr r0,[sp]
    movs r1,#1
.macro canary reg,offset
    add r2,r0,#\offset
    cmp \reg,r2
    it ne
    movne r1,#0
.endm
    canary r4,0x40
    canary r5,0x50
    canary r6,0x60
    canary r7,0x70
    canary r8,0x80
    canary r9,0x90
    canary r10,0xA0
    canary r11,0xB0
    bl node_step
#if NODE_TEST == 5
    ldr r0,[sp]
    cmp r0,#3
    beq node_spin
#endif
    movw r0,#2000
2:  subs r0,r0,#1
    bne 2b
    svc #3
.global node_after_yield
node_after_yield:
    b 1b
.global node_spin
.type node_spin,%function
.thumb_func
node_spin:
    nop
    b node_spin
.size l15_task_entry,.-l15_task_entry

.global node_overflow
.type node_overflow,%function
.thumb_func
node_overflow:
    add r0,r0,#16
    mov sp,r0
    sub sp,sp,#32
    b l15_panic
.size node_overflow,.-node_overflow

.global node_fault_entry
.type node_fault_entry,%function
.thumb_func
node_fault_entry:
    tst lr,#4
    ite eq
    mrseq r0,msp
    mrsne r0,psp
    mov r1,lr
    b node_fault
.size node_fault_entry,.-node_fault_entry
.section .note.GNU-stack,"",%progbits
