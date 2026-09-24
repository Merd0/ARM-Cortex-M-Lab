/*
 * Lesson 09 - Compiler, AAPCS & Calling Conventions
 *
 * Ders 03 "calisiyor" dedi. Bu ders SOZLESMEYI OLCUYOR:
 *
 *   1. Argumanlar gercekten nereye kondu?          (probe'lar)
 *   2. 64-bit sonuc nereden dondu?                 (r0:r1)
 *   3. Cagri sinirinda SP 8'e hizali mi?           (SP-at-call)
 *   4. Callee, callee-saved register'lari korudu mu? (harness)
 *
 * Probe'lar ve harness EL YAZIMI assembly; argumanlari yerlestiren ise
 * DERLEYICI (GCC). Yani burada olculen sey, derleyicinin cagiran tarafta
 * AAPCS'i nasil uyguladigidir.
 *
 * L09_BROKEN tanimliysa harness'te bilerek bozuk bir callee test edilir.
 *
 * Build/run: lessons/09-compiler-aapcs/README.md
 */

#include "platform/mps2_an505.h"
#include "common/console.h"
#include "abi.h"

volatile probe_capture_t g_probe;
volatile uint32_t g_chain_sp_at_call;
volatile uint32_t g_harness_sp_at_call;

static const char *pf(int ok) { return ok ? "PASS" : "FAIL"; }

/* Maskedeki register adlarini yazar: "r4 r5 SP" gibi. */
static void print_mask_regs(const char *key, uint32_t mask)
{
    static const char *names[9] = {
        "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "SP"
    };
    console_puts("[");
    console_puts(key);
    console_puts("]");
    if (mask == 0u) {
        console_puts(" (yok)");
    }
    for (unsigned i = 0u; i < 9u; i++) {
        if ((mask & (1u << i)) != 0u) {
            console_puts(" ");
            console_puts(names[i]);
        }
    }
    console_line("");
}

/* ------------------------------------------------------------------ */
static void fixture_args(void)
{
    console_line("[1] Arguman yerlesimi (cagiran: GCC, olcen: asm probe)");

    /* F1: 4 x u32 */
    probe_4u32(0xA1A1A1A1u, 0xB2B2B2B2u, 0xC3C3C3C3u, 0xD4D4D4D4u);
    console_kv_hex("  F1 r0", g_probe.r0);
    console_kv_hex("  F1 r1", g_probe.r1);
    console_kv_hex("  F1 r2", g_probe.r2);
    console_kv_hex("  F1 r3", g_probe.r3);
    console_kv_str("  CHECK_ARGS_4U32",
        pf(g_probe.r0 == 0xA1A1A1A1u && g_probe.r1 == 0xB2B2B2B2u &&
           g_probe.r2 == 0xC3C3C3C3u && g_probe.r3 == 0xD4D4D4D4u));

    /* F2: 5 x u32 -> besincisi stack'te */
    probe_5u32(0xA1A1A1A1u, 0xB2B2B2B2u, 0xC3C3C3C3u, 0xD4D4D4D4u,
               0xE5E5E5E5u);
    console_kv_hex("  F2 r3", g_probe.r3);
    console_kv_hex("  F2 [SP+0]", g_probe.stk0);
    console_kv_str("  CHECK_ARG5_ON_STACK",
        pf(g_probe.r0 == 0xA1A1A1A1u && g_probe.r3 == 0xD4D4D4D4u &&
           g_probe.stk0 == 0xE5E5E5E5u));

    /* F3: u32 + u64 -> u64 cift numarali register cifti r2:r3.
     * r1 bu argumani TASIMAZ; icerigi sozlesmenin parcasi degildir.
     * Yalnizca ham gozlem olarak basilir, PASS/FAIL'e girmez. */
    probe_u32_u64(0xA1A1A1A1u, 0x1122334455667788ull);
    console_kv_hex("  F3 r0 (a)", g_probe.r0);
    console_kv_hex("  F3 r1 (unspecified)", g_probe.r1);
    console_kv_hex("  F3 r2 (b.lo)", g_probe.r2);
    console_kv_hex("  F3 r3 (b.hi)", g_probe.r3);
    console_kv_str("  CHECK_U64_IN_R2_R3",
        pf(g_probe.r0 == 0xA1A1A1A1u &&
           g_probe.r2 == 0x55667788u && g_probe.r3 == 0x11223344u));

    /* F4: 3 x u32 + u64 -> u64 r3'e SIGMAZ, stack'e gider.
     * r3 bu argumani tasimaz; ham gozlem olarak basilir. */
    probe_3u32_u64(0xA1A1A1A1u, 0xB2B2B2B2u, 0xC3C3C3C3u,
                   0x99AABBCCDDEEFF00ull);
    console_kv_hex("  F4 r0", g_probe.r0);
    console_kv_hex("  F4 r1", g_probe.r1);
    console_kv_hex("  F4 r2", g_probe.r2);
    console_kv_hex("  F4 r3 (unspecified)", g_probe.r3);
    console_kv_hex("  F4 SP", g_probe.sp);
    console_kv_hex("  F4 [SP+0] (d.lo)", g_probe.stk0);
    console_kv_hex("  F4 [SP+4] (d.hi)", g_probe.stk1);
    console_kv_str("  CHECK_U64_ON_STACK",
        pf(g_probe.r0 == 0xA1A1A1A1u && g_probe.r1 == 0xB2B2B2B2u &&
           g_probe.r2 == 0xC3C3C3C3u &&
           g_probe.stk0 == 0xDDEEFF00u && g_probe.stk1 == 0x99AABBCCu));
    console_kv_str("  CHECK_U64_STACK_SP_ALIGNED", pf((g_probe.sp & 7u) == 0u));

    /* F5: 64-bit donus -> r0:r1 */
    uint64_t v = probe_call_u64ret();
    console_kv_hex("  F5 r0 after BL", g_probe.ret_r0);
    console_kv_hex("  F5 r1 after BL", g_probe.ret_r1);
    console_kv_str("  CHECK_U64_RETURN_R0_R1",
        pf(g_probe.ret_r0 == 0x89ABCDEFu && g_probe.ret_r1 == 0x01234567u &&
           v == 0x0123456789ABCDEFull));
}

