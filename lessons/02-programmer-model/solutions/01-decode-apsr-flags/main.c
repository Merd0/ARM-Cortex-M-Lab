/*
 * Lesson 02 - Programmer Model & Registers
 *
 * Bu firmware'in tek isi var: Cortex-M33'un programmer model'ini
 * (programci modeli) CALISAN kod uzerinden gorunur kilmak.
 *
 * Okunan/gosterilenler:
 *   - CPUID          : hangi islemcideyiz?
 *   - MSP / PSP      : iki stack pointer
 *   - CONTROL        : privilege (nPRIV) + aktif stack (SPSEL)
 *   - xPSR / IPSR    : durum bayraklari + exception numarasi
 *   - PC / LR        : nerede calisiyoruz, nereye donecegiz
 *
 * Bu SOLUTION firmware'idir: exercise 01'in tamamlanmis hali.
 * Derlemek ve calistirmak icin: lessons/02-programmer-model/README.md
 */

#include "core/cm33_teaching.h"
#include "common/console.h"

/* ------------------------------------------------------------------ */
/* Yardimcilar                                                         */
/* ------------------------------------------------------------------ */

/* PC dogrudan okunabilir bir register'dir (R15).
 * Okunan deger, bu komutun adresi + 4'tur (Thumb pipeline offset).
 * Bu yuzden "approx" diyoruz: tam degeri disassembly ile karsilastir. */
static inline uint32_t read_pc_approx(void)
{
    uint32_t pc;
    __asm volatile ("mov %0, pc" : "=r"(pc));
    return pc;
}

/* LR (R14) bir fonksiyonun NEREYE donecegini tutar.
 * __builtin_return_address(0) derleyiciye "bu fonksiyonun donus adresi"
 * sorusunu sordurur; Cortex-M'de bu deger LR'den gelir. */
static uint32_t __attribute__((noinline)) leaf_function_return_address(void)
{
    return (uint32_t)__builtin_return_address(0);
}

/* ------------------------------------------------------------------ */
/* 1) Islemci kimligi                                                  */
/* ------------------------------------------------------------------ */
static void show_cpuid(void)
{
    uint32_t cpuid = SCB->CPUID;
    console_kv_hex("CPUID", cpuid);

    /* PARTNO alani (bit 15:4) islemci ailesini soyler.
     * 0xD21 = Cortex-M33  (Cortex-M33 Devices Generic User Guide) */
    uint32_t partno = (cpuid >> 4) & 0xFFFU;
    console_kv_hex("CPUID.PARTNO", partno);
    console_kv_str("PROCESSOR", (partno == 0xD21U) ? "Cortex-M33" : "other");
}

/* ------------------------------------------------------------------ */
/* 2) Mode / privilege / aktif stack                                   */
/* ------------------------------------------------------------------ */
static void show_execution_state(void)
{
    uint32_t ipsr    = __get_IPSR();
    uint32_t control = __get_CONTROL();

    /* IPSR == 0  -> Thread Mode
     * IPSR != 0  -> Handler Mode (calisan exception'in numarasi) */
    console_kv_str("MODE", (ipsr == 0U) ? "Thread Mode" : "Handler Mode");
    console_kv_u32("IPSR", ipsr);

    /* CONTROL.nPRIV (bit 0): 0 = privileged, 1 = unprivileged
     * CONTROL.SPSEL (bit 1): 0 = MSP kullaniliyor, 1 = PSP kullaniliyor */
    console_kv_hex("CONTROL", control);
    console_kv_str("PRIVILEGE", (control & 0x1U) ? "Unprivileged" : "Privileged");
    console_kv_str("ACTIVE_STACK", (control & 0x2U) ? "PSP" : "MSP");
}

/* ------------------------------------------------------------------ */
/* 3) Stack pointer'lar                                                */
/* ------------------------------------------------------------------ */
static void show_stack_pointers(void)
{
    /* Reset'ten sonra:
     *   MSP = vector table[0] (linker'daki __stack_top)
     *   PSP = tanimsiz/0 (henuz kimse kullanmadi)  */
    console_kv_hex("MSP", __get_MSP());
    console_kv_hex("PSP", __get_PSP());

    /* Yerel bir degiskenin adresi, su anda aktif olan stack'in icindedir. */
    volatile uint32_t local_variable = 0xC0FFEEU;
    console_kv_hex("LOCAL_VAR_ADDR", (uint32_t)&local_variable);
}

/* ------------------------------------------------------------------ */
/* 4) PC ve LR                                                         */
/* ------------------------------------------------------------------ */
static void show_pc_and_lr(void)
{
    console_kv_hex("PC_APPROX", read_pc_approx());
    console_kv_hex("LEAF_RETURN_ADDR", leaf_function_return_address());
}

