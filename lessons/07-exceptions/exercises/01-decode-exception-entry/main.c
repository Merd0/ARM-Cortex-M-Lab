/*
 * Lesson 07 EXERCISE 01 - Decode an Exception Entry
 *
 * Simdiye kadar exception mekanizmasini KULLANDIK (ders 02, 04, 05, 06).
 * Burada CPU'nun exception'a girerken ne yaptigini sistematik olarak
 * olcuyoruz:
 *
 *   komut/olay -> pending -> vector lookup -> donanimsal yigma
 *   -> Handler Mode -> handler -> EXC_RETURN -> geri yukleme -> devam
 *
 * Exception kaynagi olarak SVC seciliyor: senkron, komutu bilinen,
 * yigmanin basarili olmasi beklenen en temiz kaynak.
 *
 * NOT: bu derste Thread Mode AYRICALIKLI kaliyor. Ders 05/06 ayricalik ve
 * izolasyonu isledi; burada odak exception mekanizmasi. Ayricalikli kalmak
 * Thread'in dogrudan konsola yazabilmesini saglar (ders 05'te olculen PPC
 * engeli ayricaliksiz koda aitti).
 *
 * Build/run: lessons/07-exceptions/README.md
 */

#include "platform/mps2_an505.h"
#include "common/console.h"

/* ------------------------------------------------------------------ */
/* Handler sarmalayicilarinin giriste kaydettikleri degerler.          */
/* Bunlar handler icinde C calistiktan SONRA artik guvenilir degildir. */
/* ------------------------------------------------------------------ */
volatile uint32_t g_entry_lr;      /* EXC_RETURN                        */
volatile uint32_t g_entry_msp;
volatile uint32_t g_entry_psp;
volatile uint32_t g_frame_addr;    /* EXC_RETURN bit2'den turetildi     */

/* Dispatcher'in KENDI stack kullanimindan sonraki SP - aktif $sp'nin
 * neden otomatik olarak frame adresi olmadigini gostermek icin. */
volatile uint32_t g_dispatcher_sp;

/* ------------------------------------------------------------------ */
/* Standart (basic) exception frame - 8 word.                          */
/* Sira mimari tarafindan sabittir; adresler build'e ozeldir.          */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t r0;      /* +0x00 */
    uint32_t r1;      /* +0x04 */
    uint32_t r2;      /* +0x08 */
    uint32_t r3;      /* +0x0C */
    uint32_t r12;     /* +0x10 */
    uint32_t lr;      /* +0x14 */
    uint32_t pc;      /* +0x18  kesilen baglamin devam adresi */
    uint32_t xpsr;    /* +0x1C */
} exception_frame_t;

/* Yakalanan frame'in kopyasi (Thread doner donmez bakabilmek icin). */
static volatile exception_frame_t g_saved_frame;
static volatile uint32_t g_saved_ipsr;

/* ------------------------------------------------------------------ */
/* Thread stack'i (ders 04'te dogrulanan duzen)                        */
/* ------------------------------------------------------------------ */
#define TASK_STACK_WORDS   192u
#ifndef LESSON07_MSP_ORIGIN
static uint32_t task_stack[TASK_STACK_WORDS] __attribute__((aligned(8)));
#endif
#define TASK_STACK_TOP ((uint32_t)&task_stack[TASK_STACK_WORDS])

void psp_start(uint32_t psp_top, void (*entry)(void));   /* DONMEZ */

/* ------------------------------------------------------------------ */
/* SVC servisleri                                                      */
/* ------------------------------------------------------------------ */
#define SVC_ECHO             7u    /* R0'i 1 artirip geri dondurur */
#define SVC_ERROR_UNKNOWN    0xFFFFFFFFu

void svc_dispatch(exception_frame_t *frame, uint32_t exc_return);

