/*
 * Lesson 11 - Startup: Reset -> main()
 *
 * Bu hedef (11-startup) repo'nun GERCEK, paylasilan startup yolunu
 * kullanir: platform/mps2-an505/src/startup_mps2_an505.c icindeki
 * vektor tablosu ve Reset_Handler, system_init.c icindeki SystemInit.
 * Hicbirini degistirmez; yalnizca main'e gelindiginde geride biraktiklari
 * izleri olcer.
 *
 * main, main_entry.S'teki kucuk bir shim'dir: giris SP ve LR'sini
 * derleyici prologundan ONCE yakalar ve l11_main(sp, lr)'ye dallanir.
 *
 * l11_main SONUNDA BILEREK DONER: boylece paylasilan Reset_Handler'in
 * main-sonrasi WFI dongusu calisir (README, "main donerse").
 */
#include "platform/mps2_an505.h"
#include "platform/board.h"
#include "common/console.h"
#include "fixtures.h"

extern void Reset_Handler(void);
extern void Default_Handler(void);

int l11_main(uint32_t sp_at_main, uint32_t lr_at_main);

/* Weak override gosterimi: startup'taki SysTick_Handler bir WEAK alias
 * (-> Default_Handler). Bu GUCLU tanim link zamaninda onun yerini alir ve
 * vektor tablosunun 15. girisine bu fonksiyon yazilir. SysTick bu derste
 * hic etkinlestirilmez; bu yalnizca link zamani mekanizmasinin kanitidir. */
void SysTick_Handler(void)
{
}

static const char *pf(int ok) { return ok ? "PASS" : "FAIL"; }
static uint32_t A(const void *p) { return (uint32_t)(uintptr_t)p; }

#define UART0_BAUDDIV (*(volatile uint32_t *)(BOARD_UART0_S_BASE + 0x10u))

int l11_main(uint32_t sp_at_main, uint32_t lr_at_main)
{
    const uint32_t *vec     = __vectors_start;
    const uint32_t reset_fn = A((const void *)&Reset_Handler);  /* Thumb bitli */
    const uint32_t reset_ia = reset_fn & ~1u;                    /* komut adresi */

    console_line("");
    console_line("===== LESSON 11: STARTUP (RESET -> main) =====");

    console_line("[1] Vektor tablosu ve ilk MSP");
    console_kv_hex("  vector[0]", vec[0]);
    console_kv_hex("  &__stack_top", A(&__stack_top));
    console_kv_hex("  vector[1]", vec[1]);
    console_kv_hex("  Reset_Handler komut adresi", reset_ia);
    console_kv_hex("  VTOR", SCB->VTOR);
    console_kv_hex("  MSPLIM (repo programlamiyor)", __get_MSPLIM());

    console_line("[2] C calisma ortami (.data / .bss)");
    console_kv_hex("  g_l11_init", g_l11_init);
    console_kv_hex("  g_l11_zero", g_l11_zero);
    console_line("  NOT: .bss'in main'de sifir olmasi tek basina KANIT DEGIL -");
    console_line("       QEMU RAM'i sifirla baslatiyor. Kanit: README, GDB durak D.");

    console_line("[3] main'e devir (main_entry.S shim'i)");
    console_kv_hex("  SP main girisinde", sp_at_main);
    console_kv_hex("  LR main girisinde", lr_at_main);
    console_kv_hex("  LR hedefindeki halfword",
                   (uint32_t)*(const volatile uint16_t *)(uintptr_t)(lr_at_main & ~1u));
    console_kv_hex("  CONTROL", __get_CONTROL());
    console_kv_hex("  IPSR", __get_IPSR());
    console_kv_u32("  UART0 BAUDDIV", UART0_BAUDDIV);

    console_line("[4] Kontroller");
    console_kv_str("  CHECK_VECTOR0_EQ_STACK_TOP", pf(vec[0] == A(&__stack_top)));
    console_kv_str("  CHECK_VECTOR1_EQ_RESET_HANDLER", pf(vec[1] == reset_fn));
    console_kv_str("  CHECK_VECTOR1_THUMB_BIT", pf((vec[1] & 1u) == 1u));
    console_kv_str("  CHECK_DATA_INIT", pf(g_l11_init == L11_INIT_VALUE));
    console_kv_str("  CHECK_BSS_ZERO", pf(g_l11_zero == 0u));
    console_kv_str("  CHECK_MAIN_SP_ALIGNED", pf((sp_at_main & 7u) == 0u));
    console_kv_str("  CHECK_MAIN_SP_BELOW_STACK_TOP",
                   pf(sp_at_main <= A(&__stack_top) && sp_at_main > A(&__stack_limit)));
    /* LR, Reset_Handler'in icine (bl main'den sonraki komuta) donmeli ve
     * Thumb biti set olmali. Kesin komut: statik inceleme + GDB (README). */
    console_kv_str("  CHECK_MAIN_LR_IN_RESET_HANDLER",
                   pf(((lr_at_main & 1u) == 1u) &&
                      ((lr_at_main & ~1u) > reset_ia) &&
                      ((lr_at_main & ~1u) < reset_ia + 0x100u)));
    console_kv_str("  CHECK_SYSTEMINIT_UART", pf(UART0_BAUDDIV != 0u));
    console_kv_str("  CHECK_VTOR_IS_VECTORS", pf(SCB->VTOR == A(__vectors_start)));
    console_kv_str("  CHECK_THREAD_MODE_PRIVILEGED",
                   pf(__get_IPSR() == 0u && (__get_CONTROL() & 3u) == 0u));

    console_line("===== END =====");
    console_line("main donuyor -> Reset_Handler'in WFI dongusu");
    return 0;
}
