/*
 * Lesson 08 - Faults: MemManage, BusFault, UsageFault & HardFault
 *
 * Tek bir kaynak, derleme zamani secilen DORT senaryo:
 *
 *   LESSON08_CASE_MEMMANAGE   ayricaliksiz STR -> korunan veri (ders 06)
 *   LESSON08_CASE_USAGE       UDF komutu, UsageFault ETKIN
 *   LESSON08_CASE_BUS         eslenmemis adrese LDR, BusFault ETKIN
 *   LESSON08_CASE_ESCALATION  ayni UDF, UsageFault DEVRE DISI -> HardFault
 *
 * Hepsi ayni fault kaydediciyi kullanir. Rapor iki bolumludur:
 *   RAW     register'larin kendisi   (gerceklik)
 *   DECODE  onlardan cikan anlam     (yorum)
 *
 * Build/run: lessons/08-faults/README.md
 */

#include "platform/mps2_an505.h"
#include "common/console.h"

/* ------------------------------------------------------------------ */
/* Handler sarmalayicilarinin giriste kaydettikleri                    */
/* ------------------------------------------------------------------ */
volatile uint32_t g_entry_lr;
volatile uint32_t g_entry_msp;
volatile uint32_t g_entry_psp;

#include "fault_snapshot.h"

static fault_snapshot_t g_snap;

/* ------------------------------------------------------------------ */
/* Tetikleyiciler (example/fault_triggers.S)                           */
/* ------------------------------------------------------------------ */
void     trigger_udf(void);
uint32_t trigger_load(const volatile uint32_t *addr);
void     mpu_store_word(volatile uint32_t *addr, uint32_t value);

void psp_start(uint32_t psp_top, void (*entry)(void));   /* DONMEZ */
void fault_record(exception_frame_t *frame, uint32_t exc_return);

/* ------------------------------------------------------------------ */
/* Senaryo ortami                                                      */
/* ------------------------------------------------------------------ */
#define TASK_STACK_WORDS   192u

#ifdef LESSON08_CASE_MEMMANAGE
/* Ders 06'daki duzen: ayricaliksiz Thread'in dokunabilecegi her sey tek
 * bir 32 byte hizali blokta; korunan sayac bu blogun DISINDA. */
typedef struct {
    uint32_t stack[TASK_STACK_WORDS];
    uint32_t pad[8];
} unpriv_block_t;
static unpriv_block_t g_unpriv __attribute__((aligned(32)));
static volatile uint32_t g_service_counter;
#define TASK_STACK_TOP ((uint32_t)&g_unpriv.stack[TASK_STACK_WORDS])
#else
static uint32_t task_stack[TASK_STACK_WORDS] __attribute__((aligned(8)));
#define TASK_STACK_TOP ((uint32_t)&task_stack[TASK_STACK_WORDS])
#endif

/* BusFault hedefi: QEMU mps2-an505 bellek haritasinda Secure SRAM
 * 0x30000000..0x30007FFF'dir; 0x30100000 hicbir bolgeye ESLENMEMISTIR
 * ('info mtree -f' ile dogrulandi). Bu bir TARGET CONFIG gercegidir. */
#define BUS_FAULT_ADDR  0x30100000u

/* ------------------------------------------------------------------ */
/* Durum register'larini temizleme (W1C)                               */
/*                                                                     */
/* CFSR ve HFSR "write-one-to-clear"dir: bir biti temizlemek icin o    */
/* bite 1 YAZILIR. 0 yazmak hicbir seyi temizlemez. Her hedef tek bir  */
/* fault urettigi ve reset'ten basladigi icin burada temizlik yalnizca */
/* ilkeyi gostermek icin - onceki bir calismadan kalan bit YOK.        */
/* ------------------------------------------------------------------ */
static void clear_fault_status(void)
{
    SCB->CFSR = SCB->CFSR;     /* set olan bitlere 1 yaz -> temizle */
    SCB->HFSR = SCB->HFSR;
}