void svc_dispatch(exception_frame_t *frame, uint32_t exc_return)
{
    /* Dispatcher'in kendi yerel durumu - SP'yi frame'in basindan
     * uzaklastirmasi icin bilerek birkac word. */
    volatile uint32_t pad[4];
    for (unsigned i = 0u; i < 4u; i++) {
        pad[i] = 0xC0DE0000u + i;
    }
    g_dispatcher_sp = (uint32_t)&pad[0];

    g_saved_ipsr = __get_IPSR();

    /* Frame'i kopyala: Thread donduktan sonra incelenebilsin. */
    g_saved_frame.r0   = frame->r0;
    g_saved_frame.r1   = frame->r1;
    g_saved_frame.r2   = frame->r2;
    g_saved_frame.r3   = frame->r3;
    g_saved_frame.r12  = frame->r12;
    g_saved_frame.lr   = frame->lr;
    g_saved_frame.pc   = frame->pc;
    g_saved_frame.xpsr = frame->xpsr;

    /* SVC numarasi: T1 kodlamasi 16 bit (0xDFxx), komut pc-2'de.
     * Genel bir kural DEGIL (ders 05/08). */
    const uint8_t *svc_instr = (const uint8_t *)(frame->pc - 2u);
    uint32_t svc_number = (uint32_t)svc_instr[0];

    frame->r0 = (svc_number == SVC_ECHO) ? (frame->r0 + 1u)
                                         : SVC_ERROR_UNKNOWN;
    (void)exc_return;
}

/* ------------------------------------------------------------------ */
/* Fault yolu (Break It hedefleri)                                     */
/* ------------------------------------------------------------------ */
#if defined(LESSON07_BREAK_MEMMANAGE) || defined(LESSON07_BREAK_ESCALATION)

void mpu_store_word(volatile uint32_t *addr, uint32_t value);   /* mpu_probe.S */
void fault_report(exception_frame_t *frame, uint32_t exc_return);

/* Ayricalikli SALT-OKUNUR yapilacak hedef word. 32 byte hizali ve
 * 32 byte boyutunda kendi bolgesi olacak. */
static volatile uint32_t g_ro_block[8] __attribute__((aligned(32)));

static void mpu_setup_readonly_region(int enable_memmanage)
{
    ARM_MPU_Disable();

    ARM_MPU_SetMemAttr(0u,
        ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1u, 1u, 1u, 1u),
                     ARM_MPU_ATTR_MEMORY_(1u, 1u, 1u, 1u)));

    /* Region 0: g_ro_block -> AYRICALIKLI SALT-OKUNUR.
     * RO = 1, NP = 0 (yalnizca ayricalikli), XN = 1.
     * Ayricalikli bir YAZMA bile bu bolgede reddedilir. */
    ARM_MPU_SetRegion(0u,
        ARM_MPU_RBAR((uint32_t)&g_ro_block[0], ARM_MPU_SH_NON,
                     1u /* RO */, 0u /* PO: yalnizca ayricalikli */, 1u /* XN */),
        ARM_MPU_RLAR((uint32_t)&g_ro_block[0] + sizeof(g_ro_block) - 1u, 0u));

    /* PRIVDEFENA = 1: diger her sey varsayilan haritadan calismaya devam
     * etsin; yalnizca bu bolge kisitli olsun. */
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

    /* SIRA ONEMLI - olculmus bir tuzak:
     * CMSIS'in ARM_MPU_Enable() fonksiyonu KENDI ICINDE
     *     SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
     * yapar (bkz. external/CMSIS/.../armv8m_mpu.h). Yani MemManage'i
     * ondan ONCE temizlemek etkisiz kalir. Devre disi birakmak istiyorsak
     * ARM_MPU_Enable()'dan SONRA temizlemeliyiz. */
    if (enable_memmanage) {
        SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    } else {
        SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTENA_Msk;
    }
    __DSB();
    __ISB();
}

