/*
 * Ders 09 - callee-saved register harness'i (bir OLCUM ARACI).
 *
 *   uint32_t abi_check_callee(abi_fn2_t fn, uint32_t a, uint32_t b,
 *                             uint32_t *out_result);
 *
 * Yaptigi is:
 *   1. r4-r11'e birbirinden farkli "nobetci" (sentinel) degerler yukler.
 *   2. fn(a, b)'yi cagirir.
 *   3. Donuste r4-r11 ve SP'yi kontrol eder, bozulanlari bir MASKEDE toplar.
 *
 * Neden assembly?  C'de "r4'te bir deger tutup cagridan sonra bak" yazmak
 * GCC'nin register tahsisine bagli kalirdi. Burada nobetciler KESIN olarak
 * r4-r11'dedir.
 *
 * Harness'in KENDISI AAPCS'e uymak zorunda (bir olcum aleti hatali
 * olursa olcum de hatali olur):
 *   - kendi cagiraninin r4-r11'ini saklar ve geri yukler
 *   - LR'yi saklar (yaprak degil)
 *   - BLX aninda SP 8 byte hizalidir
 *   - sonucu r0'da (maske) ve *out_result'ta dondurur
 *
 * Maske:  bit0..bit7 = r4..r11 bozuldu,  bit8 = SP geri gelmedi.
 * Not: r9, AAPCS'te platforma ozel bir register'dir. Bu repo/toolchain
 * yapilandirmasinda callee-saved (v6) olarak davrandigi OLCULDU; harness
 * onu da denetler (README, M6).
 */
    .syntax unified
    .thumb
    .text

    .global abi_check_callee
    .type   abi_check_callee, %function
    .thumb_func
abi_check_callee:
    /* 10 register = 40 byte. Girisi 8'e hizali bir SP 8'e hizali kalir.
     * r3 iki is gorur: hizalama dolgusu VE out_result'in saklandigi yuva. */
    push    {r3-r11, lr}

    mov     r12, r0             /* cagri hedefi -> IP (scratch)           */
    mov     r0, r1              /* a                                       */
    mov     r1, r2              /* b                                       */

    ldr     r4,  =0xA4A4A404
    ldr     r5,  =0xA5A5A505
    ldr     r6,  =0xA6A6A606
    ldr     r7,  =0xA7A7A707
    ldr     r8,  =0xA8A8A808
    ldr     r9,  =0xA9A9A909
    ldr     r10, =0xAAAAAA0A
    ldr     r11, =0xABABAB0B

    ldr     r2, =g_harness_sp_at_call
    mov     r3, sp
    str     r3, [r2]            /* BLX aninda SP */

    blx     r12                 /* fn(a, b) */

    /* --- donus: r0 = sonuc; r1-r3, r12 serbest --- */
    ldr     r3, [sp, #0]        /* dolgu yuvasindan out_result */
    str     r0, [r3]            /* *out_result = sonuc */

    movs    r0, #0              /* r0 = maske */

    ldr     r1, =0xA4A4A404
    cmp     r4, r1
    it      ne
    orrne   r0, r0, #0x001
    ldr     r1, =0xA5A5A505
    cmp     r5, r1
    it      ne
    orrne   r0, r0, #0x002
    ldr     r1, =0xA6A6A606
    cmp     r6, r1
    it      ne
    orrne   r0, r0, #0x004
    ldr     r1, =0xA7A7A707
    cmp     r7, r1
    it      ne
    orrne   r0, r0, #0x008
    ldr     r1, =0xA8A8A808
    cmp     r8, r1
    it      ne
    orrne   r0, r0, #0x010
    ldr     r1, =0xA9A9A909
    cmp     r9, r1
    it      ne
    orrne   r0, r0, #0x020
    ldr     r1, =0xAAAAAA0A
    cmp     r10, r1
    it      ne
    orrne   r0, r0, #0x040
    ldr     r1, =0xABABAB0B
    cmp     r11, r1
    it      ne
    orrne   r0, r0, #0x080

    ldr     r2, =g_harness_sp_at_call
    ldr     r2, [r2]
    mov     r1, sp
    cmp     r1, r2
    it      ne
    orrne   r0, r0, #0x100      /* SP geri gelmedi */

    pop     {r3-r11, pc}        /* cagiranin r4-r11'i geri; r3 scratch */
    .size   abi_check_callee, . - abi_check_callee

    .ltorg