/* ------------------------------------------------------------------ */
/* Kaydedici. Sarmalayicidan DALLANILIR; donmez.                       */
/* ------------------------------------------------------------------ */
void fault_record(exception_frame_t *frame, uint32_t exc_return)
{
    /* === 1) ONCE YAKALA ========================================== */
    g_snap.ipsr          = __get_IPSR();
    g_snap.exc_return    = exc_return;
    g_snap.control       = __get_CONTROL();
    g_snap.msp           = g_entry_msp;
    g_snap.psp           = g_entry_psp;
    g_snap.cfsr          = SCB->CFSR;
    g_snap.hfsr          = SCB->HFSR;
    g_snap.shcsr         = SCB->SHCSR;
    g_snap.mmfar_raw     = SCB->MMFAR;
    g_snap.bfar_raw      = SCB->BFAR;
    g_snap.frame_address = (uint32_t)frame;

    /* Frame yalnizca yigma BASARILIYSA ve bicimi desteklenen bicimse
     * gecerlidir. Yigma hatasi bitleri: MSTKERR / STKERR / STKOF. */
    int stacking_failed =
        ((g_snap.cfsr & SCB_CFSR_MSTKERR_Msk) != 0u) ||
        ((g_snap.cfsr & SCB_CFSR_STKERR_Msk)  != 0u) ||
        ((g_snap.cfsr & SCB_CFSR_STKOF_Msk)   != 0u);
    int basic_frame = ((exc_return & 0x10u) != 0u);   /* FType = 1 */

    g_snap.frame_valid = (!stacking_failed) && basic_frame;
    if (g_snap.frame_valid) {
        g_snap.frame = *frame;
    }

    /* === 2) SONRA RAPORLA ======================================== */
    extern void fault_report(const fault_snapshot_t *s);
    fault_report(&g_snap);

    for (;;) {
        __WFI();
    }
}

/* ------------------------------------------------------------------ */
/* Yorumlama yardimcilari                                              */
/* ------------------------------------------------------------------ */
static const char *exception_name(uint32_t ipsr)
{
    switch (ipsr) {
    case 3u:  return "HardFault";
    case 4u:  return "MemManage";
    case 5u:  return "BusFault";
    case 6u:  return "UsageFault";
    default:  return "diger";
    }
}

static void print_bit(const char *name, uint32_t reg, uint32_t mask)
{
    if ((reg & mask) != 0u) {
        console_kv_str(name, "1");
    }
}

void fault_report(const fault_snapshot_t *s);