void fault_report(exception_frame_t *frame, uint32_t exc_return)
{
    uint32_t cfsr = SCB->CFSR;
    uint32_t hfsr = SCB->HFSR;

    console_line("");
    console_line("--- FAULT ---");
    console_kv_u32("  IPSR", __get_IPSR());
    console_kv_hex("  CFSR", cfsr);
    console_kv_hex("  MMFSR", cfsr & 0xFFu);
    console_kv_hex("  HFSR", hfsr);
    console_kv_str("  HFSR_FORCED",
                   ((hfsr & SCB_HFSR_FORCED_Msk) != 0u) ? "1" : "0");
    console_kv_str("  MMARVALID",
                   ((cfsr & SCB_CFSR_MMARVALID_Msk) != 0u) ? "1" : "0");
    console_kv_hex("  MMFAR", SCB->MMFAR);
    console_kv_hex("  EXC_RETURN", exc_return);
    console_kv_hex("  FRAME_ADDR", (uint32_t)frame);
    console_kv_hex("  RO_BLOCK_ADDR", (uint32_t)&g_ro_block[0]);
    console_kv_hex("  SHCSR", SCB->SHCSR);
    console_kv_str("  MEMFAULTENA",
                   ((SCB->SHCSR & SCB_SHCSR_MEMFAULTENA_Msk) != 0u) ? "1" : "0");

    /* Yigma basarili mi? Basarisizsa frame'i COZME. */
    int stacking_failed =
        ((cfsr & SCB_CFSR_MSTKERR_Msk) != 0u) ||
        ((cfsr & SCB_CFSR_STKERR_Msk)  != 0u) ||
        ((cfsr & SCB_CFSR_STKOF_Msk)   != 0u);

    if (stacking_failed) {
        console_line("  FRAME: yigma hatasi bildirildi -> COZULMEDI");
    } else {
        console_kv_hex("  STACKED_PC", frame->pc);
        console_kv_hex("  STACKED_XPSR", frame->xpsr);
        console_kv_hex("  STACKED_R0", frame->r0);
        console_kv_hex("  STACKED_R1", frame->r1);
    }

    console_line("===== FAULT END =====");
    for (;;) {
        __WFI();
    }
}
#endif

/* ------------------------------------------------------------------ */
/* SVC cagrisi: R0-R3 ve R12'ye taninabilir degerler koyup svc #7       */
/* ------------------------------------------------------------------ */
#if !defined(LESSON07_BREAK_MEMMANAGE) && !defined(LESSON07_BREAK_ESCALATION)
static uint32_t do_marked_svc(void)
{
    register uint32_t r0 __asm__("r0") = 0x11111111u;
    register uint32_t r1 __asm__("r1") = 0x22222222u;
    register uint32_t r2 __asm__("r2") = 0x33333333u;
    register uint32_t r3 __asm__("r3") = 0x44444444u;
    register uint32_t r12 __asm__("r12") = 0xCCCCCCCCu;

    __asm__ volatile ("svc %[imm]"
                      : "+r" (r0)
                      : [imm] "I" (SVC_ECHO),
                        "r" (r1), "r" (r2), "r" (r3), "r" (r12)
                      : "memory");
    return r0;
}


/* ------------------------------------------------------------------ */
/* Exercise 01: exception girisini coz                                 */
/* ------------------------------------------------------------------ */
typedef struct {
    const char *interrupted_mode;   /* "Thread" veya "Handler"        */
    const char *frame_stack;        /* "PSP" veya "MSP"               */
    uint32_t    exception_number;   /* IPSR'den                       */
    uint32_t    stacked_r0;
    uint32_t    stacked_pc;
    uint32_t    stacked_xpsr;
} entry_info_t;

static entry_info_t g_info;

static void decode_exception_entry(const exception_frame_t *frame,
                                   uint32_t exc_return,
                                   uint32_t ipsr,
                                   entry_info_t *out)
{
    /* ------------------------------------------------------------------
     * GOREV (Exercise 01)
     *
     * Asagidaki alti alani doldur:
     *
     *   out->interrupted_mode   "Thread" / "Handler"
     *       Ipucu: EXC_RETURN'un bit3'u NEREYE donulecegini soyler.
     *
     *   out->frame_stack        "PSP" / "MSP"
     *       Ipucu: EXC_RETURN'un bit2'si frame'in hangi stack'te
     *              oldugunu soyler. Bu, "handler hangi stack'i
     *              kullaniyor" sorusuyla AYNI SEY DEGILDIR.
     *
     *   out->exception_number   hangi exception calisiyor?
     *       Ipucu: ipsr parametresi.
     *
     *   out->stacked_r0         frame'in R0 yuvasi
     *   out->stacked_pc         frame'in PC yuvasi
     *   out->stacked_xpsr       frame'in xPSR yuvasi
     *
     * KAPSAM: yalnizca bu repo'nun destekledigi alt kume - standart
     * 8 word frame, tek guvenlik durumu. Her EXC_RETURN kuralini
     * yeniden uygulamani istemiyoruz.
     *
     * Su an alanlar bos/varsayilan; CHECK satirlari FAIL veriyor.
     * Bu BEKLENEN baslangic durumudur.
     *
     * Referans cozum: solutions/01-decode-exception-entry/
     * ------------------------------------------------------------------ */
    (void)frame;
    (void)exc_return;
    (void)ipsr;

    out->interrupted_mode = "?";
    out->frame_stack      = "?";
    out->exception_number = 0u;
    out->stacked_r0       = 0u;
    out->stacked_pc       = 0u;
    out->stacked_xpsr     = 0u;
}

