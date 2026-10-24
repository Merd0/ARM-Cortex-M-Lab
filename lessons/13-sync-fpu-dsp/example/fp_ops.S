    .syntax unified
    .thumb
    .fpu fpv5-sp-d16
    .text
    .align 2

    .global l13_fp_run
    .type l13_fp_run, %function
    .thumb_func
l13_fp_run:
    push {r4, lr}
    mov r4, r0
    cbz r4, .Lno_fp
    .global l13_fp_first
l13_fp_first:
    vmov.f32 s0, #1.5
    vmov.f32 s1, #2.0
    movs r0, #0
    vmsr fpscr, r0
    vadd.f32 s2, s0, s1
.Lno_fp:
    /* Seed the future 104-byte frame inside a temporarily allocated stack
     * area. Restore SP before SVC; no C frame or stack-switch trick is used. */
    sub sp, sp, #104
    mov r0, sp
    movs r1, #26
    ldr r2, =0xA5A5A5A5
.Lseed_frame:
    str r2, [r0], #4
    subs r1, r1, #1
    bne .Lseed_frame
    add sp, sp, #104
    ldr r0, =g_l13_thread_control
    mrs r1, control
    str r1, [r0]
    ldr r0, =g_l13_before_sp
    mov r1, sp
    str r1, [r0]
    svc #13
    .global l13_after_svc
l13_after_svc:
    movs r0, #0
    cbz r4, .Lfp_return
    vmov r0, s0                    /* Must be restored to 1.5 after exception. */
.Lfp_return:
    pop {r4, pc}
    .size l13_fp_run, .-l13_fp_run

    .global SVC_Handler
    .type SVC_Handler, %function
    .thumb_func
SVC_Handler:
    /* Capture before any software frame or floating-point instruction. */
    mrs r0, msp
    ldr r2, =g_l13_entry_sp
    str r0, [r2]
    ldr r2, =g_l13_exc_return
    str lr, [r2]
    ldr r2, =0xE000EF34
    ldr r3, [r2]
    ldr r2, =g_l13_fpccr_entry
    str r3, [r2]
    movs r3, #0
    tst lr, #0x10
    it eq
    ldreq r3, [r0, #32]           /* Raw slot, not a valid FP value if lazy. */
    ldr r2, =g_l13_fp_slot_before
    str r3, [r2]
    push {r4, lr}                  /* Preserve callee-saved r4 and EXC_RETURN. */
    mov r4, r0
    ldr r2, =g_l13_fp_active
    ldr r2, [r2]
    cbz r2, .Linteger_handler
    vmov.f32 s0, #8.0             /* Triggers lazy save, then clobbers live s0. */
.Linteger_handler:
    ldr r2, =0xE000EF34
    ldr r3, [r2]
    ldr r2, =g_l13_fpccr_after
    str r3, [r2]
    mov r0, r4
    bl l13_capture_frame           /* Integer-only C; SP is still 8-aligned. */
    .global l13_frame_ready
l13_frame_ready:
    pop {r4, lr}
    bx lr
    .size SVC_Handler, .-SVC_Handler

    .global UsageFault_Handler
    .type UsageFault_Handler, %function
    .thumb_func
UsageFault_Handler:
    mrs r0, msp
    mov r1, lr
    b l13_nocp_report              /* Never returns; no FP in reporter. */
    .size UsageFault_Handler, .-UsageFault_Handler

    .global l13_abi_call_bits
    .type l13_abi_call_bits, %function
    .thumb_func
l13_abi_call_bits:
    /* Integer public signature on both builds. Adapt explicitly to the
     * selected C float ABI; this does not make arbitrary mixed objects safe. */
    push {r4, lr}
#if defined(L13_FP_hard)
    vmov s0, r0
    vmov s1, r1
#endif
    bl l13_abi_add
#if defined(L13_FP_hard)
    vmov r0, s0
#endif
    pop {r4, pc}
    .size l13_abi_call_bits, .-l13_abi_call_bits
