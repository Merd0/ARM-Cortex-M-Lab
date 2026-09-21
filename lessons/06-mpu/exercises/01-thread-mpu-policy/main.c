/*
 * Lesson 06 EXERCISE 01 - Build the Thread MPU Policy
 *
 * Ders 05'te sunu kurduk:
 *     Unprivileged Thread -> SVC -> Privileged Handler
 * ama g_service_counter hala siradan RAM'deydi: "SVC yolundan gec" bir
 * YAZILIM SOZLESMESIYDI.
 *
 * Bu derste ayni sozlesmeyi DONANIM uyguluyor:
 *     MPU politikasi -> izinli bolgeler -> korunan kernel verisi
 *     -> izinsiz dogrudan erisim -> MemManage fault
 *
 * DIKKAT - iki ayri mekanizma:
 *   Ders 05'teki UART engeli  = SoC Peripheral Protection Controller (PPC)
 *   Bu dersteki RAM engeli    = Cortex-M MPU
 * Ayni sey degiller. Bkz. README, "PPC vs MPU".
 *
 * Build/run: lessons/06-mpu/README.md
 */

#include "platform/mps2_an505.h"      /* CMSIS-Core + Armv8-M MPU API */
#include "common/console.h"

/* ------------------------------------------------------------------ */
/* Bellek yerlesimi                                                    */
/*                                                                     */
/* Ayricaliksiz Thread'in dokunabilecegi HER SEY tek bir blokta:       */
/* kendi stack'i ve kendi veri alani. Blok 32 byte hizali ve boyutu    */
/* 32'nin kati - cunku Armv8-M MPU bolge sinirlari 32 byte granularite */
/* ile ifade edilir (RBAR/RLAR bit [31:5]).                            */
/*                                                                     */
/* g_service_counter bu blogun DISINDA. Ayricaliksiz koda acilan       */
/* hicbir bolge onu kapsamiyor -> erisim MemManage uretir.             */
/* ------------------------------------------------------------------ */
#define UNPRIV_STACK_WORDS   192u

typedef struct {
    uint32_t control_unpriv;
    uint32_t ipsr_unpriv;
    uint32_t svc_first;
    uint32_t svc_second;
    uint32_t direct_write_attempted;
    uint32_t stack_write_magic;      /* ayricaliksiz yazma kaniti */
    uint32_t counter_after_direct;   /* dogrudan yazma sonrasi sayac */
    uint32_t pad[1];                 /* blogu 32 byte katina tamamlar */
} unpriv_obs_t;

typedef struct {
    uint32_t     stack[UNPRIV_STACK_WORDS];   /* asagi dogru buyur */
    unpriv_obs_t obs;                          /* stack'in hemen ustunde */
} unpriv_block_t;

static unpriv_block_t g_unpriv __attribute__((aligned(32)));

#define UNPRIV_BASE   ((uint32_t)&g_unpriv)
#define UNPRIV_LIMIT  ((uint32_t)&g_unpriv + sizeof(g_unpriv) - 1u)  /* DAHIL */
#define UNPRIV_STACK_TOP ((uint32_t)&g_unpriv.stack[UNPRIV_STACK_WORDS])

/* Korunan kernel verisi. Ayricaliksiz koda acik bir bolgede DEGIL. */
static volatile uint32_t g_service_counter;

/* FLASH (Secure code SRAM) araligi - linker script ile ayni. */
#define FLASH_BASE_ADDR   0x10000000u
#define FLASH_LIMIT_ADDR  (0x10000000u + 0x80000u - 1u)

/* MAIR indeksleri */
#define ATTR_IDX_NORMAL   0u

void psp_start(uint32_t psp_top, void (*entry)(void));      /* DONMEZ */
void mpu_store_word(volatile uint32_t *addr, uint32_t value); /* mpu_probe.S */

/* ------------------------------------------------------------------ */
/* SVC servisleri                                                      */
/* ------------------------------------------------------------------ */
#define SVC_INCREMENT        1u
#define SVC_READ_COUNTER     2u
#define SVC_RESTORE_PRIV     3u
#define SVC_ERROR_UNKNOWN    0xFFFFFFFFu

typedef struct {
    uint32_t r0, r1, r2, r3, r12, lr, pc, xpsr;
} exception_frame_t;

void svc_dispatch(exception_frame_t *frame, uint32_t exc_return);

