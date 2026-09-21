/*
 * Ders 08 - KONTROLLU fault tetikleyicileri.
 *
 * Her biri tek bir komutla hata uretir; hangi komutun hata verdigini
 * disassembly'de tam olarak gorebilelim diye assembly'de yazildi. C'de
 * tanimsiz davranisla fault "uretmek" derleyiciye bagli olurdu.
 */

    .syntax unified
    .thumb
    .text
    .align  2

/* void trigger_udf(void)
 * UDF: mimari olarak TANIMSIZ komut. UsageFault (UNDEFINSTR) beklenir. */
    .global trigger_udf
    .type   trigger_udf, %function
    .thumb_func
trigger_udf:
    udf     #0x42
    bx      lr
    .size   trigger_udf, . - trigger_udf

/* uint32_t trigger_load(const volatile uint32_t *addr)
 * Tek bir okuma. Adres bellek sistemi tarafindan reddedilirse BusFault. */
    .global trigger_load
    .type   trigger_load, %function
    .thumb_func
trigger_load:
    ldr     r0, [r0]
    bx      lr
    .size   trigger_load, . - trigger_load

/* void mpu_store_word(volatile uint32_t *addr, uint32_t value)
 * Tek bir yazma. MPU izin vermezse MemManage (DACCVIOL). */
    .global mpu_store_word
    .type   mpu_store_word, %function
    .thumb_func
mpu_store_word:
    str     r1, [r0]
    bx      lr
    .size   mpu_store_word, . - mpu_store_word
