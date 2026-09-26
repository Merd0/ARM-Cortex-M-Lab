/*
 * Ders 10 - BREAK IT: l10_asm.S ile ayni, TEK fark: ".global asm_fn" yok.
 *
 *   uint32_t asm_fn(uint32_t x)  ->  text_fn(local_helper(x)) = 3*(2x)+1
 *
 * asm_fn      .global  -> baska nesne dosyalari onu gorebilir
 * local_helper (no .global) -> LOCAL; yalnizca bu dosyanin icinden gorunur
 * asm_word    .global, .data icinde
 *
 * Bu dosya -ffunction-sections ile DERLENMEZ (o bir C derleyici bayragi):
 * kod tek bir ".text" input section'inda, veri tek bir ".data"da durur.
 */
    .syntax unified
    .thumb

    .text
    /* BREAK IT: ".global asm_fn" BILEREK SILINDI -> asm_fn LOCAL kalir */
    .type   asm_fn, %function
    .thumb_func
asm_fn:
    push    {r3, lr}            /* 8 byte: SP 8'e hizali kalir */
    bl      local_helper        /* ayni section'daki LOCAL sembol */
    pop     {r3, lr}
    b.w     text_fn             /* baska TU'daki C fonksiyonuna kuyruk dallanmasi */
    .size   asm_fn, . - asm_fn

    .type   local_helper, %function
    .thumb_func
local_helper:
    lsls    r0, r0, #1
    bx      lr
    .size   local_helper, . - local_helper

    .data
    .align  2
    .global asm_word
    .type   asm_word, %object
asm_word:
    .word   0x5A5A5A5A
    .size   asm_word, . - asm_word
