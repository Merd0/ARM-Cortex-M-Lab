/*
 * Ders 11 - ozel startup hedeflerinin main'i
 * (11-startup-broken, 11-exercise-startup, 11-solution-startup).
 *
 * Bu dosyayi DEGISTIRME. Kontroller ders-yerel Reset_Handler'in
 * sonucunu olcer; sabit adreslere dayanmaz.
 */
#include "platform/mps2_an505.h"
#include "common/console.h"
#include "fixtures.h"

static const char *pf(int ok) { return ok ? "PASS" : "FAIL"; }
static uint32_t A(const void *p) { return (uint32_t)(uintptr_t)p; }

#ifndef L11_BANNER
#define L11_BANNER "LESSON 11: CUSTOM STARTUP"
#endif

int main(void)
{
    const uint32_t *vec = __vectors_start;

    console_line("");
    console_line("===== " L11_BANNER " =====");
    console_kv_hex("  g_l11_init", g_l11_init);
    console_kv_hex("  g_l11_zero", g_l11_zero);
    console_kv_hex("  g_l11_phase", g_l11_phase);
    console_kv_hex("  vector[0]", vec[0]);

    console_kv_str("  CHECK_DATA_INIT", pf(g_l11_init == L11_INIT_VALUE));
    console_kv_str("  CHECK_BSS_ZERO", pf(g_l11_zero == 0u));
    console_kv_str("  CHECK_PHASE_ORDER", pf(g_l11_phase == L11_PHASE_OK));
    console_kv_str("  CHECK_VECTOR0_EQ_STACK_TOP", pf(vec[0] == A(&__stack_top)));

    console_line("===== END =====");
    return 0;
}
