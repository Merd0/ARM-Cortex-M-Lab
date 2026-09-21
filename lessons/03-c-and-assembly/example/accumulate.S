/*
 * accumulate / helper - callee-saved register kuralini gosteren ikili.
 *
 *     int accumulate(int a, int b);   ->  a + b
 *
 * accumulate, HEM 'a' HEM 'b' degerini bir fonksiyon cagrisi BOYUNCA
 * saklamak zorunda. AAPCS'e gore cagri boyunca yasamasi gereken degerler
 * korunan register'lara konur: bir altyordam r4-r8, r10, r11 ve SP'yi
 * teslim aldigi gibi geri vermek zorundadir (r9'un rolu platforma ozeldir).
 * r0-r3 ise cagrilan fonksiyon tarafindan serbestce bozulabilir.
 * (Ayrintili kural ders 09'da; burada sadece fikri goruyoruz.)
 *
 * ONARIM NOTU: ilk surum yalnizca 'a'yi r4'e alip 'b'yi r1'de birakiyordu.
 * Mevcut helper() r1'e dokunmadigi icin bu CALISIYORDU - ama gizli bir
 * yerel sozlesmeydi: helper'a r1'i bozan tek bir satir eklendiginde
 * sessizce yanlis sonuc verirdi. Simdi iki deger de korunan register'larda.
 *
 * helper() bilerek bos birakildi. LESSON03_BREAK_CALLEE_SAVED tanimliysa
 * kurali ihlal eder: r4'u bozar ve geri yuklemez.
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global accumulate
    .type   accumulate, %function
    .thumb_func
accumulate:
    /* Dort register itiyoruz: r4, r5, r6, lr.
       r6'ya aslinda ihtiyacimiz yok; yalnizca itilen bayt sayisini 16'ya
       tamamlamak icin var. 3 register itseydik SP 12 byte inerdi ve
       'bl helper' cagri sinirinda SP 8 byte hizali OLMAZDI. */
    push    {r4, r5, r6, lr}
    mov     r4, r0               /* a: cagri boyunca korunan register'da */
    mov     r5, r1               /* b: cagri boyunca korunan register'da */
    bl      helper               /* helper lr'yi ezer - bu yuzden ittik   */
    adds    r0, r4, r5           /* r0 = a + b   (r4 hala 'a' mi?)        */
    pop     {r4, r5, r6, pc}     /* geri ver ve don                       */
    .size   accumulate, . - accumulate

    .type   helper, %function
    .thumb_func
helper:
#ifdef LESSON03_BREAK_CALLEE_SAVED
    movs    r4, #0               /* HATA: r4 callee-saved; bozuldu, geri yuklenmedi */
#endif
    bx      lr
    .size   helper, . - helper
