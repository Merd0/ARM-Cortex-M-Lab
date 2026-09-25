/*
 * Ders 09 - OLCUM ALETI REGRESYONU: SP bozan callee'ler.
 *
 * Bu dosya ogrenciye yonelik bir ornek DEGILDIR. abi_check_callee'nin
 * (example/abi_harness.S) SP'yi bozan bir callee'den SAG CIKIP bunu
 * raporlayabildigini kanitlar.
 *
 * Bagimsiz denetim eski harness'te sunu buldu: callee SP'yi +4 kaydirinca
 * harness [sp]'den "out_result" okumaya calisti, oraya yigilmis olan
 * cagiranin r4'unu (0xDEAD0000) isaretci sandi ve oraya yazdi -> BusFault.
 * Buradaki sarmalayici tam o durumu yeniden kurar: harness'i cagirirken
 * r4-r11'de TANINABILIR degerler (0xDEAD0000..0xDEAD0007) tutar.
 */
    .syntax unified
    .thumb
    .text

/* uint32_t sp_up_callee(a, b): a+b, ama SP +4 kaymis donus */
    .global sp_up_callee
    .type   sp_up_callee, %function
    .thumb_func
sp_up_callee:
    adds    r0, r0, r1
    add     sp, sp, #4          /* BILEREK: SP geri gelmiyor */
    bx      lr
    .size   sp_up_callee, . - sp_up_callee

/* uint32_t sp_down_callee(a, b): a+b, ama SP -8 kaymis donus */
    .global sp_down_callee
    .type   sp_down_callee, %function
    .thumb_func
sp_down_callee:
    adds    r0, r0, r1
    sub     sp, sp, #8          /* BILEREK: SP geri gelmiyor */
    bx      lr
    .size   sp_down_callee, . - sp_down_callee

/* uint32_t sp_up_r4_callee(a, b): a+b, r4 bozuk VE SP +4 kaymis */
    .global sp_up_r4_callee
    .type   sp_up_r4_callee, %function
    .thumb_func
sp_up_r4_callee:
    movs    r4, #0
    adds    r0, r0, r1
    add     sp, sp, #4
    bx      lr
    .size   sp_up_r4_callee, . - sp_up_r4_callee

/* ---------------------------------------------------------------- */
/* uint32_t sp_regression_run(abi_fn2_t fn)                          */
/*                                                                   */
/* Harness'i, r4-r11'de 0xDEAD0000..0xDEAD0007 tutarak cagirir.      */
/* Kendi cagirani icin AAPCS'e uyar: r4-r11 ve LR saklanir, 40 byte  */
/* yigilir (SP 8'e hizali kalir).                                    */
/* Donuste: g_reg_sp_before / g_reg_sp_after (harness cagrisinin     */
/* hemen oncesi ve sonrasi SP) ve g_reg_after[8] (harness donunce    */
/* r4-r11) doldurulur. Donus degeri: harness maskesi.                */
/* ---------------------------------------------------------------- */
    .global sp_regression_run
    .type   sp_regression_run, %function
    .thumb_func
sp_regression_run:
    push    {r3-r11, lr}

    ldr     r4,  =0xDEAD0000
    ldr     r5,  =0xDEAD0001
    ldr     r6,  =0xDEAD0002
    ldr     r7,  =0xDEAD0003
    ldr     r8,  =0xDEAD0004
    ldr     r9,  =0xDEAD0005
    ldr     r10, =0xDEAD0006
    ldr     r11, =0xDEAD0007

    ldr     r1, =g_reg_sp_before
    mov     r2, sp
    str     r2, [r1]

    /* r0 = fn (zaten) */
    movs    r1, #7              /* a */
    movs    r2, #5              /* b */
    ldr     r3, =g_reg_result   /* out_result */
    bl      abi_check_callee    /* r0 = maske */

    ldr     r1, =g_reg_sp_after
    mov     r2, sp
    str     r2, [r1]

    ldr     r1, =g_reg_after
    stmia   r1, {r4-r11}        /* harness donunce cagiranin r4-r11'i */

    pop     {r3-r11, pc}
    .size   sp_regression_run, . - sp_regression_run

    .ltorg
