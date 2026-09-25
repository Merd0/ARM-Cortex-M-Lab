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
 *
 * SP'YE GUVENMEYEN DONUS YOLU:
 *   Test edilen callee SP'yi bozmus olabilir. O durumda [sp] artik
 *   harness'in frame'ini gostermez. Bu yuzden BLX'ten donduktan sonra, SP
 *   dogrulanip bilinen iyi degere geri yuklenene kadar HICBIR stack-goreli
 *   okuma/yazma yapilmaz. Donuste gereken her sey - beklenen SP
 *   (g_harness_sp_at_call) ve out_result (g_harness_out_result) - BLX'ten
 *   ONCE global yuvalara yazilir. Harness tek bagli (single-thread),
 *   egitim amacli bir tani aracidir; global yuvalar bu yuzden yeterlidir.
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
abi_check_callee:                       @ r0=fn r1=a r2=b r3=out_result
    /* 10 register = 40 byte. Girisi 8'e hizali bir SP 8'e hizali kalir.
     * r3 burada yalnizca hizalama DOLGUSU. */
    push    {r3-r11, lr}

    /* out_result'i SP'den BAGIMSIZ bir yuvaya koy (r12: scratch). */
    ldr     r12, =g_harness_out_result
    str     r3, [r12]

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
    str     r3, [r2]            /* BEKLENEN SP = BLX anindaki SP */

    blx     r12                 /* fn(a, b) */

    /* ================================================================
     * DONUS. SP GUVENILMEZ. Asagidaki bloklarda SP DOGRULANANA KADAR
     * hicbir [sp] erisimi, push veya pop YOK. r4-r11'e de dokunulmuyor:
     * onlar test edilen callee'nin kanitidir.
     * ================================================================ */

    /* 1) Sonucu sakla: isaretci global yuvadan, SP'den degil. */
    ldr     r3, =g_harness_out_result
    ldr     r3, [r3]
    str     r0, [r3]            /* *out_result = sonuc */

    movs    r0, #0              /* r0 = maske */

    /* 2) SP'yi dogrula ve bilinen iyi degere geri yukle. */
    ldr     r2, =g_harness_sp_at_call
    ldr     r2, [r2]            /* beklenen SP */
    mov     r1, sp              /* donen SP */
    cmp     r1, r2
    it      ne
    orrne   r0, r0, #0x100      /* SP geri gelmedi */
    mov     sp, r2              /* her durumda bilinen iyi SP (esitse etkisiz) */

    /* 3) SP artik harness'in frame'ini gosteriyor. Nobetcileri denetle. */
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

    /* 4) Cagiranin r4-r11'ini geri ver ve don (SP dogrulandi). */
    pop     {r3-r11, pc}
    .size   abi_check_callee, . - abi_check_callee

    .ltorg

/* ------------------------------------------------------------------ */
/* SP'den bagimsiz donus yuvasi. g_harness_sp_at_call C tarafinda      */
/* tanimli (main.c); bu yuva yalnizca harness'e ait.                   */
/* ------------------------------------------------------------------ */
    .bss
    .align  2
    .global g_harness_out_result
g_harness_out_result:
    .space  4