void fault_report(const fault_snapshot_t *s)
{
    console_line("");
    console_line("===== FAULT SNAPSHOT =====");
    console_line("--- RAW ---");
    console_kv_u32("  IPSR", s->ipsr);
    console_kv_hex("  EXC_RETURN", s->exc_return);
    console_kv_hex("  CONTROL", s->control);
    console_kv_hex("  MSP", s->msp);
    console_kv_hex("  PSP", s->psp);
    console_kv_hex("  CFSR", s->cfsr);
    console_kv_hex("  HFSR", s->hfsr);
    console_kv_hex("  SHCSR", s->shcsr);
    console_kv_hex("  MMFAR_RAW", s->mmfar_raw);
    console_kv_hex("  BFAR_RAW", s->bfar_raw);
    console_kv_hex("  FRAME_ADDR", s->frame_address);

    console_line("--- DECODE ---");
    console_kv_str("  ACTIVE_EXCEPTION", exception_name(s->ipsr));
    console_kv_str("  ESCALATED",
                   ((s->hfsr & SCB_HFSR_FORCED_Msk) != 0u) ? "YES" : "NO");
    console_kv_hex("  MMFSR", (s->cfsr >> SCB_CFSR_MEMFAULTSR_Pos) & 0xFFu);
    console_kv_hex("  BFSR",  (s->cfsr >> SCB_CFSR_BUSFAULTSR_Pos) & 0xFFu);
    console_kv_hex("  UFSR",  (s->cfsr >> SCB_CFSR_USGFAULTSR_Pos) & 0xFFFFu);

    /* Set olan bitleri isimleriyle yaz (yalnizca bu dersin kapsami). */
    print_bit("  MMFSR.IACCVIOL",  s->cfsr, SCB_CFSR_IACCVIOL_Msk);
    print_bit("  MMFSR.DACCVIOL",  s->cfsr, SCB_CFSR_DACCVIOL_Msk);
    print_bit("  MMFSR.MUNSTKERR", s->cfsr, SCB_CFSR_MUNSTKERR_Msk);
    print_bit("  MMFSR.MSTKERR",   s->cfsr, SCB_CFSR_MSTKERR_Msk);
    print_bit("  MMFSR.MMARVALID", s->cfsr, SCB_CFSR_MMARVALID_Msk);
    print_bit("  BFSR.IBUSERR",    s->cfsr, SCB_CFSR_IBUSERR_Msk);
    print_bit("  BFSR.PRECISERR",  s->cfsr, SCB_CFSR_PRECISERR_Msk);
    print_bit("  BFSR.IMPRECISERR",s->cfsr, SCB_CFSR_IMPRECISERR_Msk);
    print_bit("  BFSR.UNSTKERR",   s->cfsr, SCB_CFSR_UNSTKERR_Msk);
    print_bit("  BFSR.STKERR",     s->cfsr, SCB_CFSR_STKERR_Msk);
    print_bit("  BFSR.BFARVALID",  s->cfsr, SCB_CFSR_BFARVALID_Msk);
    print_bit("  UFSR.UNDEFINSTR", s->cfsr, SCB_CFSR_UNDEFINSTR_Msk);
    print_bit("  UFSR.INVSTATE",   s->cfsr, SCB_CFSR_INVSTATE_Msk);
    print_bit("  UFSR.INVPC",      s->cfsr, SCB_CFSR_INVPC_Msk);
    print_bit("  UFSR.NOCP",       s->cfsr, SCB_CFSR_NOCP_Msk);
    print_bit("  UFSR.STKOF",      s->cfsr, SCB_CFSR_STKOF_Msk);
    print_bit("  UFSR.UNALIGNED",  s->cfsr, SCB_CFSR_UNALIGNED_Msk);
    print_bit("  UFSR.DIVBYZERO",  s->cfsr, SCB_CFSR_DIVBYZERO_Msk);

    /* Adres register'i + gecerlilik biti = kullanilabilir kanit. */
    if ((s->cfsr & SCB_CFSR_MMARVALID_Msk) != 0u) {
        console_kv_hex("  FAULT_ADDRESS(MMFAR)", s->mmfar_raw);
    } else {
        console_kv_str("  MMFAR", "GECERSIZ - kanit degil");
    }
    if ((s->cfsr & SCB_CFSR_BFARVALID_Msk) != 0u) {
        console_kv_hex("  FAULT_ADDRESS(BFAR)", s->bfar_raw);
    } else {
        console_kv_str("  BFAR", "GECERSIZ - kanit degil");
    }

    console_kv_str("  FRAME_STACK", (s->exc_return & 0x4u) ? "PSP" : "MSP");
    console_kv_str("  FRAME_VALID", s->frame_valid ? "YES" : "NO");
    if (s->frame_valid) {
        console_kv_hex("  STACKED_R0", s->frame.r0);
        console_kv_hex("  STACKED_R1", s->frame.r1);
        console_kv_hex("  STACKED_LR", s->frame.lr);
        console_kv_hex("  STACKED_PC", s->frame.pc);
        console_kv_hex("  STACKED_XPSR", s->frame.xpsr);
    } else {
        console_line("  FRAME: gecersiz veya desteklenmiyor -> COZULMEDI");
    }

#ifdef LESSON08_HAS_DECODE
    /* Exercise 01: ham snapshot -> kararlar */
    fault_decision_t d;
    decode_fault_snapshot(s, &d);
    console_line("--- DECISION (decode_fault_snapshot) ---");
    console_kv_str("  D_ACTIVE_EXCEPTION", d.active_exception);
    console_kv_str("  D_ESCALATED", d.escalated ? "YES" : "NO");
    console_kv_str("  D_FAULT_CLASS", d.fault_class);
    console_kv_str("  D_PRIMARY_BIT", d.primary_bit);
    console_kv_str("  D_ADDRESS_VALID", d.address_valid ? "YES" : "NO");
    console_kv_hex("  D_FAULT_ADDRESS", d.fault_address);
    console_kv_str("  D_FRAME_SAFE", d.frame_safe ? "YES" : "NO");
    console_kv_hex("  D_STACKED_PC", d.stacked_pc);

    /* Referans karsilastirma - bu hedefte MemManage DACCVIOL beklenir */
    console_kv_str("  CHECK_D_ACTIVE",
        (d.active_exception != 0 && d.active_exception[0] == 'M') ? "PASS" : "FAIL");
    console_kv_str("  CHECK_D_NOT_ESCALATED", (d.escalated == 0 && d.active_exception != 0) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_D_CLASS",
        (d.fault_class != 0 && d.fault_class[0] == 'M') ? "PASS" : "FAIL");
    console_kv_str("  CHECK_D_BIT",
        (d.primary_bit != 0 && d.primary_bit[0] == 'D') ? "PASS" : "FAIL");
    console_kv_str("  CHECK_D_ADDRESS",
        (d.address_valid && d.fault_address == s->mmfar_raw) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_D_FRAME",
        (d.frame_safe && d.stacked_pc == s->frame.pc) ? "PASS" : "FAIL");
#endif

    console_line("===== FAULT END =====");
}

