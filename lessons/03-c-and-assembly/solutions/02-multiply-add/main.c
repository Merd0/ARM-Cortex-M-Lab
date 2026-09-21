/*
 * SOLUTION 02 firmware - multiply_add testi.
 * Uc argument'in r0/r1/r2'ye gelisini gosterir.
 */
#include "common/console.h"

int multiply_add(int a, int b, int c);

int main(void)
{
    console_line("");
    console_line("===== LESSON 03 SOLUTION 02 =====");

    console_kv_u32("MULTIPLY_ADD(3,4,5)", (uint32_t)multiply_add(3, 4, 5));
    console_kv_u32("MULTIPLY_ADD(7,7,1)", (uint32_t)multiply_add(7, 7, 1));

    console_kv_str("CHECK_3x4p5",
                   (multiply_add(3, 4, 5) == 17) ? "PASS" : "FAIL");

    console_line("===== END =====");
    for (;;) { __asm__ volatile ("wfi"); }
}