void svc_dispatch(exception_frame_t *frame, uint32_t exc_return)
{
    /* SVC T1 kodlamasi 16 bittir (0xDFxx); komut yigilmis PC'nin 2 byte
     * gerisindedir. Bu GENEL bir kural degildir - ders 05'te aciklandi. */
    const uint8_t *svc_instr = (const uint8_t *)(frame->pc - 2u);
    uint32_t svc_number = (uint32_t)svc_instr[0];
    uint32_t result;

    switch (svc_number) {
    case SVC_INCREMENT:
        /* Handler ayricalikli calisir. PRIVDEFENA = 1 oldugu icin
         * ayricalikli erisimler varsayilan bellek haritasini kullanir;
         * bu yuzden g_service_counter buradan erisilebilir. */
        g_service_counter++;
        result = g_service_counter;
        break;

    case SVC_READ_COUNTER:
        result = g_service_counter;
        break;

    case SVC_RESTORE_PRIV:
        /* Ayricalikli HANDLER, donecek Thread baglaminin nPRIV bitini
         * temizliyor (ders 05). Thread kendini yukseltmiyor. */
        __set_CONTROL(__get_CONTROL() & ~0x1u);
        __ISB();
        result = 0u;
        break;

    default:
        console_kv_u32("  UNKNOWN_SVC", svc_number);
        result = SVC_ERROR_UNKNOWN;
        break;
    }

    frame->r0 = result;
    (void)exc_return;
}

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
/* MPU politikasi                                                      */
/*                                                                     */
/* CMSIS'in resmi Armv8-M MPU API'si kullaniliyor (ARM_MPU_RBAR /      */
/* ARM_MPU_RLAR / ARM_MPU_SetMemAttr / ARM_MPU_Enable). Kendi          */
/* soyutlamamizi yazmiyoruz: bu API zaten register alanlarini birebir  */
/* yansitiyor ve uretilen degerleri GDB'de ham register olarak da      */
/* dogrulayacagiz.                                                     */
/* ------------------------------------------------------------------ */
static void mpu_configure(void)
{
    ARM_MPU_Disable();

    /* MAIR[0]: Normal bellek, Outer + Inner Write-Back, non-transient,
     * read/write allocate. Bu kisim HAZIR - MPU yalnizca "kim erisebilir"
     * degil, "bu bellek ne tur bellek" sorusunu da cevaplar. */
    ARM_MPU_SetMemAttr(ATTR_IDX_NORMAL,
                       ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1u, 1u, 1u, 1u),
                                    ARM_MPU_ATTR_MEMORY_(1u, 1u, 1u, 1u)));

    /* ------------------------------------------------------------------
     * GOREV (Exercise 01)
     *
     * Asagidaki dort adimi tamamla:
     *
     *  1. Region 0 - FLASH
     *     Ayricaliksiz kod BURADAN KOD CALISTIRABILMELI ve sabitleri
     *     okuyabilmeli, ama YAZAMAMALI.
     *     Adres araligi: FLASH_BASE_ADDR .. FLASH_LIMIT_ADDR
     *
     *  2. Region 1 - ayricaliksiz Thread blogu (stack + kendi verisi)
     *     Okuma + yazma serbest, ama BURADAN KOD CALISTIRILAMAMALI.
     *     Adres araligi: UNPRIV_BASE .. UNPRIV_LIMIT
     *
     *  3. MemManage exception'i etkinlestir (SCB->SHCSR).
     *     Etkinlestirmezsen ihlal HardFault'a yukseltilir ve
     *     MMFAR/MMFSR okumak zorlasir.
     *
     *  4. MPU'yu, ayricalikli erisimler varsayilan bellek haritasini
     *     kullanacak sekilde ENABLE et.
     *
     * g_service_counter icin BILEREK bolge TANIMLAMA. Korumanin kaynagi
     * bir bolge eklememek: ayricaliksiz erisim hicbir bolgeyle
     * eslesmezse reddedilir.
     *
     * KULLANABILECEGIN CMSIS API'si (external/CMSIS, resmi):
     *   ARM_MPU_SetRegion(rnr, rbar, rlar)
     *   ARM_MPU_RBAR(BASE, SH, RO, NP, XN)
     *   ARM_MPU_RLAR(LIMIT, AttrIndx)       <- LIMIT bolgenin SON byte'i
     *   ARM_MPU_SH_NON
     *   ARM_MPU_Enable(control)
     *   MPU_CTRL_PRIVDEFENA_Msk
     *   SCB_SHCSR_MEMFAULTENA_Msk
     *
     * DIKKAT: Armv8-M MPU base+limit modelini kullanir. Armv7-M'in
     * "region size" alani BURADA YOKTUR; internette bulacagin cogu
     * ornek Armv7-M icindir ve dogrudan uygulanamaz.
     *
     * Su an hicbir bolge tanimlanmadi ve MPU enable edilmedi; firmware
     * calisir ama koruma YOKTUR. Bu BEKLENEN baslangic durumudur.
     *
     * Referans cozum: solutions/01-thread-mpu-policy/
     * ------------------------------------------------------------------ */
}

