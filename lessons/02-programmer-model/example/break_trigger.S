/*
 * break_invalid_psp - Ders 02 "Break It" tetikleyicisi. DONMEZ.
 *
 *     void break_invalid_psp(void);
 *
 * NIYET
 *   Gecersiz bir stack pointer ile calismanin ne demek oldugunu KONTROLLU
 *   sekilde gostermek.
 *
 * NEDEN ASSEMBLY? (onarim notu)
 *   Ilk surum bunu C icinde yapiyordu: CONTROL.SPSEL'i siradan bir C
 *   fonksiyonunun ortasinda degistirip ayni fonksiyonda devam ediyordu.
 *   Bu gecersizdir (derleyici frame'i MSP'de kalir) ve ustelik hatanin
 *   olustugu an derleyicinin urettigi koda bagliydi. Ayrica PSP'nin
 *   baslangic degerinin 0 olmasina guveniyordu; bu MIMARI BIR GARANTI
 *   DEGILDIR - reset sonrasi PSP degeri belirtilmemistir.
 *
 *   Simdi kosullar ACIKCA kuruluyor:
 *     1. PSP'ye bilerek gecersiz bir deger yaziyoruz (0x00000000).
 *     2. SPSEL = 1 ile onu aktif stack yapiyoruz.
 *     3. TEK bir store yapiyoruz; hata tam olarak o komutta olusur.
 *
 *   0x00000000'dan asagi inen bir PUSH 0xFFFFFFFC'ye yazmaya calisir.
 *   Bu adres, Armv8-M sistem adres alaninin ayrilmis (reserved) kismindadir
 *   ve bu hedefte yazilabilir degildir.
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global break_invalid_psp
    .type   break_invalid_psp, %function
    .thumb_func
break_invalid_psp:
    movs    r0, #0
    msr     psp, r0             /* 1) PSP acikca gecersiz             */

    mrs     r1, control
    orr     r1, r1, #2          /* 2) SPSEL = 1 -> aktif stack PSP    */
    msr     control, r1
    isb

    movs    r2, #0x11
    push    {r2}                /* 3) KONTROLLU gecersiz erisim       */

    b       .                   /* buraya ulasilmamali                */
    .size   break_invalid_psp, . - break_invalid_psp
