/*
 * Lesson 05 SOLUTION 01 - Add an SVC Service
 *
 * Tek bir zinciri gozle gorulur kiliyor:
 *
 *   Privileged Thread (PSP)
 *     -> CONTROL.nPRIV = 1
 *   Unprivileged Thread
 *     -> dogrudan kendini yukseltme DENEMESI  -> BASARISIZ
 *     -> SVC
 *   Handler Mode (ayricalikli, MSP)
 *     -> ayricalikli servis
 *     -> exception return
 *   Thread Mode devam eder
 *
 * Build/run: lessons/05-privilege-svc/README.md
 */

#include "platform/mps2_an505.h"      /* CMSIS-Core */
#include "common/console.h"
#include "platform/board.h"

/* ------------------------------------------------------------------ */
/* Thread Mode stack'i (ders 04'teki duzen)                            */
/* ------------------------------------------------------------------ */
#define TASK_STACK_WORDS   192u
#define GUARD_WORDS        2u
#define GUARD_PATTERN      0xDEADBEEFu

static uint32_t task_stack_block[TASK_STACK_WORDS + GUARD_WORDS]
    __attribute__((aligned(8)));

#define TASK_STACK_TOP ((uint32_t)&task_stack_block[TASK_STACK_WORDS])

void psp_start(uint32_t psp_top, void (*entry)(void));   /* DONMEZ */

/* ------------------------------------------------------------------ */
/* SVC servis numaralari                                               */
/* ------------------------------------------------------------------ */
#define SVC_INCREMENT        1u
#define SVC_GET_VERSION      2u    /* Exercise 01: ogrenci ekleyecek  */
#define SVC_RESTORE_PRIV     4u
#define SVC_ERROR_UNKNOWN    0xFFFFFFFFu

/* Servisin dokundugu durum.
 * DIKKAT: bu siradan bir RAM degiskenidir. Su asamada onu koruyan sey
 * DONANIM DEGIL, uygulama sozlesmesidir ("buraya yalnizca SVC yolundan
 * dokunulur"). Gercek bellek izolasyonu ders 06'da MPU ile gelir. */
static volatile uint32_t g_service_counter;

/* SVC_Handler sarmalayicisinin dispatcher'a gecirdigi frame duzeni.
 * Standart 8 word exception frame (soft-float build). */
typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;      /* yigilmis PC: SVC komutundan SONRAKI komut */
    uint32_t xpsr;
} exception_frame_t;

/* ------------------------------------------------------------------ */
/* C dispatcher. svc_handler.S buraya DALLANIR (bl degil, b).           */
/* r0 = frame adresi, r1 = EXC_RETURN. lr hala EXC_RETURN'dur, bu       */
/* yuzden bu fonksiyonun normal donusu exception return'u tetikler.     */
/* ------------------------------------------------------------------ */
void svc_dispatch(exception_frame_t *frame, uint32_t exc_return);