/* ------------------------------------------------------------------ */
/* Politikanin gercekten programlandigini register'lardan dogrula.     */
/* ------------------------------------------------------------------ */
static void check_region_policy(void)
{
    MPU->RNR = 0u;
    uint32_t rbar0 = MPU->RBAR;
    uint32_t rlar0 = MPU->RLAR;
    MPU->RNR = 1u;
    uint32_t rbar1 = MPU->RBAR;
    uint32_t rlar1 = MPU->RLAR;

    /* Region 0: FLASH tabani, ayricaliksiz (AP bit0 = NP = 1),
     * salt-okunur (AP bit1 = RO = 1), calistirilabilir (XN = 0), etkin. */
    int flash_ok =
        ((rbar0 & MPU_RBAR_BASE_Msk) == FLASH_BASE_ADDR) &&
        (((rbar0 & MPU_RBAR_AP_Msk) >> MPU_RBAR_AP_Pos) == 0x3u) &&
        ((rbar0 & MPU_RBAR_XN_Msk) == 0u) &&
        ((rlar0 & MPU_RLAR_EN_Msk) != 0u);

    /* Region 1: ayricaliksiz blok, RW (AP = 01), XN = 1, etkin. */
    int stack_ok =
        ((rbar1 & MPU_RBAR_BASE_Msk) == (UNPRIV_BASE & MPU_RBAR_BASE_Msk)) &&
        (((rbar1 & MPU_RBAR_AP_Msk) >> MPU_RBAR_AP_Pos) == 0x1u) &&
        ((rbar1 & MPU_RBAR_XN_Msk) != 0u) &&
        ((rlar1 & MPU_RLAR_EN_Msk) != 0u);

    console_kv_str("  CHECK_FLASH_REGION_RO_EXEC", flash_ok ? "PASS" : "FAIL");
    console_kv_str("  CHECK_STACK_REGION_RW_XN", stack_ok ? "PASS" : "FAIL");
}

static void report_mpu(void)
{
    console_line("--- MPU yapilandirmasi ---");
    console_kv_hex("  MPU_TYPE", MPU->TYPE);
    console_kv_u32("  MPU_REGIONS", (MPU->TYPE >> 8) & 0xFFu);
    console_kv_hex("  MPU_CTRL", MPU->CTRL);
    console_kv_hex("  MPU_MAIR0", MPU->MAIR0);

    for (uint32_t r = 0u; r < 2u; r++) {
        MPU->RNR = r;
        console_kv_u32("  REGION", r);
        console_kv_hex("    RBAR", MPU->RBAR);
        console_kv_hex("    RLAR", MPU->RLAR);
        console_kv_hex("    BASE", MPU->RBAR & MPU_RBAR_BASE_Msk);
        console_kv_hex("    LIMIT_INCL", (MPU->RLAR & MPU_RLAR_LIMIT_Msk) | 0x1Fu);
    }
    console_kv_hex("  COUNTER_ADDR", (uint32_t)&g_service_counter);
    console_kv_hex("  UNPRIV_BASE", UNPRIV_BASE);
    console_kv_hex("  UNPRIV_LIMIT", UNPRIV_LIMIT);
}

