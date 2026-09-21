/*
 * EXERCISE 01 firmware - sub_numbers testi.
 * Doldurman gereken dosya: sub_numbers.S
 */
#include "common/console.h"

int sub_numbers(int a, int b);

int main(void)
{
    console_line("");
    console_line("===== LESSON 03 EXERCISE 01 =====");

    console_kv_u32("SUB_NUMBERS(12,5)", (uint32_t)sub_numbers(12, 5));
    console_kv_u32("SUB_NUMBERS(5,12)", (uint32_t)sub_numbers(5, 12));

    console_kv_str("CHECK_12_MINUS_5",
                   (sub_numbers(12, 5) == 7) ? "PASS" : "FAIL");
    console_kv_str("CHECK_5_MINUS_12",
                   (sub_numbers(5, 12) == -7) ? "PASS" : "FAIL");

    console_line("===== END =====");
    for (;;) { __asm__ volatile ("wfi"); }
}
