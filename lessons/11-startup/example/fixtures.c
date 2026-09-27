/*
 * Ders 11 - startup'in dokunmasi gereken iki nesne (+ faz kaydi).
 * -fdata-sections sayesinde her biri kendi input section'inda:
 *   .data.g_l11_init, .bss.g_l11_zero, .bss.g_l11_phase
 */
#include "fixtures.h"

uint32_t g_l11_init = L11_INIT_VALUE;   /* .data */
uint32_t g_l11_zero;                    /* .bss  */
uint32_t g_l11_phase;                   /* .bss  */