/* ------------------------------------------------------------------ */
/* Ayricaliksiz Thread                                                 */
/*                                                                     */
/* Konsola YAZMIYOR. Sebep ders 05'te olculdu: bu hedefte ayricaliksiz */
/* kod UART'a erisemiyor ve bunun sebebi MPU DEGIL, SoC'nin PPC'si.    */
/* Gozlemler kendi (izinli) veri alanina yazilir, sonra ayricalikli    */
/* baglamdan basilir.                                                  */
/* ------------------------------------------------------------------ */
static void __attribute__((noreturn)) thread_entry(void)
{
    /* Ayricaligi birak (bu satir hala ayricalikli calisiyor). */
    __set_CONTROL(__get_CONTROL() | 0x1u);
    __ISB();

    /* ===== AYRICALIKSIZ THREAD MODE ============================== */

    /* Kendi bolgesi: yazabiliyor mu? (Region 1: RW, NP=1) */
    g_unpriv.obs.control_unpriv    = __get_CONTROL();
    g_unpriv.obs.ipsr_unpriv       = __get_IPSR();
    g_unpriv.obs.stack_write_magic = 0xA5A5C0DEu;   /* Region 1'e yazma */

    /* Korunan veriye SVC uzerinden yasal erisim. */
    g_unpriv.obs.svc_first  = SVC_CALL(SVC_INCREMENT);
    g_unpriv.obs.svc_second = SVC_CALL(SVC_INCREMENT);

#ifdef LESSON06_BREAK_DIRECT_ACCESS
    /* BREAK IT: korunan veriye DOGRUDAN yazma denemesi.
     * Tek bir "str" komutu (example/mpu_probe.S) - hangi komutun hata
     * urettigini tam olarak bilelim diye. */
    g_unpriv.obs.direct_write_attempted = 1u;
    mpu_store_word(&g_service_counter, 0xBADC0DEu);
    /* MPU politikasi sayaci koruyorsa buraya ULASILMAZ. */
    g_unpriv.obs.direct_write_attempted = 2u;
    g_unpriv.obs.counter_after_direct = SVC_CALL(SVC_READ_COUNTER);
#endif

    /* Raporlama konsola yazmayi gerektiriyor; ayricaliksiz kod UART'a
     * erisemiyor (ders 05, PPC). Bu yuzden ayricaligi ayricalikli
     * handler'a geri aldiriyoruz - Thread kendini yukseltmiyor. */
    (void)SVC_CALL(SVC_RESTORE_PRIV);

    /* ===== YENIDEN AYRICALIKLI =================================== */
    console_line("");
    console_line("[3] Ayricaliksiz bolumde kaydedilenler");
    console_kv_hex("  CONTROL_UNPRIV", g_unpriv.obs.control_unpriv);
    console_kv_u32("  IPSR_UNPRIV", g_unpriv.obs.ipsr_unpriv);
    console_kv_str("  CHECK_THREAD_UNPRIV",
                   ((g_unpriv.obs.control_unpriv & 0x1u) != 0u) ? "PASS" : "FAIL");
    console_kv_hex("  STACK_WRITE_MAGIC", g_unpriv.obs.stack_write_magic);
    console_kv_str("  CHECK_UNPRIV_REGION_WRITABLE",
                   (g_unpriv.obs.stack_write_magic == 0xA5A5C0DEu) ? "PASS" : "FAIL");

    console_line("[4] Korunan sayaca SVC uzerinden erisim");
    console_kv_u32("  SVC_INCREMENT_1", g_unpriv.obs.svc_first);
    console_kv_u32("  SVC_INCREMENT_2", g_unpriv.obs.svc_second);
    console_kv_str("  CHECK_SVC_PROTECTED_ACCESS",
                   (g_unpriv.obs.svc_first == 1u && g_unpriv.obs.svc_second == 2u)
                       ? "PASS" : "FAIL");

#ifdef LESSON06_BREAK_DIRECT_ACCESS
    console_kv_u32("  DIRECT_WRITE_MARKER", g_unpriv.obs.direct_write_attempted);
    console_kv_hex("  COUNTER_AFTER_DIRECT", g_unpriv.obs.counter_after_direct);
    console_kv_str("  CHECK_DIRECT_WRITE_SUCCEEDED",
                   (g_unpriv.obs.counter_after_direct == 0x0BADC0DEu)
                       ? "PASS (MPU izin verdi)" : "FAIL");
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
    console_line("===== LESSON 06 EXERCISE 01 =====");

    console_line("[1] MPU yapilandirilmadan once");
    console_kv_hex("  MPU_CTRL", MPU->CTRL);
    console_kv_str("  CHECK_MPU_INITIALLY_DISABLED",
                   ((MPU->CTRL & MPU_CTRL_ENABLE_Msk) == 0u) ? "PASS" : "FAIL");

    mpu_configure();

    console_line("[2] MPU etkin");
    report_mpu();
    console_kv_str("  CHECK_MPU_ENABLED",
                   ((MPU->CTRL & MPU_CTRL_ENABLE_Msk) != 0u) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_PRIVDEFENA",
                   ((MPU->CTRL & MPU_CTRL_PRIVDEFENA_Msk) != 0u) ? "PASS" : "FAIL");
    console_kv_str("  CHECK_MEMFAULT_ENABLED",
                   ((SCB->SHCSR & SCB_SHCSR_MEMFAULTENA_Msk) != 0u) ? "PASS" : "FAIL");
    check_region_policy();

    psp_start(UNPRIV_STACK_TOP, thread_entry);   /* donmez */
}
