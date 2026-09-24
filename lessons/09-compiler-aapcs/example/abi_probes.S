/*
 * Ders 09 - arguman ve donus degeri probe'lari.
 *
 * Her probe, GIRISTEKI durumu, register'lara veya stack'e dokunan HICBIR
 * isten once kaydeder. Isaretci olarak yalnizca r12 (IP) kullanilir:
 * AAPCS'e gore r12 bir cagri boyunca korunmak zorunda olmayan (scratch)
 * register'dir ve arguman tasimaz.
 *
 * Probe'lar YAPRAK (leaf) fonksiyonlardir: hicbir sey cagirmaz, stack'e
 * yazmaz. [SP] bu yuzden hala cagiranin yigdigi ilk stack argumanini
 * gosterir.
 */
    .syntax unified
    .thumb
    .text

/* ---------------------------------------------------------------- */
/* Ortak yakalama makrosu: r0-r3, SP, [SP], [SP+4]                   */
/* r0 kaydedildikten SONRA scratch olarak kullanilir.                */
/* ---------------------------------------------------------------- */
.macro CAPTURE_ENTRY
    ldr     r12, =g_probe
    str     r0, [r12, #0]
    str     r1, [r12, #4]
    str     r2, [r12, #8]
    str     r3, [r12, #12]
    mov     r0, sp
    str     r0, [r12, #16]
    ldr     r0, [sp, #0]
    str     r0, [r12, #20]
    ldr     r0, [sp, #4]
    str     r0, [r12, #24]
.endm

    .global probe_4u32
    .type   probe_4u32, %function
    .thumb_func
probe_4u32:
    CAPTURE_ENTRY
    bx      lr
    .size   probe_4u32, . - probe_4u32

    .global probe_5u32
    .type   probe_5u32, %function
    .thumb_func
probe_5u32:
    CAPTURE_ENTRY
    bx      lr
    .size   probe_5u32, . - probe_5u32

    .global probe_u32_u64
    .type   probe_u32_u64, %function
    .thumb_func
probe_u32_u64:
    CAPTURE_ENTRY
    bx      lr
    .size   probe_u32_u64, . - probe_u32_u64

    .global probe_3u32_u64
    .type   probe_3u32_u64, %function
    .thumb_func
probe_3u32_u64:
    CAPTURE_ENTRY
    bx      lr
    .size   probe_3u32_u64, . - probe_3u32_u64

/* ---------------------------------------------------------------- */
/* uint64_t probe_call_u64ret(void)                                  */
/* C'deki make_u64()'u cagirir ve BL'DEN HEMEN SONRA r0/r1'i kaydeder.*/
/* r0:r1 dokunulmadan geri dondurulur.                               */
/* Yaprak DEGIL: LR'yi saklamak zorunda.                             */
/* ---------------------------------------------------------------- */
    .global probe_call_u64ret
    .type   probe_call_u64ret, %function
    .thumb_func
probe_call_u64ret:
    push    {r4, lr}            /* 8 byte: SP 8'e hizali kalir */
    bl      make_u64
    ldr     r12, =g_probe
    str     r0, [r12, #28]
    str     r1, [r12, #32]
    pop     {r4, pc}
    .size   probe_call_u64ret, . - probe_call_u64ret

/* ---------------------------------------------------------------- */
/* uint32_t abi_chain(uint32_t x)   ->   helper_scale(x) + x         */
/*                                                                   */
/* C main  ->  abi_chain (asm)  ->  helper_scale (C)                 */
/* x'i cagri boyunca r4'te (callee-saved) tutar; bu yuzden r4'u      */
/* kendi cagirani icin saklar. BL'den HEMEN ONCE SP'yi kaydeder.     */
/* ---------------------------------------------------------------- */
    .global abi_chain
    .type   abi_chain, %function
    .thumb_func
abi_chain:
    push    {r4, lr}            /* r4: bizim kullanacagimiz callee-saved */
                                /* lr: yaprak degiliz, BL onu ezecek     */
    mov     r4, r0
    ldr     r1, =g_chain_sp_at_call
    mov     r2, sp
    str     r2, [r1]            /* BL aninda SP */
    mov     r0, r4
    bl      helper_scale
    add     r0, r0, r4          /* r4 hala x: helper AAPCS'e uydu */
    pop     {r4, pc}
    .size   abi_chain, . - abi_chain
