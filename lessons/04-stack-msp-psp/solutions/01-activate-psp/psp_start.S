/*
 * SOLUTION 01 - psp_start
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
 * Referans cozum. Exercise: exercises/01-activate-psp/
 *
 * Uc adim, uc sebep:
 *   msr psp, r0   -> PSP'nin ICERIGI. Tek basina hicbir sey secmez.
 *   CONTROL bit 1 -> SPSEL. Asil secimi bu yapar. Oku-degistir-yaz
 *                    kullaniyoruz ki bit 0 (nPRIV) ezilmesin.
 *   isb           -> CONTROL yazimi yurutme baglamini degistirir; mimari,
 *                    sonraki komutlarin yeni baglami gormesinin garanti
 *                    olmasi icin bir komut senkronizasyon bariyeri ister.
 *                    (CMSIS'in __set_CONTROL sarmalayicisi bu ISB'yi kendi
 *                    icinde yapar; burada MSR'yi elle yazdigimiz icin ISB
 *                    de bize ait.)
 *
 * Sira zorunludur: once gecerli bir adres, sonra gecis. Ters sirada
 * SPSEL=1 oldugu anda SP tanimsiz/gecersiz bir PSP'yi gosterir.
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global psp_start
    .type   psp_start, %function
    .thumb_func
psp_start:
    msr     psp, r0                 /* 1) PSP = yeni stack tepesi   */

    mrs     r2, control
    orr     r2, r2, #2              /* 2) SPSEL = 1                 */
    msr     control, r2
    isb                             /* 3) senkronize et             */

    ldr     lr, =psp_entry_trap     /* entry donerse gidecegi yer */
    orr     lr, lr, #1              /* Thumb biti */
    bx      r1
    .size   psp_start, . - psp_start

    .type   psp_entry_trap, %function
    .thumb_func
psp_entry_trap:
    b       .
    .size   psp_entry_trap, . - psp_entry_trap
