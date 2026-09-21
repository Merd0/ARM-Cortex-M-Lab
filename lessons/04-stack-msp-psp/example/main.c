/*
 * Lesson 04 - Stack, MSP & PSP
 *
 * Uc asamada ilerler:
 *   1) Reset sonrasi durum   : MSP var, PSP bos, CONTROL = 0
 *   2) PSP'ye adres yazilir   : PSP dolu ama AKTIF DEGIL
 *   3) psp_start() ile GECIS  : yeni bir C baglami PSP uzerinde baslar
 *
 * Arada iki olcum daha var:
 *   - bir fonksiyon cagrisinin SP'yi nasil hareket ettirdigi
 *   - elle yazilmis PUSH/POP'un SP'yi tam olarak kac byte oynattigi
 *
 * ONEMLI (onarim notu):
 * Bu ornegin ilk surumu CONTROL.SPSEL'i siradan bir C fonksiyonunun
 * ORTASINDA degistiriyordu. Bu gecersizdir: derleyici o fonksiyon icin
 * MSP uzerinde frame ayirmistir ve yerel degiskenlere [sp, #offset] ile
 * erisir. SPSEL degisince ayni offset'ler baska bir adrese denk gelir.
 * Bagimsiz bir denetim, -O2/-Os'de bu yuzden task_stack'in BIR USTUNE
 * yazildigini gosterdi. Simdi gecis example/psp_start.S icindeki kucuk
 * bir assembly parcasi ile yapiliyor ve ardindan SIFIRDAN bir C giris
 * noktasina (thread_entry) dallaniliyor.
 *
 * Build/run: lessons/04-stack-msp-psp/README.md
 */

#include "platform/mps2_an505.h"      /* CMSIS-Core */
#include "common/console.h"

/* ------------------------------------------------------------------ */
/* Thread Mode icin ayri bir stack alani + tasma nobetcileri (guard)   */
/*                                                                     */
/* aligned(8): AAPCS, bir genel arayuz sinirinda (yani normal bir      */
/* fonksiyon cagrisinda) SP'nin 8 byte hizali olmasini sart kosar.     */
/* Diziyi 8'e hizalayip kullanilabilir boyutu 8'in kati tutunca tepe   */
/* adresi de 8 byte hizali cikar.                                      */
/*                                                                     */
/* Blogun sonundaki iki word stack'e AIT DEGILDIR. Bilinen bir desenle */
/* doldurulur. Normal calisma sonunda bozulmamis olmalari, BU IKI      */
/* SECILMIS KONUMUN uzerine yazilmadigina dair kanit sunar - butun     */
/* olasi sinir ihlallerinin yoklugunu KANITLAMAZ (alt sinirdan tasma,  */
/* baska yere kacak yazma, ya da SP'nin gecici olarak araliktan        */
/* cikmasi bu yontemle gorunmez). Daha guclu kanit GDB olcumleridir.   */
/* ------------------------------------------------------------------ */
#define TASK_STACK_WORDS   128u                     /* 128 * 4 = 512 byte */
#define GUARD_WORDS        2u
#define GUARD_PATTERN      0xDEADBEEFu

static uint32_t task_stack_block[TASK_STACK_WORDS + GUARD_WORDS]
    __attribute__((aligned(8)));

#define TASK_STACK_BASE ((uint32_t)&task_stack_block[0])

/* Full-descending stack: tepe, kullanilabilir alanin SONUNUN BIR OTESIDIR.
 * Bu adresin kendisine hicbir zaman yazilmaz; ilk push once SP'yi azaltir. */
#define TASK_STACK_TOP  ((uint32_t)&task_stack_block[TASK_STACK_WORDS])

static void guards_init(void)
{
    for (unsigned i = 0u; i < GUARD_WORDS; i++) {
        task_stack_block[TASK_STACK_WORDS + i] = GUARD_PATTERN;
    }
}

