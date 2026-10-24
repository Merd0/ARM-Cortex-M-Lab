.syntax unified
.cpu cortex-m33
.fpu fpv5-sp-d16
.thumb
.text
.global node_calibrate
.type node_calibrate,%function
.thumb_func
node_calibrate:
    /* Integer ABI. S0/S1 and R1-R3 are caller-clobbered; S16-S31 untouched.
     * Two packed signed halfword products form 3*x; x is bounded [0,15]. */
    orr r1,r0,r0,lsl #16
    movw r2,#1
    movt r2,#2
    movs r3,#0
    smlad r0,r1,r2,r3
    vmrs r2,fpscr
    vmov s0,r0
    vcvt.f32.u32 s0,s0
    vmov.f32 s1,#0.5
    vmul.f32 s0,s0,s1
    vcvt.u32.f32 s0,s0
    vmov r0,s0
    movs r3,#0
    vmov s0,r3
    vmov s1,r3
    vmsr fpscr,r2
    bx lr
.size node_calibrate,.-node_calibrate
.section .note.GNU-stack,"",%progbits
