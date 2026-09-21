/*
 * cm33_teaching.h — hand-written, minimal Armv8-M (Cortex-M33) definitions.
 *
 * !! THIS IS NOT THE REPOSITORY BASELINE !!
 *
 * The repository's production core API is official CMSIS-Core, configured by
 * platform/include/platform/mps2_an505.h. Infrastructure code (startup,
 * system init, drivers, tools) uses THAT.
 *
 * This header exists for one reason only: in some lessons, seeing the raw
 * register layout IS the learning objective. CMSIS is excellent engineering
 * but it is also a very large abstraction; reading 3000 lines to find where
 * CONTROL lives does not teach the programmer model. So lesson firmware may
 * include this 200-line version instead, then compare it against CMSIS.
 *
 * Naming follows CMSIS (SCB, SysTick, NVIC, MPU, SAU, DWT) so that every name
 * here can be looked up verbatim in the official Arm documentation.
 *
 * It is NOT a drop-in replacement for CMSIS. This is an educational SUBSET
 * with deliberate differences:
 *   - only the registers the lessons actually touch are declared;
 *   - bit-position/mask macros (SCB_CFSR_STKOF_Msk and friends) are absent;
 *   - peripherals such as FPU, PMU, DCB and the Armv8-M MPU helper API are
 *     missing entirely;
 *   - CMSIS guards SAU behind -mcmse; this header does not.
 * Code written against this header may fail to compile against CMSIS and
 * vice versa. Swap them only in the small lesson files that were written for
 * that comparison.
 *
 * Rule of thumb:
 *   lessons/   -> may use this header when registers are the topic
 *   everything else -> CMSIS (platform/mps2_an505.h)
 *
 * Note: __set_CONTROL() below executes an ISB, exactly like the CMSIS one.
 * So calling __ISB() again after it is redundant here too.
 *
 * See docs/ARCHITECTURE.md, section "CMSIS vs teaching header".
 *
 * Reference: Armv8-M Architecture Reference Manual (Arm DDI 0553),
 *            Cortex-M33 Devices Generic User Guide (Arm DUI 1096).
 */
#ifndef CORE_CM33_TEACHING_H
#define CORE_CM33_TEACHING_H

#include <stdint.h>

#define __IO volatile
#define __I  volatile const

/* ---------------------------------------------------------------- SCS map */
#define SCS_BASE        (0xE000E000UL)
#define SYSTICK_BASE    (SCS_BASE + 0x0010UL)
#define NVIC_BASE       (SCS_BASE + 0x0100UL)
#define SCB_BASE        (SCS_BASE + 0x0D00UL)
#define MPU_BASE        (SCS_BASE + 0x0D90UL)
#define SAU_BASE        (SCS_BASE + 0x0DD0UL)
#define DWT_BASE        (0xE0001000UL)

/* --------------------------------------------------- System Control Block */
typedef struct {
    __I  uint32_t CPUID;    /* 0x000 CPUID Base Register              */
    __IO uint32_t ICSR;     /* 0x004 Interrupt Control and State      */
    __IO uint32_t VTOR;     /* 0x008 Vector Table Offset              */
    __IO uint32_t AIRCR;    /* 0x00C App Interrupt / Reset Control    */
    __IO uint32_t SCR;      /* 0x010 System Control                   */
    __IO uint32_t CCR;      /* 0x014 Configuration and Control        */
    __IO uint8_t  SHPR[12]; /* 0x018 System Handler Priority          */
    __IO uint32_t SHCSR;    /* 0x024 System Handler Control and State */
    __IO uint32_t CFSR;     /* 0x028 Configurable Fault Status        */
    __IO uint32_t HFSR;     /* 0x02C HardFault Status                 */
    __IO uint32_t DFSR;     /* 0x030 Debug Fault Status               */
    __IO uint32_t MMFAR;    /* 0x034 MemManage Fault Address          */
    __IO uint32_t BFAR;     /* 0x038 BusFault Address                 */
    __IO uint32_t AFSR;     /* 0x03C Auxiliary Fault Status           */
    __I  uint32_t ID_PFR[2];
    __I  uint32_t ID_DFR;
    __I  uint32_t ID_AFR;
    __I  uint32_t ID_MMFR[4];
    __I  uint32_t ID_ISAR[6];
    uint32_t RESERVED0[1];
    __I  uint32_t CLIDR;
    __I  uint32_t CTR;
    __I  uint32_t CCSIDR;
    __IO uint32_t CSSELR;
    __IO uint32_t CPACR;    /* 0x088 Coprocessor Access Control       */
    __IO uint32_t NSACR;    /* 0x08C Non-secure Access Control        */
} SCB_Type;
#define SCB  ((SCB_Type *)SCB_BASE)

#define SCB_CCR_DIV_0_TRP        (1UL << 4)
#define SCB_CCR_UNALIGN_TRP      (1UL << 3)
#define SCB_SHCSR_MEMFAULTENA    (1UL << 16)
#define SCB_SHCSR_BUSFAULTENA    (1UL << 17)
#define SCB_SHCSR_USGFAULTENA    (1UL << 18)

/* Debug Exception and Monitor Control Register */
#define DEMCR          (*(__IO uint32_t *)0xE000EDFCUL)
#define DEMCR_TRCENA   (1UL << 24)

