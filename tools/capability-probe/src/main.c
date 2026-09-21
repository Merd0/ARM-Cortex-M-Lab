/*
 * Target Capability Probe
 *
 * Bu firmware, uzerinde calistigi target'in NELERI gercekten destekledigini
 * TAHMIN ETMEDEN, ID register'larini okuyarak raporlar.
 * Ciktisi docs/TARGET_CAPABILITIES.md dosyasini beslemek icin kullanilir.
 *
 * Okunan register'lar:
 *   CPUID     - islemci kimligi           (Armv8-M ARM, DDI 0553)
 *   MPU_TYPE  - MPU region sayisi
 *   SAU_TYPE  - SAU region sayisi (TrustZone gostergesi)
 *   MVFR0/1/2 - FP / MVE ozellikleri
 *   DWT_CTRL  - cycle counter var mi
 *   ID_ISAR*  - instruction set ozellikleri (DSP vb.)
 */
#include "platform/mps2_an505.h"   /* CMSIS-Core baseline */
#include "common/console.h"

static void probe_cpuid(void)
{
    uint32_t cpuid = SCB->CPUID;
    console_kv_hex("CPUID", cpuid);
    console_kv_hex("CPUID.PARTNO", (cpuid >> 4) & 0xFFFU);
    console_kv_hex("CPUID.VARIANT", (cpuid >> 20) & 0xFU);
    console_kv_hex("CPUID.REVISION", cpuid & 0xFU);
}

static void probe_mpu(void)
{
    uint32_t type = MPU->TYPE;
    console_kv_hex("MPU_TYPE", type);
    console_kv_u32("MPU_REGIONS", (type >> 8) & 0xFFU);
}

/*
 * SAU: CMSIS-Core, SAU_Type'i yalnizca derleyici -mcmse ile cagrildiginda
 * (__ARM_FEATURE_CMSE == 3) tanimlar. Bu repo su an -mcmse KULLANMIYOR,
 * bu yuzden SAU_TYPE'i mimari olarak sabit adresinden dogrudan okuyoruz.
 * Ders 16'da secure/non-secure ayrimi kurulurken -mcmse acilacak ve o
 * noktada CMSIS'in SAU tanimlari kullanilabilir hale gelecek.
 * Adres: SAU blogu SCS_BASE + 0x0DD0'da baslar; SAU_TYPE blogun IKINCI
 * register'idir (CTRL 0x00, TYPE 0x04), yani SCS_BASE + 0x0DD4.
 * (Armv8-M ARM, DDI 0553.)
 */
#define SAU_TYPE_ADDR  (SCS_BASE + 0x0DD4UL)

/*
 * NVIC oncelik biti sayisi - OLCUM.
 * Bir oncelik register'ina 0xFF yazip geri okuruz: uygulanmayan ALT bitler
 * sifir okunur, uygulanan UST bitler korunur. Deger geri yazilarak durum
 * eski haline getirilir.
 */
static void probe_nvic_prio_bits(void)
{
    uint8_t saved_irq = NVIC->IPR[0];
    NVIC->IPR[0] = 0xFFu;
    __DSB();
    __ISB();
    uint8_t rb_irq = NVIC->IPR[0];
    NVIC->IPR[0] = saved_irq;

    /* Sistem handler'lari icin de ayni olcum (SVCall = SHPR2 ust byte). */
    uint8_t saved_sh = SCB->SHPR[7];
    SCB->SHPR[7] = 0xFFu;
    __DSB();
    __ISB();
    uint8_t rb_sh = SCB->SHPR[7];
    SCB->SHPR[7] = saved_sh;

    uint32_t bits = 0u;
    for (uint32_t i = 0u; i < 8u; i++) {
        if (rb_irq & (1u << i)) {
            bits++;
        }
    }

    console_kv_hex("NVIC_IPR0_READBACK", rb_irq);
    console_kv_hex("SCB_SHPR_SVC_READBACK", rb_sh);
    console_kv_u32("NVIC_PRIO_BITS_MEASURED", bits);
    console_kv_u32("NVIC_PRIO_BITS_COMPILED", (uint32_t)__NVIC_PRIO_BITS);
    console_kv_str("NVIC_PRIO_BITS_MATCH",
                   (bits == (uint32_t)__NVIC_PRIO_BITS) ? "YES" : "NO");
}

static void probe_sau(void)
{
    uint32_t type = *(volatile const uint32_t *)SAU_TYPE_ADDR;
    console_kv_hex("SAU_TYPE", type);
    console_kv_u32("SAU_REGIONS", type & 0xFFU);
}

static void probe_fp_mve(void)
{
    console_kv_hex("MVFR0", FPU->MVFR0);
    console_kv_hex("MVFR1", FPU->MVFR1);
    console_kv_hex("MVFR2", FPU->MVFR2);
    console_kv_hex("MVFR1.MVE", (FPU->MVFR1 >> 8) & 0xFU);
    console_kv_hex("CPACR", SCB->CPACR);
}

static void probe_isar(void)
{
    for (int i = 0; i < 6; i++) {
        console_puts("[ID_ISAR");
        console_u32((uint32_t)i);
        console_puts("] ");
        console_hex32(SCB->ID_ISAR[i]);
        console_putc('\n');
    }
}

static void probe_dwt(void)
{
    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;
    __DSB();

    uint32_t ctrl = DWT->CTRL;
    console_kv_hex("DWT_CTRL", ctrl);
    console_kv_str("DWT_CYCCNT_IMPLEMENTED",
                   (ctrl & DWT_CTRL_NOCYCCNT_Msk) ? "NO" : "YES");

    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    __DSB();

    uint32_t t0 = DWT->CYCCNT;
    for (volatile int i = 0; i < 100; i++) { }
    uint32_t t1 = DWT->CYCCNT;

    console_kv_u32("DWT_CYCCNT_T0", t0);
    console_kv_u32("DWT_CYCCNT_T1", t1);
    console_kv_str("DWT_CYCCNT_COUNTING", (t1 != t0) ? "YES" : "NO");
}

static void probe_systick(void)
{
    console_kv_hex("SYSTICK_CALIB", SysTick->CALIB);
}

int main(void)
{
    console_line("");
    console_line("===== TARGET CAPABILITY PROBE =====");
    probe_cpuid();
    probe_mpu();
    probe_sau();
    probe_nvic_prio_bits();
    probe_fp_mve();
    probe_isar();
    probe_dwt();
    probe_systick();
    console_line("===== PROBE COMPLETE =====");

    for (;;) {
        __WFI();
    }
}