static int guards_intact(void)
{
    for (unsigned i = 0u; i < GUARD_WORDS; i++) {
        if (task_stack_block[TASK_STACK_WORDS + i] != GUARD_PATTERN) {
            return 0;
        }
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Yardimcilar                                                         */
/* ------------------------------------------------------------------ */

/* Su anda hangi stack aktif? Once mod, sonra SPSEL.
 * Handler Mode'da SPSEL'in degeri ne olursa olsun MSP kullanilir. */
static const char *active_stack_name(void)
{
    if (__get_IPSR() != 0u) {
        return "MSP (Handler Mode)";
    }
    return (__get_CONTROL() & 0x2U) ? "PSP" : "MSP";
}

static void report_state(const char *label)
{
    console_line(label);
    console_kv_hex("  MSP", __get_MSP());
    console_kv_hex("  PSP", __get_PSP());
    console_kv_hex("  CONTROL", __get_CONTROL());
    console_kv_str("  ACTIVE_SP", active_stack_name());

    /* Yerel bir degiskenin adresi, o anda AKTIF olan stack'in icindedir.
     * Bu SP'nin kendisi degildir - derleyicinin frame'i icinde bir nokta. */
    volatile uint32_t probe = 0u;
    console_kv_hex("  LOCAL_ADDR", (uint32_t)&probe);
}

/* Derleyicinin gercekten stack frame uretmesi icin yeterince yerel durum
 * tasiyan, ama hala okunabilir bir fonksiyon.
 * noinline: cagri noktasina gomulmesini engeller.
 * Uretilen prolog'u objdump ile dogrulayacagiz - "her C fonksiyonu stack
 * kullanir" diye bir kural YOK. */
static uint32_t __attribute__((noinline))
frame_using_function(uint32_t seed, uint32_t *sp_inside)
{
    volatile uint32_t a = seed + 1u;
    volatile uint32_t b = seed + 2u;
    volatile uint32_t c = seed + 3u;
    volatile uint32_t d = seed + 4u;

    volatile uint32_t here = 0u;
    *sp_inside = (uint32_t)&here;

    return a + b + c + d;
}

/* Elle yazilmis assembly (example/push_pop_probe.S) */
void push_pop_probe(uint32_t out[3]);

/* MSP -> PSP gecisi (example/psp_start.S). DONMEZ. */
void psp_start(uint32_t psp_top, void (*entry)(void));

/* ------------------------------------------------------------------ */
/* BREAK IT: kontrollu stack tasmasi                                   */
/* ------------------------------------------------------------------ */
#ifdef LESSON04_BREAK_STACK_OVERFLOW
/*
 * Armv8-M Main Extension'da stack limit kontrolu MIMARI bir ozelliktir:
 * PSPLIM/MSPLIM'e bir alt sinir yazilir; bir stack islemi SP'yi bu
 * sinirin altina indirirse islemci UsageFault (UFSR.STKOF) uretir.
 *
 * Bu SADECE stack pointer guncellemelerini denetler - rastgele bellek
 * erisimlerini degil. Yani genel bir bellek korumasi degildir.
 *
 * Armv7-M'de (M3/M4/M7) bu register'lar yoktur; orada tasma otomatik
 * olarak yakalanmaz. "Sessizce bellegi bozar" da kesin bir ifade degil:
 * ne oldugu tasilan adresin ne oldugu ve MPU yapilandirmasi gibi
 * etkenlere baglidir.
 */
static uint32_t __attribute__((noinline)) recurse(uint32_t depth)
{
    volatile uint32_t pad[8];       /* her cagri stack'ten yer alir */
    pad[0] = depth;
    if (depth > 10000u) {           /* pratikte ulasilmaz: once tasar */
        return pad[0];
    }
    return recurse(depth + 1u) + pad[0];
}

static void break_it_stack_overflow(void)
{
    console_line("");
    console_line("--- BREAK IT: PSPLIM + derin recursion ---");

    /* UsageFault'u ayri gorebilmek icin etkinlestir; aksi halde
     * HardFault'a yukseltilir (escalation). */
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;

    __set_PSPLIM(TASK_STACK_BASE);
    __ISB();

    console_kv_hex("  PSPLIM", __get_PSPLIM());
    console_kv_hex("  PSP", __get_PSP());
    console_line("  recursion basliyor...");

    console_kv_u32("  RESULT", recurse(0u));
    console_line("  bu satiri gorebildin mi?");
}
#endif

/* ------------------------------------------------------------------ */
/* ASAMA 3: PSP uzerinde calisan TAZE C baglami                        */
/*                                                                     */
/* psp_start() buraya dallanir. Bu fonksiyon kendi prolog'unu yeni SP  */
/* (= PSP) uzerinde kurar; MSP'de birakilan frame'e bir daha erisilmez.*/
/* Donus yolu yoktur, bu yuzden noreturn.                              */
/* ------------------------------------------------------------------ */
static void __attribute__((noreturn)) thread_entry(void)
{
    report_state("[3] psp_start() sonrasi -> PSP aktif");

    /* Artik her fonksiyon cagrisi PSP uzerinde calisiyor. */
    console_line("--- PSP uzerinde fonksiyon cagrisi ---");
    uint32_t sp_inside_psp = 0u;
    (void)frame_using_function(20u, &sp_inside_psp);
    console_kv_hex("  PSP_NOW", __get_PSP());
    console_kv_hex("  SP_INSIDE(approx)", sp_inside_psp);
    console_kv_hex("  MSP_UNTOUCHED", __get_MSP());

    /* Tahsis edilen alanin disina yazildi mi? */
    console_kv_hex("  GUARD_ADDR", (uint32_t)&task_stack_block[TASK_STACK_WORDS]);
    console_kv_hex("  GUARD_VALUE", task_stack_block[TASK_STACK_WORDS]);
    console_kv_str("  CHECK_STACK_GUARD", guards_intact() ? "PASS" : "FAIL");

#ifdef LESSON04_BREAK_STACK_OVERFLOW
    break_it_stack_overflow();
#endif

    console_line("===== END =====");

    for (;;) {
        __WFI();
    }
}

/* ------------------------------------------------------------------ */
int main(void)
{
    console_line("");
    console_line("===== LESSON 04: STACK, MSP & PSP =====");

    guards_init();

    /* --- ASAMA 1: reset sonrasi --------------------------------- */
    report_state("[1] reset sonrasi");

    /* --- Fonksiyon cagrisi SP'yi ne kadar oynatiyor? ------------- */
    console_line("--- fonksiyon cagrisi ---");
    uint32_t sp_inside = 0u;
    volatile uint32_t sp_before_marker = 0u;
    uint32_t sum = frame_using_function(10u, &sp_inside);
    console_kv_hex("  SP_OUTSIDE(approx)", (uint32_t)&sp_before_marker);
    console_kv_hex("  SP_INSIDE(approx)", sp_inside);
    console_kv_u32("  SUM", sum);

    /* --- Elle yazilmis PUSH/POP --------------------------------- */
    console_line("--- push/pop (assembly) ---");
    uint32_t sp[3] = { 0u, 0u, 0u };
    push_pop_probe(sp);
    console_kv_hex("  SP_BEFORE_PUSH", sp[0]);
    console_kv_hex("  SP_AFTER_PUSH", sp[1]);
    console_kv_hex("  SP_AFTER_POP", sp[2]);
    console_kv_u32("  PUSH_DELTA_BYTES", sp[0] - sp[1]);
    console_kv_str("  SP_RESTORED", (sp[0] == sp[2]) ? "YES" : "NO");

    /* --- ASAMA 2: PSP'ye deger yaz, AMA aktif etme -------------- */
    console_line("--- PSP'ye adres yaziliyor ---");
    console_kv_hex("  TASK_STACK_BASE", TASK_STACK_BASE);
    console_kv_hex("  TASK_STACK_TOP", TASK_STACK_TOP);
    console_kv_u32("  TOP_MOD_8", TASK_STACK_TOP % 8u);

    __set_PSP(TASK_STACK_TOP);

    report_state("[2] PSP yazildi, SPSEL DEGISMEDI");

    /* --- ASAMA 3: gecerli bir yurutme siniri uzerinden gecis ----- */
    /* psp_start DONMEZ; main'in MSP uzerindeki frame'i burada terk
     * edilir ve bir daha erisilmez. */
    psp_start(TASK_STACK_TOP, thread_entry);
}
