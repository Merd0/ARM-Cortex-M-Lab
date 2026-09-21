/*
 * SVC_Handler sarmalayicisi - ders 05.
 *
 * Neden assembly?
 *   Donanim, handler'a girerken lr'ye EXC_RETURN yazar. Bu deger frame'in
 *   HANGI stack'te oldugunu soyleyen tek kaynaktir (bit2). Siradan bir C
 *   fonksiyonu prolog'unda lr'yi stack'e itip bozabilir; ustelik frame
 *   adresini C'den guvenilir sekilde elde etmenin yolu yoktur.
 *
 * Yaptigi sey minimum:
 *   1. EXC_RETURN bit2'ye bak: frame MSP'de mi PSP'de mi?
 *   2. Frame adresini r0'a koy.
 *   3. EXC_RETURN'u r1'e koy.
 *   4. C dispatcher'a DALLAN (bl degil, b).
 *
 * "b" kullanmak bilincli: lr'ye dokunmuyoruz, dolayisiyla lr hala
 * EXC_RETURN'u tasiyor. svc_dispatch normal bir C fonksiyonu gibi
 * donmeye calistiginda (pop {.., pc}) o deger pc'ye yazilir ve exception
 * return gerceklesir. AAPCS'e gore r0/r1 argumanlar icin serbesttir.
 *
 * DESTEKLENEN VARSAYIMLAR (bu ders icin):
 *   - standart 8 word exception frame (soft-float build; FP baglami yok)
 *   - tek guvenlik durumu (Secure)
 *   - ek durum baglami yok (DCRS = 1)
 *   Genel bir exception cercevesi DEGILDIR. FP veya TrustZone baglamlari
 *   ders 13 ve 16'ya aittir.
 */

    .syntax unified
    .thumb
    .text
    .align  2

    .global SVC_Handler
    .type   SVC_Handler, %function
    .thumb_func
SVC_Handler:
    tst     lr, #4              /* EXC_RETURN bit2: 0 = MSP, 1 = PSP */
    ite     eq
    mrseq   r0, msp
    mrsne   r0, psp             /* r0 = exception frame adresi        */
    mov     r1, lr              /* r1 = EXC_RETURN                    */
    b       svc_dispatch        /* lr KORUNUR -> donusu o yapar       */
    .size   SVC_Handler, . - SVC_Handler
