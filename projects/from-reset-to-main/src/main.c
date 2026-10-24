#include "platform/mps2_an505.h"
#include "common/console.h"
#include "fixtures.h"
extern void Reset_Handler(void);
extern uint32_t __data_load_start;
volatile uint32_t project_calibration[3] = {11U, 22U, 33U};
volatile uint32_t project_buffer[12];
static void check(const char *name, uint32_t ok) { console_kv_str(name, ok ? "PASS" : "FAIL"); }
int main(void)
{
    console_line("===== PROJECT: FROM RESET TO MAIN =====");
    const uint32_t *v = __vectors_start;
    uint32_t zero = 1U;
    for (uint32_t i = 0; i < 12U; ++i) if (project_buffer[i]) zero = 0U;
    check("CHECK_DATA", project_calibration[0] == 11U && project_calibration[2] == 33U &&
          g_l11_init == L11_INIT_VALUE);
    check("CHECK_BSS", zero && g_l11_zero == 0U);
    check("CHECK_ORDER", g_l11_phase == L11_PHASE_OK);
    check("CHECK_VECTORS", v[0] == (uint32_t)(uintptr_t)&__stack_top &&
          v[1] == ((uint32_t)(uintptr_t)&Reset_Handler | 1U) &&
          v[8] == 0U && v[9] == 0U && v[10] == 0U && v[13] == 0U);
    check("CHECK_VTOR", SCB->VTOR == (uint32_t)(uintptr_t)v);
    check("CHECK_LMA", (uintptr_t)&__data_load_start >= 0x10000000U &&
          (uintptr_t)&__data_load_start < 0x10080000U &&
          (uintptr_t)&project_calibration >= 0x30000000U);
    console_kv_hex("LMA", (uint32_t)(uintptr_t)&__data_load_start);
    console_kv_hex("DATA", (uint32_t)(uintptr_t)project_calibration);
    console_line("===== END =====");
    return 0;
}