/* ------------------------------------------------------------------ */
/* MPU politikasi (yalnizca MemManage senaryosu - ders 06 ile ayni)    */
/* ------------------------------------------------------------------ */
#ifdef LESSON08_CASE_MEMMANAGE
static void mpu_configure(void)
{
    ARM_MPU_Disable();
    ARM_MPU_SetMemAttr(0u,
        ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1u, 1u, 1u, 1u),
                     ARM_MPU_ATTR_MEMORY_(1u, 1u, 1u, 1u)));
    ARM_MPU_SetRegion(0u,
        ARM_MPU_RBAR(0x10000000u, ARM_MPU_SH_NON, 1u, 1u, 0u),
        ARM_MPU_RLAR(0x1007FFFFu, 0u));
    ARM_MPU_SetRegion(1u,
        ARM_MPU_RBAR((uint32_t)&g_unpriv, ARM_MPU_SH_NON, 0u, 1u, 1u),
        ARM_MPU_RLAR((uint32_t)&g_unpriv + sizeof(g_unpriv) - 1u, 0u));
    /* ARM_MPU_Enable() MEMFAULTENA'yi KENDISI set eder (ders 07). */
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
}
#endif

/* ------------------------------------------------------------------ */
/* Thread (PSP uzerinde)                                               */
/* ------------------------------------------------------------------ */
static void __attribute__((noreturn)) thread_entry(void)
{
#if defined(LESSON08_CASE_MEMMANAGE)
    /* Ayricaliksiz ol, sonra korunan veriye DOGRUDAN yaz. */
    __set_CONTROL(__get_CONTROL() | 0x1u);
    __ISB();
    mpu_store_word(&g_service_counter, 0xBADC0DEu);

#elif defined(LESSON08_CASE_USAGE)
    console_line("--- CASE: UDF, UsageFault ETKIN ---");
    trigger_udf();

#elif defined(LESSON08_CASE_BUS)
    console_line("--- CASE: eslenmemis adrese LDR, BusFault ETKIN ---");
    console_kv_hex("  TARGET_ADDR", BUS_FAULT_ADDR);
    (void)trigger_load((const volatile uint32_t *)BUS_FAULT_ADDR);

#elif defined(LESSON08_CASE_ESCALATION)
    console_line("--- CASE: ayni UDF, UsageFault DEVRE DISI ---");
    trigger_udf();
#endif

    /* Hicbir senaryoda buraya ULASILMAMALI. */
    console_line("===== END =====");
    for (;;) {
        __WFI();
    }
}

/* ------------------------------------------------------------------ */
int main(void)
{
    console_line("");
    console_line("===== LESSON 08: FAULTS =====");

    clear_fault_status();

#if defined(LESSON08_CASE_MEMMANAGE)
    console_line("--- CASE: ayricaliksiz STR -> korunan veri, MemManage ETKIN ---");
    console_kv_hex("  PROTECTED_ADDR", (uint32_t)&g_service_counter);
    mpu_configure();
#elif defined(LESSON08_CASE_USAGE)
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;
#elif defined(LESSON08_CASE_BUS)
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
#elif defined(LESSON08_CASE_ESCALATION)
    SCB->SHCSR &= ~SCB_SHCSR_USGFAULTENA_Msk;
#endif
    __DSB();
    __ISB();

    console_kv_hex("SHCSR_BEFORE_FAULT", SCB->SHCSR);

    psp_start(TASK_STACK_TOP, thread_entry);   /* donmez */
}