/* ------------------------------------------------------------------ */
/* 5) xPSR bayraklari                                                  */
/* ------------------------------------------------------------------ */

/* Bu fonksiyon bilerek "bayrak ureten" bir islem yapar ve hemen
 * sonrasinda APSR'yi okur. Derleyici araya baska komut sokarsa deger
 * degisebilir - bu yuzden inline assembly ile tek blokta yapiyoruz. */
static uint32_t subs_then_read_apsr(uint32_t a, uint32_t b)
{
    uint32_t apsr;
    __asm volatile (
        "subs   %[tmp], %[x], %[y]  \n"   /* bayraklari GUNCELLE (s = set flags) */
        "mrs    %[out], apsr        \n"   /* APSR'yi oku                          */
        : [out] "=r" (apsr), [tmp] "=&r" (a)
        : [x] "r" (a), [y] "r" (b)
        : "cc"
    );
    return apsr;
}

/*
 * SOLUTION 01 - decode_apsr_flags()
 *
 * Referans cozum. Once kendin denemediysen:
 *     exercises/01-decode-apsr-flags/
 *
 * Yaklasim: her bayrak tek bir bit, bu yuzden kaydir-ve-maskele yeterli.
 * (apsr >> 31) & 1  -> N       (apsr >> 30) & 1  -> Z
 * (apsr >> 29) & 1  -> C       (apsr >> 28) & 1  -> V
 *
 * '0' + bit numarasi, bit'i dogrudan ASCII karaktere cevirir; sprintf ya da
 * ek bir yardimci fonksiyon gerekmez. Bare-metal'de her satir kod, flash
 * demektir.
 *
 * Neden bit kaydirma, bit alani (bitfield) degil?
 *   C standardi bitfield'larin bellekteki yerlesimini garanti etmez
 *   (implementation-defined). Donanim register'i okurken maske + kaydirma
 *   tasinabilir olan tek yontemdir.
 */
static void decode_apsr_flags(uint32_t apsr)
{
    console_puts("[FLAGS] N=");
    console_putc((char)('0' + ((apsr >> 31) & 1U)));
    console_puts(" Z=");
    console_putc((char)('0' + ((apsr >> 30) & 1U)));
    console_puts(" C=");
    console_putc((char)('0' + ((apsr >> 29) & 1U)));
    console_puts(" V=");
    console_putc((char)('0' + ((apsr >> 28) & 1U)));
    console_putc('\n');
}

static void show_flags(void)
{
    uint32_t apsr_equal    = subs_then_read_apsr(5U, 5U);
    uint32_t apsr_negative = subs_then_read_apsr(5U, 7U);

    console_kv_hex("APSR_AFTER_5_MINUS_5", apsr_equal);
    decode_apsr_flags(apsr_equal);

    console_kv_hex("APSR_AFTER_5_MINUS_7", apsr_negative);
    decode_apsr_flags(apsr_negative);
}

/* ------------------------------------------------------------------ */
/* 6) BREAK IT: kontrollu hata                                         */
/* ------------------------------------------------------------------ */
#ifdef LESSON02_BREAK_SPSEL
/*
 * Bu blok yalnizca "02-programmer-model-broken" hedefinde derlenir.
 *
 * Yaptigi sey: PSP'ye gecerli bir deger YAZMADAN CONTROL.SPSEL = 1 yapar.
 * Yani islemciye "bundan sonra PSP kullan" der, ama PSP = 0'dir.
 * Bir sonraki stack islemi 0x00000000 civarina yazmaya calisir.
 *
 * Ne olacagini simdiden okuma - once calistir, sonra debug et.
 */
static void break_it_switch_to_empty_psp(void)
{
    console_line("");
    console_line("--- BREAK IT: SPSEL=1, PSP hazirlanmadan ---");
    __set_CONTROL(__get_CONTROL() | 0x2U);   /* SPSEL = 1 -> PSP aktif */

    /* Buradan sonra her fonksiyon cagrisi PSP uzerinde stack kullanir. */
    console_line("bu satiri gorebildin mi?");
}
#endif

/* ------------------------------------------------------------------ */
int main(void)
{
    console_line("");
    console_line("===== LESSON 02 SOLUTION 01 =====");

    show_cpuid();
    console_line("--- execution state ---");
    show_execution_state();
    console_line("--- stack pointers ---");
    show_stack_pointers();
    console_line("--- pc / lr ---");
    show_pc_and_lr();
    console_line("--- xpsr flags ---");
    show_flags();

#ifdef LESSON02_BREAK_SPSEL
    break_it_switch_to_empty_psp();
#endif

    console_line("===== END =====");

    for (;;) {
        __WFI();
    }
}
