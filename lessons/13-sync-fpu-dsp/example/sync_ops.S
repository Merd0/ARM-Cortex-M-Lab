    .syntax unified
    .thumb
    .text
    .align 2

/* Pend SysTick after the load, then wait for its one update. r2 holds the
 * interrupted value; hardware exception stacking preserves it. No BL occurs
 * inside an exclusive sequence. All waits and retries are bounded. */
.macro INJECT_AND_WAIT timeout
    ldr r0, =0xE000ED04
    mov r1, #0x04000000
    str r1, [r0]
    dsb sy
    isb sy
    ldr r0, =g_l13_irq_count
    movw r12, #1024
1:
    ldr r1, [r0]
    cbnz r1, 2f
    subs r12, r12, #1
    bne 1b
    b \timeout
2:
.endm

    .global l13_racy_increment
    .type l13_racy_increment, %function
    .thumb_func
l13_racy_increment:
    push {r4, lr}                 /* AAPCS: preserve r4; SP stays 8-aligned. */
    mov r4, r0
    ldr r2, [r4]
    INJECT_AND_WAIT .Lracy_timeout
    adds r2, r2, #1
    str r2, [r4]                  /* Overwrites handler's increment. */
    movs r0, #1
    pop {r4, pc}
.Lracy_timeout:
    movs r0, #0
    pop {r4, pc}
    .size l13_racy_increment, .-l13_racy_increment

    .global l13_exclusive_increment
    .type l13_exclusive_increment, %function
    .thumb_func
l13_exclusive_increment:
    push {r4-r8, lr}              /* 24 bytes; r9-r11 never touched. */
    mov r4, r0
    mov r5, r1                    /* positive attempt budget */
    movs r6, #0                   /* failed STREX count */
    movs r7, #0                   /* inject exactly once */
    cbz r5, .Lexclusive_failed
    dmb sy
.Lretry:
    ldrex r2, [r4]
    cbnz r7, .Lstore
    movs r7, #1
    INJECT_AND_WAIT .Lexclusive_failed
.Lstore:
    adds r2, r2, #1
    strex r3, r2, [r4]
    cbnz r6, .Lstatus_saved
    ldr r0, =g_l13_first_status
    str r3, [r0]
.Lstatus_saved:
    cbz r3, .Lexclusive_ok
    adds r6, r6, #1
    subs r5, r5, #1
    bne .Lretry
.Lexclusive_failed:
    clrex
    movs r0, #0                   /* Caller must handle exhaustion. */
    b .Lexclusive_done
.Lexclusive_ok:
    dmb sy
    movs r0, #1
.Lexclusive_done:
    ldr r1, =g_l13_retries
    str r6, [r1]
    pop {r4-r8, pc}
    .size l13_exclusive_increment, .-l13_exclusive_increment