void svc_dispatch(exception_frame_t *frame, uint32_t exc_return)
{
    /* SVC numarasini bul.
     *
     * Yigilmis PC, SVC komutundan SONRAKI komutu gosterir. Thumb'da
     * SVC'nin T1 kodlamasi 16 BITTIR: 0b11011111:imm8 (0xDFxx). Yani
     * komutun kendisi (pc - 2) adresindedir ve dusuk byte imm8'dir.
     *
     * BU GENEL BIR KURAL DEGILDIR. Yalnizca 16 bitlik SVC komutu icin
     * gecerlidir; "her exception'da hatali komut pc-2'dedir" diye bir sey
     * YOKTUR (bkz. ders 08). Uretilen kodlamayi objdump ile dogrula. */
    const uint8_t *svc_instr = (const uint8_t *)(frame->pc - 2u);
    uint32_t svc_number = (uint32_t)svc_instr[0];

    uint32_t result;

    switch (svc_number) {
    case SVC_INCREMENT:
        g_service_counter++;
        result = g_service_counter;
        break;

    case SVC_GET_VERSION:
        /* get_service_version(): sabit bir surum/kabiliyet degeri.
         * Ayricalikli Handler Mode'da calisir. Sonuc asagida frame->r0'a
         * yazilir; exception return sirasinda donanim o word'u Thread'in
         * R0'ina geri yukler ve cagiran kod onu fonksiyon donus degeri
         * gibi okur. Ayrinti: solutions/01-add-svc-service/README.md */
        result = 0x00010002u;
        break;

    case SVC_RESTORE_PRIV:
        /* Ayricalikli HANDLER, donecek Thread baglaminin nPRIV bitini
         * temizliyor. Thread kendini yukseltmiyor - politika karari
         * burada, ayricalikli tarafta veriliyor. */
        __set_CONTROL(__get_CONTROL() & ~0x1u);
        __ISB();
        result = 0u;
        break;

    default:
        /* Tanimsiz servis: sessizce rastgele bir is YAPILMAZ. */
        console_kv_u32("  UNKNOWN_SVC", svc_number);
        result = SVC_ERROR_UNKNOWN;
        break;
    }

    /* Sonuc, Thread'e frame'in R0 yuvasindan doner: exception return
     * sirasinda donanim o word'u R0'a geri yukler ve cagiran kod onu
     * fonksiyon donus degeri gibi okur (AAPCS: r0 = donus degeri). */
    frame->r0 = result;

    (void)exc_return;
}

/* ------------------------------------------------------------------ */
/* SVC cagrilari                                                       */
/* SVC'nin immediate'i DERLEME ZAMANI sabitidir; degisken olamaz.      */
/* Bu yuzden her servis icin ayri bir satir ici asm gerekir.           */
/* ------------------------------------------------------------------ */
#define SVC_CALL(num)                                                   \
    ({                                                                  \
        register uint32_t _r0 __asm__("r0");                            \
        __asm__ volatile ("svc %[imm]"                                  \
                          : "=r" (_r0)                                  \
                          : [imm] "I" (num)                             \
                          : "memory");                                  \
        _r0;                                                            \
    })

/* ------------------------------------------------------------------ */
/* Durum raporlama                                                     */
/* ------------------------------------------------------------------ */
static const char *mode_name(void)
{
    return (__get_IPSR() != 0u) ? "Handler" : "Thread";
}

static const char *privilege_name(void)
{
    /* Handler Mode HER ZAMAN ayricaliklidir. CONTROL.nPRIV, THREAD
     * Mode'un ayricaligini tanimlar - mevcut handler'inkini degil. */
    if (__get_IPSR() != 0u) {
        return "Privileged (Handler Mode)";
    }
    return (__get_CONTROL() & 0x1u) ? "Unprivileged" : "Privileged";
}

static void report(const char *label)
{
    console_line(label);
    console_kv_str("  MODE", mode_name());
    console_kv_str("  PRIVILEGE", privilege_name());
    console_kv_hex("  CONTROL", __get_CONTROL());
    console_kv_u32("  IPSR", __get_IPSR());
    console_kv_hex("  PSP", __get_PSP());
    volatile uint32_t probe = 0u;
    console_kv_hex("  LOCAL_ADDR", (uint32_t)&probe);
}

/* ------------------------------------------------------------------ */
/* PSP uzerinde calisan taze Thread baglami                            */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* Ayricaliksiz bolumde TOPLANAN gozlemler.                            */
/*                                                                     */
/* Neden RAM'e kaydedip sonra basiyoruz? Cunku bu hedefte ayricaliksiz */
/* Thread Mode UART'a yazamiyor - asagidaki UART_PROBE olcumu bunu     */
/* gosteriyor. Bu, MPU'dan ONCE var olan gercek bir ayricalik siniri.  */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t control_privileged;   /* dusurmeden once            */
    uint32_t control_unpriv;       /* nPRIV = 1 sonrasi          */
    uint32_t ipsr_unpriv;
    uint32_t psp_unpriv;
    uint32_t control_after_attempt;/* kendini yukseltme sonrasi  */
    uint32_t uart_bauddiv_priv;    /* ayni register, ayricalikli okuma  */
    uint32_t uart_bauddiv_unpriv;  /* ayricaliksiz okuma                */
    uint32_t svc1_first;
    uint32_t svc1_second;
    uint32_t control_in_thread_after_svc;
} observation_t;

