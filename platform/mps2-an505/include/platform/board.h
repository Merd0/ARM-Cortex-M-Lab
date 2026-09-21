/*
 * MPS2-AN505 (Cortex-M33, Arm IoT Kit) board definitions.
 *
 * Adresler QEMU mps2-an505 modelinden ("info mtree -f") dogrulanmis ve
 * Arm Application Note AN505 (DAI0505) / IoT Kit memory map ile
 * karsilastirilmistir.
 *
 * Firmware Secure state'te boot eder; bu yuzden Secure alias (bit 28 = 1)
 * adresleri kullanilir.
 */
#ifndef PLATFORM_BOARD_H
#define PLATFORM_BOARD_H

#include <stdint.h>

/* Kod / veri bolgeleri (Secure alias) */
#define BOARD_CODE_S_BASE   0x10000000UL   /* ZBT SSRAM1, 4 MB            */
#define BOARD_SRAM_S_BASE   0x30000000UL   /* IoT Kit internal SRAM, 32 KB */
#define BOARD_SSRAM23_S     0x38000000UL   /* ZBT SSRAM2 + SSRAM3, 2+2 MB  */

/* Non-secure alias'lar (bit 28 = 0) */
#define BOARD_CODE_NS_BASE  0x00000000UL
#define BOARD_SRAM_NS_BASE  0x20000000UL

/* CMSDK APB UART0 */
#define BOARD_UART0_S_BASE  0x50200000UL
#define BOARD_UART0_NS_BASE 0x40200000UL

/*
 * SYSCLK: DOGRULANMIS QEMU SOZLESMESI, tahmin degil.
 *
 * QEMU v8.2.2, hw/arm/mps2-tz.c, mps2tz_an505_class_init():
 *     mmc->sysclk_frq      = 20 * 1000 * 1000;   -- 20 MHz
 *     mmc->apb_periph_frq  = mmc->sysclk_frq;    -- APB = SYSCLK
 *
 * Yani bu hedefte cekirdek saati de APB cevre birimi saati de 20 MHz'dir.
 * Onceki 25 MHz degeri YANLISTI (bagimsiz denetim bulgusu I1) ve
 * SysTick tabanli her hesabi %25 saptiriyordu.
 *
 * Fiziksel MPS2+ AN505 karti icin bu deger bagimsiz olarak
 * DOGRULANMAMISTIR; bkz. docs/REAL_HARDWARE_MIGRATION.md.
 */
#define BOARD_SYSCLK_HZ     20000000UL   /* QEMU mps2-an505 sozlesmesi */

/* APB cevre birimi saati. Bu modelde SYSCLK ile aynidir. */
#define BOARD_APB_CLK_HZ    BOARD_SYSCLK_HZ
#define BOARD_UART_BAUD     115200UL

void board_uart_init(void);
void board_uart_putc(char c);

#endif /* PLATFORM_BOARD_H */
