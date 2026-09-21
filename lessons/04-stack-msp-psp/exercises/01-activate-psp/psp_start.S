/*
 * EXERCISE 01 - psp_start
 *
 *     void psp_start(uint32_t psp_top, void (*entry)(void));   -- DONMEZ
 *
 * AAPCS: r0 = psp_top, r1 = entry.
 *
 * NEDEN BU KOD ASSEMBLY?
 *   CONTROL.SPSEL'i siradan bir C fonksiyonunun ortasinda degistirmek
 *   GECERSIZDIR. Derleyici o fonksiyon icin MSP uzerinde bir frame
 *   ayirmistir ve yerel degiskenlere [sp, #offset] ile erisir. SPSEL
 *   degisince ayni offset'ler baska bir adrese denk gelir; CONTROL
 *   yazmak ne mevcut frame'i tasir ne de yenisini olusturur.
 *   Bu yuzden gecis, derleyicinin frame varsayimi olmayan bir yerde
 *   yapilir ve ardindan SIFIRDAN bir C fonksiyonuna dallanilir.
 *
 * GOREV
 *   Asagiya uc adimi ekle:
 *
 *     1. PSP'ye r0'daki adresi yaz.
 *        Hangi komut ozel register'a yazar? (Ipucu: ders 02'de gordun.)
 *
 *     2. Thread Mode'u PSP kullanacak sekilde ayarla.
 *        CONTROL'un hangi biti? Diger bitleri EZME - oku, degistir, yaz.
 *
 *     3. Baglam degisikligini senkronize et.
 *
 *   Su an hicbiri yok: kod dogrudan entry'ye dallaniyor, dolayisiyla
 *   entry hala MSP uzerinde calisiyor ve CHECK'ler FAIL veriyor.
 *   Bu beklenen baslangic durumudur, bir bug degildir.
 *
 * Referans cozum: solutions/01-activate-psp/psp_start.S
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global psp_start
    .type   psp_start, %function
    .thumb_func
psp_start:
    /* TODO(student): 1) msr psp  2) CONTROL.SPSEL = 1  3) isb */

    ldr     lr, =psp_entry_trap     /* entry donerse gidecegi yer */
    orr     lr, lr, #1              /* Thumb biti */
    bx      r1
    .size   psp_start, . - psp_start

    .type   psp_entry_trap, %function
    .thumb_func
psp_entry_trap:
    b       .
    .size   psp_entry_trap, . - psp_entry_trap
