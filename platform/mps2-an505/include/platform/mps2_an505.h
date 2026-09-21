/*
 * mps2_an505.h — CMSIS device header for the MPS2+ AN505 (Cortex-M33) target.
 *
 * This is the repository's PRODUCTION / BASELINE core API.
 * It configures official CMSIS-Core (Arm, Apache-2.0) for this device and
 * pulls in core_cm33.h. Every infrastructure source (startup, system init,
 * platform drivers, tools) includes this file.
 *
 * The hand-written, minimal alternative used for teaching lives in
 * core/include/core/cm33_teaching.h and is used ONLY by lesson firmware
 * where seeing the raw register layout is the point.
 * See docs/ARCHITECTURE.md ("CMSIS vs teaching header").
 *
 * Feature macros below are set from MEASURED target properties, not guesses:
 *   CPUID    = 0x410FD213  -> Cortex-M33 r0p3
 *   MPU_TYPE = 0x00000800  -> 8 MPU regions present
 *   SAU_TYPE = 0x00000008  -> 8 SAU regions present (TrustZone)
 * (source: tools/capability-probe, QEMU 8.2.2 mps2-an505, see
 *  docs/TARGET_CAPABILITIES.md)
 */
#ifndef MPS2_AN505_H
#define MPS2_AN505_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* Interrupt Number Definition                                              */
/* ------------------------------------------------------------------------ */
/*
 * Only the Cortex-M33 core exceptions are declared here. They are
 * architecturally fixed, so they are safe to state.
 *
 * Device-specific (external) IRQ numbers for the AN505 are deliberately NOT
 * listed yet: they come from the AN505 application note and have not been
 * verified against it in this repository. They will be added in lesson 07
 * (Exceptions & NVIC) after verification, rather than guessed now.
 */
typedef enum IRQn
{
    /* ----- Cortex-M33 processor exceptions ----- */
    NonMaskableInt_IRQn   = -14,   /*!< Non Maskable Interrupt            */
    HardFault_IRQn        = -13,   /*!< HardFault                         */
    MemoryManagement_IRQn = -12,   /*!< MemManage fault                   */
    BusFault_IRQn         = -11,   /*!< BusFault                          */
    UsageFault_IRQn       = -10,   /*!< UsageFault                        */
    SecureFault_IRQn      =  -9,   /*!< SecureFault                       */
    SVCall_IRQn           =  -5,   /*!< SVCall                            */
    DebugMonitor_IRQn     =  -4,   /*!< Debug Monitor                     */
    PendSV_IRQn           =  -2,   /*!< PendSV                            */
    SysTick_IRQn          =  -1    /*!< System Tick                       */

    /* TODO(lesson 07): AN505 external IRQ numbers, verified against the
     *                  Arm MPS2+ AN505 application note.               */
} IRQn_Type;

/* ------------------------------------------------------------------------ */
/* Processor and Core Peripheral Configuration                              */
/* ------------------------------------------------------------------------ */
#define __CM33_REV              0x0003U   /* Core revision r0p3 (CPUID) */
/*
 * NVIC oncelik biti sayisi: hedefe ozgu, mimari bir "varsayilan" YOKTUR.
 *
 * Onceki deger (3) "Armv8-M Mainline default" diye gerekcelendirilmisti;
 * boyle bir evrensel varsayilan yok (bagimsiz denetim bulgusu I2).
 *
 * Bu hedegin sozlesmesi QEMU kaynagindan dogrulandi:
 *   QEMU v8.2.2, hw/intc/armv7m_nvic.c, armv7m_nvic_realize():
 *     s->num_prio_bits = arm_feature(&s->cpu->env, ARM_FEATURE_V7) ? 8 : 2;
 *   Cortex-M33 ARM_FEATURE_V7 tasir -> 8 bit.
 *
 * Ayrica calisma zamaninda olculur: tools/capability-probe bir oncelik
 * register'ina 0xFF yazip geri okur ve uygulanan bit sayisini raporlar.
 *
 * Fiziksel MPS2+ AN505 icin bu deger bagimsiz olarak DOGRULANMAMISTIR.
 */
#define __NVIC_PRIO_BITS        8U        /* QEMU mps2-an505: olculdu    */
#define __Vendor_SysTickConfig  0U        /* CMSIS SysTick_Config used   */
#define __VTOR_PRESENT          1U
#define __MPU_PRESENT           1U        /* measured: 8 regions         */
#define __SAUREGION_PRESENT     1U        /* measured: 8 SAU regions     */
#define __FPU_PRESENT           1U        /* present in HW ...           */
#define __DSP_PRESENT           1U
#define __PMU_PRESENT           0U        /* not implemented on Cortex-M33 */

/*
 * NOTE on the FPU: the processor implements it, but this repository builds
 * with -mfloat-abi=soft on purpose, so CPACR is left at 0 and the exception
 * stack frame stays 8 words. That keeps lessons 07/08 simple. The FPU is
 * switched on deliberately in lesson 13.
 */

#include "core_cm33.h"

/* ------------------------------------------------------------------------ */
/* System init (CMSIS system_<device> contract)                             */
/* ------------------------------------------------------------------------ */
extern uint32_t SystemCoreClock;

void SystemInit(void);
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* MPS2_AN505_H */