/* ------------------------------------------------------------- SysTick */
typedef struct {
    __IO uint32_t CTRL;
    __IO uint32_t LOAD;
    __IO uint32_t VAL;
    __I  uint32_t CALIB;
} SysTick_Type;
#define SysTick ((SysTick_Type *)SYSTICK_BASE)

#define SysTick_CTRL_ENABLE     (1UL << 0)
#define SysTick_CTRL_TICKINT    (1UL << 1)
#define SysTick_CTRL_CLKSOURCE  (1UL << 2)
#define SysTick_CTRL_COUNTFLAG  (1UL << 16)

/* ---------------------------------------------------------------- NVIC */
typedef struct {
    __IO uint32_t ISER[16];
    uint32_t RESERVED0[16];
    __IO uint32_t ICER[16];
    uint32_t RESERVED1[16];
    __IO uint32_t ISPR[16];
    uint32_t RESERVED2[16];
    __IO uint32_t ICPR[16];
    uint32_t RESERVED3[16];
    __IO uint32_t IABR[16];
    uint32_t RESERVED4[16];
    __IO uint32_t ITNS[16];
    uint32_t RESERVED5[16];
    __IO uint8_t  IPR[496];
} NVIC_Type;
#define NVIC ((NVIC_Type *)NVIC_BASE)

/* ----------------------------------------------------------------- MPU */
typedef struct {
    __I  uint32_t TYPE;
    __IO uint32_t CTRL;
    __IO uint32_t RNR;
    __IO uint32_t RBAR;
    __IO uint32_t RLAR;
    __IO uint32_t RBAR_A1;
    __IO uint32_t RLAR_A1;
    __IO uint32_t RBAR_A2;
    __IO uint32_t RLAR_A2;
    __IO uint32_t RBAR_A3;
    __IO uint32_t RLAR_A3;
    uint32_t RESERVED0[1];
    __IO uint32_t MAIR[2];
} MPU_Type;
#define MPU ((MPU_Type *)MPU_BASE)

#define MPU_CTRL_ENABLE        (1UL << 0)
#define MPU_CTRL_HFNMIENA      (1UL << 1)
#define MPU_CTRL_PRIVDEFENA    (1UL << 2)

/* ----------------------------------------------------------------- SAU */
typedef struct {
    __IO uint32_t CTRL;
    __I  uint32_t TYPE;
    __IO uint32_t RNR;
    __IO uint32_t RBAR;
    __IO uint32_t RLAR;
} SAU_Type;
#define SAU ((SAU_Type *)SAU_BASE)

/* ----------------------------------------------------------------- DWT */
typedef struct {
    __IO uint32_t CTRL;
    __IO uint32_t CYCCNT;
    __IO uint32_t CPICNT;
    __IO uint32_t EXCCNT;
    __IO uint32_t SLEEPCNT;
    __IO uint32_t LSUCNT;
    __IO uint32_t FOLDCNT;
} DWT_Type;
#define DWT ((DWT_Type *)DWT_BASE)

#define DWT_CTRL_CYCCNTENA     (1UL << 0)
#define DWT_CTRL_NOCYCCNT      (1UL << 25)

/* -------------------------------------------------- FPU / MVE ID regs */
#define MVFR0  (*(__I uint32_t *)0xE000EF40UL)
#define MVFR1  (*(__I uint32_t *)0xE000EF44UL)
#define MVFR2  (*(__I uint32_t *)0xE000EF48UL)

/* ---------------------------------------------------- Core intrinsics */
#define __NOP()   __asm volatile ("nop")
#define __WFI()   __asm volatile ("wfi")
#define __DSB()   __asm volatile ("dsb 0xF" ::: "memory")
#define __DMB()   __asm volatile ("dmb 0xF" ::: "memory")
#define __ISB()   __asm volatile ("isb 0xF" ::: "memory")
#define __BKPT()  __asm volatile ("bkpt #0")

static inline uint32_t __get_MSP(void)
{ uint32_t r; __asm volatile ("mrs %0, msp" : "=r"(r)); return r; }

static inline uint32_t __get_PSP(void)
{ uint32_t r; __asm volatile ("mrs %0, psp" : "=r"(r)); return r; }

static inline void __set_PSP(uint32_t v)
{ __asm volatile ("msr psp, %0" :: "r"(v)); }

static inline uint32_t __get_CONTROL(void)
{ uint32_t r; __asm volatile ("mrs %0, control" : "=r"(r)); return r; }

static inline void __set_CONTROL(uint32_t v)
{ __asm volatile ("msr control, %0" :: "r"(v) : "memory"); __ISB(); }

static inline uint32_t __get_xPSR(void)
{ uint32_t r; __asm volatile ("mrs %0, xpsr" : "=r"(r)); return r; }

static inline uint32_t __get_IPSR(void)
{ uint32_t r; __asm volatile ("mrs %0, ipsr" : "=r"(r)); return r; }

static inline uint32_t __get_PRIMASK(void)
{ uint32_t r; __asm volatile ("mrs %0, primask" : "=r"(r)); return r; }

static inline void __disable_irq(void) { __asm volatile ("cpsid i" ::: "memory"); }
static inline void __enable_irq(void)  { __asm volatile ("cpsie i" ::: "memory"); }

#endif /* CORE_CM33_TEACHING_H */