/* ------------------------------------------------------------------ */
static void fixture_chain(void)
{
    console_line("[2] C -> asm -> C zinciri");
    uint32_t r = abi_chain(10u);            /* helper_scale(10) + 10 = 41 */
    console_kv_u32("  CHAIN_RESULT", r);
    console_kv_hex("  CHAIN_SP_AT_BL", g_chain_sp_at_call);
    console_kv_str("  CHECK_CHAIN_RESULT", pf(r == 41u));
    console_kv_str("  CHECK_SP_ALIGNED_AT_CALL", pf((g_chain_sp_at_call & 7u) == 0u));
}

/* ------------------------------------------------------------------ */
static void fixture_harness(void)
{
    console_line("[3] Callee-saved harness");

    /* Pozitif kontrol ONCE: harness dogru bir callee'de temiz mi? */
    uint32_t res = 0u;
    uint32_t mask = abi_check_callee(good_callee, 7u, 5u, &res);
    console_kv_hex("  CONTROL_MASK", mask);
    console_kv_u32("  CONTROL_RESULT", res);
    console_kv_hex("  HARNESS_SP_AT_BLX", g_harness_sp_at_call);
    console_kv_str("  CHECK_HARNESS_POSITIVE_CONTROL",
                   pf(mask == 0u && res == 12u));
    console_kv_str("  CHECK_HARNESS_SP_ALIGNED",
                   pf((g_harness_sp_at_call & 7u) == 0u));

#ifdef L09_BROKEN
    console_line("--- test edilen: bad_callee (BREAK IT) ---");
    res = 0u;
    mask = abi_check_callee(bad_callee, 7u, 5u, &res);
#else
    console_line("--- test edilen: good_callee ---");
    res = 0u;
    mask = abi_check_callee(good_callee, 7u, 5u, &res);
#endif
    console_kv_u32("  TESTED_RESULT", res);
    console_kv_hex("  TESTED_MASK", mask);
    print_mask_regs("  CORRUPTED", mask);
    console_kv_str("  CHECK_RESULT", pf(res == 12u));
    console_kv_str("  CHECK_CALLEE_SAVED", pf(mask == 0u));
}

/* ------------------------------------------------------------------ */
int main(void)
{
    console_line("");
#ifdef L09_BROKEN
    console_line("===== LESSON 09: AAPCS (BROKEN) =====");
#else
    console_line("===== LESSON 09: AAPCS =====");
#endif
    fixture_args();
    fixture_chain();
    fixture_harness();

    /* Harness kendi cagiraninin durumunu geri yuklemeseydi, buradan
     * sonrasi (main'in kendi r4-r11 degerleri) bozulmus olurdu. */
    console_line("===== END =====");
    for (;;) {
        __WFI();
    }
}