static volatile observation_t g_obs;

/* Cevre birimine dogrudan erisim probu.
 *
 * BAUDDIV'i okuyoruz cunku degerini BIZ yazdik (system_init.c) ve sifir
 * DEGIL - yani "0 okudum" sonucu anlamli. STATE gibi bir register'i
 * okumak yaniltici olurdu: bos bir UART'ta zaten 0 olabilir.
 *
 * Ayni fonksiyon hem ayricalikli hem ayricaliksiz baglamdan cagrilir;
 * karsilastirma boylece kendi icinde kontrollu olur. */
static uint32_t uart_bauddiv_probe(void)
{
    volatile const uint32_t *uart_bauddiv =
        (volatile const uint32_t *)(BOARD_UART0_S_BASE + 0x10u);
    return *uart_bauddiv;
}

/* ------------------------------------------------------------------ */
/* PSP uzerinde calisan taze Thread baglami                            */
/* ------------------------------------------------------------------ */
static void __attribute__((noreturn)) thread_entry(void)
{
    report("[1] Thread Mode, ayricalikli, PSP uzerinde");
    g_obs.control_privileged  = __get_CONTROL();
    g_obs.uart_bauddiv_priv   = uart_bauddiv_probe();   /* referans olcum */

    /* --- 2) Ayricaligi GONULLU olarak birak ---------------------- */
    console_line("--- CONTROL.nPRIV = 1 (ayricalikli kod yapiyor) ---");
    console_line("    (buradan sonraki konsol ciktisi SVC'ye kadar susacak)");
    __set_CONTROL(__get_CONTROL() | 0x1u);
    __ISB();

    /* ====== BURADAN ITIBAREN AYRICALIKSIZ THREAD MODE ============= */
    g_obs.control_unpriv = __get_CONTROL();
    g_obs.ipsr_unpriv    = __get_IPSR();
    g_obs.psp_unpriv     = __get_PSP();

    /* --- 3) Dogrudan kendini yukseltme denemesi ------------------ */
    __set_CONTROL(__get_CONTROL() & ~0x1u);   /* yok sayilmali */
    __ISB();
    g_obs.control_after_attempt = __get_CONTROL();

    /* --- 3b) Ayricaliksiz cevre birimi erisimi -------------------- */
    g_obs.uart_bauddiv_unpriv = uart_bauddiv_probe();

    /* --- 4) SVC: kontrollu ayricalikli yol ----------------------- */
    g_obs.svc1_first  = SVC_CALL(SVC_INCREMENT);
    g_obs.svc1_second = SVC_CALL(SVC_INCREMENT);
    g_obs.control_in_thread_after_svc = __get_CONTROL();

#ifdef LESSON05_BREAK_UNKNOWN_SVC
    uint32_t rbad = SVC_CALL(99);
#endif
#ifdef LESSON05_SOLUTION_SVC2
    uint32_t ver = SVC_CALL(SVC_GET_VERSION);
#endif
#ifdef LESSON05_EXERCISE_SVC2
    uint32_t ver = SVC_CALL(SVC_GET_VERSION);
#endif

    /* --- 5) Ayricaligi HANDLER geri veriyor ---------------------- */
    (void)SVC_CALL(SVC_RESTORE_PRIV);
    /* ====== YENIDEN AYRICALIKLI THREAD MODE ======================= */

    console_line("");
    console_line("[2] Ayricaliksiz bolumde kaydedilenler");
    console_kv_hex("  CONTROL_BEFORE_DROP", g_obs.control_privileged);
    console_kv_hex("  CONTROL_UNPRIV", g_obs.control_unpriv);
    console_kv_u32("  IPSR_UNPRIV", g_obs.ipsr_unpriv);
    console_kv_hex("  PSP_UNPRIV", g_obs.psp_unpriv);
    console_kv_str("  CHECK_PRIV_DROPPED",
                   ((g_obs.control_unpriv & 0x1u) != 0u) ? "PASS" : "FAIL");

    console_line("[3] Dogrudan kendini yukseltme denemesi");
    console_kv_hex("  CONTROL_AFTER_ATTEMPT", g_obs.control_after_attempt);
    console_kv_str("  CHECK_SELF_PROMOTION_BLOCKED",
                   ((g_obs.control_after_attempt & 0x1u) != 0u) ? "PASS" : "FAIL");

    console_line("[4] Cevre birimi erisimi: ayni register, iki baglam");
    console_kv_u32("  UART_BAUDDIV_PRIV", g_obs.uart_bauddiv_priv);
    console_kv_u32("  UART_BAUDDIV_UNPRIV", g_obs.uart_bauddiv_unpriv);
    console_kv_str("  CHECK_PERIPHERAL_BLOCKED_UNPRIV",
                   (g_obs.uart_bauddiv_priv != g_obs.uart_bauddiv_unpriv)
                       ? "PASS" : "FAIL");

    console_line("[5] SVC uzerinden ayricalikli servis");
    console_kv_u32("  SVC1_RESULT", g_obs.svc1_first);
    console_kv_u32("  SVC1_RESULT_AGAIN", g_obs.svc1_second);
    console_kv_str("  CHECK_SVC_SERVICE",
                   (g_obs.svc1_second == g_obs.svc1_first + 1u) ? "PASS" : "FAIL");
    console_kv_hex("  CONTROL_IN_THREAD_AFTER_SVC", g_obs.control_in_thread_after_svc);
    console_kv_str("  CHECK_SVC_DID_NOT_PROMOTE",
                   ((g_obs.control_in_thread_after_svc & 0x1u) != 0u) ? "PASS" : "FAIL");

#ifdef LESSON05_BREAK_UNKNOWN_SVC
    console_line("[6] BREAK IT: SVC #99 (tanimsiz servis)");
    console_kv_hex("  SVC99_RESULT", rbad);
    console_kv_str("  CHECK_UNKNOWN_SVC_REJECTED",
                   (rbad == SVC_ERROR_UNKNOWN) ? "PASS" : "FAIL");
#endif
#if defined(LESSON05_SOLUTION_SVC2) || defined(LESSON05_EXERCISE_SVC2)
    console_line("[6] SVC #2 (get_service_version)");
    console_kv_hex("  SVC2_VERSION", ver);
    console_kv_str("  CHECK_SVC2_VERSION",
                   (ver == 0x00010002u) ? "PASS" : "FAIL");
#endif

    report("[7] Handler nPRIV'i temizledi - Thread yeniden ayricalikli");
    console_kv_str("  CHECK_PRIV_RESTORED_BY_HANDLER",
                   ((__get_CONTROL() & 0x1u) == 0u) ? "PASS" : "FAIL");

    console_line("===== END =====");

    for (;;) {
        __WFI();
    }
}

/* ------------------------------------------------------------------ */
int main(void)
{
    console_line("");
    console_line("===== LESSON 05 SOLUTION 01 =====");

    for (unsigned i = 0u; i < GUARD_WORDS; i++) {
        task_stack_block[TASK_STACK_WORDS + i] = GUARD_PATTERN;
    }

    console_kv_hex("MSP_AT_START", __get_MSP());
    console_kv_hex("TASK_STACK_TOP", TASK_STACK_TOP);

    /* Ders 04'teki gecerli yurutme siniri: assembly gecisi + taze C girisi */
    psp_start(TASK_STACK_TOP, thread_entry);   /* donmez */
}
