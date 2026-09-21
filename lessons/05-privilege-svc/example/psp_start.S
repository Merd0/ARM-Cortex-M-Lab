/*
 * psp_start - MSP uzerindeki C baglamindan PSP uzerindeki YENI bir C
 *             baglamina gecis.
 *
 *     void psp_start(uint32_t psp_top, void (*entry)(void));   -- DONMEZ
 *
 * NEDEN ASSEMBLY?
 * ---------------
 * CONTROL.SPSEL'i siradan bir C fonksiyonunun ORTASINDA degistirmek
 * gecerli degildir. Derleyici o fonksiyon icin MSP uzerinde bir frame
 * ayirmistir ve yerel degiskenlere [sp, #offset] ile erisen komutlar
 * uretmistir. SPSEL degisince SP baska bir bolgeyi gosterir; ayni
 * offset'ler artik BASKA bir adrese denk gelir. CONTROL yazmak ne
 * mevcut frame'i tasir ne de yenisini olusturur.
 *
 * Dogru sinir: stack'i degistiren kod, derleyicinin frame varsayimi
 * olmayan bir yerde olmali ve ardindan SIFIRDAN bir C fonksiyonuna
 * dallanmali. O fonksiyon kendi prolog'unu yeni SP uzerinde kurar.
 *
 * AAPCS: r0 = psp_top, r1 = entry.
 * psp_top 8 byte hizali olmali (cagiran sorumlulugunda).
 *
 * Bu fonksiyon DONMEZ. MSP uzerinde birakilan frame'e bir daha erisilmez.
 * Guvenlik agi olarak lr, entry yine de donerse sonsuz donguye giden bir
 * tuzaga ayarlanir.
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global psp_start
    .type   psp_start, %function
    .thumb_func
psp_start:
    msr     psp, r0             /* 1) PSP = yeni stack tepesi            */

    mrs     r2, control
    orr     r2, r2, #2          /* 2) SPSEL = 1 -> Thread Mode artik PSP */
    msr     control, r2
    isb                         /* 3) baglam degisikligini senkronize et */

    ldr     lr, =psp_entry_trap /* entry donerse gidecegi yer            */
    orr     lr, lr, #1          /* Thumb biti                            */
    bx      r1                  /* yeni C baglamina dallan (donmez)      */
    .size   psp_start, . - psp_start

    .type   psp_entry_trap, %function
    .thumb_func
psp_entry_trap:
    b       .                   /* entry donerse burada kal              */
    .size   psp_entry_trap, . - psp_entry_trap
