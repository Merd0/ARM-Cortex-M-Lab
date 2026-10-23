#include "measurement.h"
#include "platform/mps2_an505.h"
#include "common/console.h"

int main(void)
{
    static const struct {
        const char *name;
        struct l12_sample s;
        enum l12_kind expected;
    } cases[] = {
        {"FORWARD", {1U, 1U<<24, 10U, 42U, 0U}, L12_COUNTER_DELTA},
        {"WRAP", {1U, 1U<<24, 0xFFFFFFF0U, 0x10U, 0U}, L12_COUNTER_DELTA},
        {"STOPPED_ZERO", {1U, 1U<<24, 0U, 0U, 0U}, L12_UNAVAILABLE},
        {"STOPPED_NONZERO", {1U, 1U<<24, 42U, 42U, 0U}, L12_UNAVAILABLE},
        {"NO_TRACE", {1U, 0U, 1U, 2U, 0U}, L12_UNAVAILABLE},
        {"DISABLED", {0U, 1U<<24, 1U, 2U, 0U}, L12_UNAVAILABLE},
        {"NO_COUNTER", {(1U<<25)|1U, 1U<<24, 1U, 2U, 0U}, L12_UNAVAILABLE},
        {"SECURE_DISABLED", {(1U<<23)|1U, 1U<<24, 1U, 2U, 0U}, L12_UNAVAILABLE},
        {"IO", {1U, 1U<<24, 1U, 2U, 1U}, L12_CONTAMINATED},
        {"IO_AND_STOPPED", {1U, 1U<<24, 0U, 0U, 1U}, L12_CONTAMINATED},
    };
    console_line("===== LESSON 12: SYNTHETIC MEASUREMENT CASES =====");
    uint32_t passed = 0U;
    for (uint32_t i = 0U; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        uint32_t good = l12_classify(&cases[i].s) == cases[i].expected;
        console_kv_str(cases[i].name, good ? "PASS" : "FAIL");
        passed += good;
    }
    uint32_t forward = l12_delta(10U, 42U) == 32U;
    uint32_t wrap = l12_delta(0xFFFFFFF0U, 0x10U) == 32U;
    console_kv_str("DELTA_FORWARD", forward ? "PASS" : "FAIL");
    console_kv_str("DELTA_WRAP", wrap ? "PASS" : "FAIL");
    console_kv_u32("CASES_PASSED", passed + forward + wrap);
    console_line("===== END =====");
    for (;;) __WFI();
}