/* ------------------------------------------------------------------ */
static void report_frame(void)
{
    console_line("--- yakalanan exception frame ---");
    console_kv_hex("  FRAME_ADDR", g_frame_addr);
    console_kv_hex("  +0x00 R0", g_saved_frame.r0);
    console_kv_hex("  +0x04 R1", g_saved_frame.r1);
    console_kv_hex("  +0x08 R2", g_saved_frame.r2);
    console_kv_hex("  +0x0C R3", g_saved_frame.r3);
    console_kv_hex("  +0x10 R12", g_saved_frame.r12);
    console_kv_hex("  +0x14 LR", g_saved_frame.lr);
    console_kv_hex("  +0x18 PC", g_saved_frame.pc);
    console_kv_hex("  +0x1C xPSR", g_saved_frame.xpsr);
    console_kv_u32("  xPSR_IPSR_FIELD", g_saved_frame.xpsr & 0x1FFu);
    console_kv_str("  xPSR_T_BIT",
                   ((g_saved_frame.xpsr & 0x01000000u) != 0u) ? "1" : "0");
    console_kv_str("  xPSR_ALIGN_BIT9",
                   ((g_saved_frame.xpsr & 0x00000200u) != 0u) ? "1" : "0");
}
#endif  /* !BREAK_* */

