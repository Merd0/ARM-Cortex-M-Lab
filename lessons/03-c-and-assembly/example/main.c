/*
 * Lesson 03 - C and Assembly
 *
 * Bu firmware tek bir soruyu gorunur kiliyor:
 * bir C fonksiyon cagrisi, islemci seviyesinde tam olarak nedir?
 *
 * Iki adet toplama fonksiyonu var:
 *   add_numbers()    -> elle yazilmis assembly (example/add_numbers.S)
 *   add_numbers_c()  -> ayni isi yapan C fonksiyonu (derleyici uretiyor)
 * Ikisi de ayni ABI kurallarina uymak ZORUNDA, cunku ikisini de ayni C
 * kodu cagiriyor.
 *
 * accumulate() ise bir fonksiyon cagrisi BOYUNCA deger saklamayi gosterir;
 * "Break It" hedefi tam olarak orayi bozar.
 *
 * Build/run icin: lessons/03-c-and-assembly/README.md
 */

#include "common/console.h"

/* Assembly'de tanimli. Derleyici govdeyi gormez; yalnizca imzaya guvenir.
 * Imza yanlissa derleyici uyarmaz - hata calisma zamaninda ortaya cikar. */
int add_numbers(int a, int b);
int accumulate(int a, int b);

/* Ayni is, bu kez C ile. Disassembly'de ikisini karsilastiracagiz. */
int add_numbers_c(int a, int b)
{
    return a + b;
}

int main(void)
{
    console_line("");
    console_line("===== LESSON 03: C AND ASSEMBLY =====");

    /* --- 1) Elle yazilmis assembly fonksiyonu --------------------- */
    int asm_result = add_numbers(7, 5);
    console_kv_u32("ADD_NUMBERS_ASM(7,5)", (uint32_t)asm_result);

    /* --- 2) Ayni isi yapan C fonksiyonu -------------------------- */
    int c_result = add_numbers_c(7, 5);
    console_kv_u32("ADD_NUMBERS_C(7,5)", (uint32_t)c_result);

    console_kv_str("SAME_RESULT", (asm_result == c_result) ? "YES" : "NO");

    /* --- 3) Cagri boyunca deger saklamak ------------------------- */
    /* accumulate(), 'a' degerini helper() cagrisi boyunca r4'te tutar. */
    console_line("--- callee-saved ---");
    console_kv_u32("ACCUMULATE(7,5)", (uint32_t)accumulate(7, 5));

    console_line("===== END =====");

    for (;;) {
        __asm__ volatile ("wfi");
    }
}