/* ------------------------------------------------------------------ */
/* Thread baglami                                                      */
/* ------------------------------------------------------------------ */
static void __attribute__((noreturn)) thread_entry(void)
{
    console_line("[1] Exception'dan ONCE (Thread Mode)");
    console_kv_u32("  IPSR", __get_IPSR());
    console_kv_hex("  CONTROL", __get_CONTROL());
    console_kv_hex("  MSP", __get_MSP());
    console_kv_hex("  PSP", __get_PSP());
    volatile uint32_t here = 0u;
    console_kv_hex("  ACTIVE_SP_APPROX", (uint32_t)&here);
    console_kv_str("  ACTIVE_STACK",
                   (__get_CONTROL() & 0x2u) ? "PSP" : "MSP");

#if defined(LESSON07_BREAK_MEMMANAGE)
    console_line("--- BREAK IT: MPU ihlali, MemManage ETKIN ---");
    mpu_setup_readonly_region(1);
    mpu_store_word(&g_ro_block[0], 0xDEAD0001u);
    console_line("  buraya ULASILMAMALI");
#elif defined(LESSON07_BREAK_ESCALATION)
    console_line("--- BREAK IT: ayni ihlal, MemManage DEVRE DISI ---");
    mpu_setup_readonly_region(0);
    mpu_store_word(&g_ro_block[0], 0xDEAD0001u);
    console_line("  buraya ULASILMAMALI");
#else
    console_line("--- svc #7 calistiriliyor ---");
    uint32_t result = do_marked_svc();

    console_line("[2] Exception'dan SONRA (Thread Mode)");
    console_kv_u32("  IPSR", __get_IPSR());
    console_kv_hex("  PSP", __get_PSP());
    console_kv_u32("  SVC_RESULT", result);
    console_kv_str("  CHECK_THREAD_RESUMED", (__get_IPSR() == 0u) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_SVC_RESULT",
                   (result == 0x11111112u) ? "PASS" : "FAIL");

    console_line("[3] Handler giriste ne gordu?");
    console_kv_u32("  HANDLER_IPSR", g_saved_ipsr);
    console_kv_hex("  ENTRY_EXC_RETURN", g_entry_lr);
    console_kv_hex("  ENTRY_MSP", g_entry_msp);
    console_kv_hex("  ENTRY_PSP", g_entry_psp);
    console_kv_hex("  DISPATCHER_SP", g_dispatcher_sp);
    console_kv_str("  CHECK_IPSR_IS_SVCALL", (g_saved_ipsr == 11u) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_HANDLER_USED_MSP",
                   (g_dispatcher_sp < g_entry_msp &&
                    g_dispatcher_sp > (g_entry_msp - 0x200u)) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_ACTIVE_SP_NOT_FRAME",
                   (g_dispatcher_sp != g_frame_addr) ? "PASS" : "FAIL");

    report_frame();
    console_kv_str("  CHECK_FRAME_MATCHES_MARKERS",
                   (g_saved_frame.r0 == 0x11111111u &&
                    g_saved_frame.r1 == 0x22222222u &&
                    g_saved_frame.r2 == 0x33333333u &&
                    g_saved_frame.r3 == 0x44444444u &&
                    g_saved_frame.r12 == 0xCCCCCCCCu) ? "PASS" : "FAIL");

#ifdef LESSON07_MSP_ORIGIN
    console_kv_str("  CHECK_FRAME_ON_MSP",
                   (g_frame_addr == g_entry_msp) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_EXC_RETURN_MSP",
                   (g_entry_lr == 0xFFFFFFF9u) ? "PASS" : "FAIL");
#else
    console_kv_str("  CHECK_FRAME_ON_PSP",
                   (g_frame_addr == g_entry_psp) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_EXC_RETURN_PSP",
                   (g_entry_lr == 0xFFFFFFFDu) ? "PASS" : "FAIL");
#endif


    console_line("[4] Exception girisi cozumu");
    decode_exception_entry((const exception_frame_t *)&g_saved_frame,
                           g_entry_lr, g_saved_ipsr, &g_info);
    console_kv_str("  INTERRUPTED_MODE", g_info.interrupted_mode);
    console_kv_str("  FRAME_STACK", g_info.frame_stack);
    console_kv_u32("  EXCEPTION_NUMBER", g_info.exception_number);
    console_kv_hex("  STACKED_R0", g_info.stacked_r0);
    console_kv_hex("  STACKED_PC", g_info.stacked_pc);
    console_kv_hex("  STACKED_XPSR", g_info.stacked_xpsr);

    console_kv_str("  CHECK_DECODE_MODE",
                   (g_info.interrupted_mode[0] == 'T') ? "PASS" : "FAIL");
    console_kv_str("  CHECK_DECODE_STACK",
                   (g_info.frame_stack[0] == 'P') ? "PASS" : "FAIL");
    console_kv_str("  CHECK_DECODE_EXCNUM",
                   (g_info.exception_number == 11u) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_DECODE_R0",
                   (g_info.stacked_r0 == 0x11111111u) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_DECODE_PC",
                   (g_info.stacked_pc == g_saved_frame.pc) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_DECODE_XPSR",
                   (g_info.stacked_xpsr == g_saved_frame.xpsr) ? "PASS" : "FAIL");

    console_line("===== END =====");
#endif

    for (;;) {
        __WFI();
    }
}

/* ------------------------------------------------------------------ */
static void report_vectors(void)
{
    const uint32_t *vt = (const uint32_t *)SCB->VTOR;
    console_line("--- vector table ---");
    console_kv_hex("  VTOR", SCB->VTOR);
    console_kv_hex("  [0] initial MSP", vt[0]);
    console_kv_hex("  [1] Reset", vt[1]);
    console_kv_hex("  [3] HardFault", vt[3]);
    console_kv_hex("  [4] MemManage", vt[4]);
    console_kv_hex("  [11] SVCall", vt[11]);
    console_kv_hex("  [14] PendSV", vt[14]);
    console_kv_str("  CHECK_VEC0_IS_NOT_ADDRESS",
                   ((vt[0] & 1u) == 0u) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_SVCALL_THUMB_BIT",
                   ((vt[11] & 1u) != 0u) ? "PASS" : "FAIL");
}

int main(void)
{
    console_line("");
    console_line("===== LESSON 07 EXERCISE 01 =====");

    console_kv_hex("MSP_AT_START", __get_MSP());
    report_vectors();

#ifdef LESSON07_MSP_ORIGIN
    /* Deney: Thread Mode MSP uzerinde kalsin. Stack degistirmiyoruz;
     * main zaten MSP uzerinde calisiyor, dogrudan devam ediyoruz. */
    console_line("--- Thread Mode: MSP (psp_start CAGRILMADI) ---");
    thread_entry();
#else
    /* Ders 04'te dogrulanan gecis: assembly siniri + taze C girisi. */
    psp_start(TASK_STACK_TOP, thread_entry);
#endif
}
