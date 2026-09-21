/*
 * CMSDK APB UART driver (yalnizca transmit).
 * Register yerlesimi: Arm Cortex-M System Design Kit TRM (Arm DDI 0479).
 */
#include "platform/board.h"

typedef struct {
    volatile uint32_t DATA;      /* 0x00 RX/TX data                      */
    volatile uint32_t STATE;     /* 0x04 bit0: TX full, bit1: RX full    */
    volatile uint32_t CTRL;      /* 0x08 bit0: TX enable, bit1: RX enable */
    volatile uint32_t INTSTATUS; /* 0x0C                                  */
    volatile uint32_t BAUDDIV;   /* 0x10 (QEMU: >= 16 olmali)             */
} cmsdk_uart_t;

#define UART0 ((cmsdk_uart_t *)BOARD_UART0_S_BASE)

#define UART_STATE_TX_FULL  (1UL << 0)
#define UART_CTRL_TX_EN     (1UL << 0)

void board_uart_init(void)
{
    /* CMSDK UART: BAUDDIV = APB cevre birimi saati / baud.
     * 20 MHz / 115200 = 173 (tam bolunmez; kalan hata QEMU'da onemsizdir
     * cunku model seri veriyi bit zamanlamasiyla ornekle-miyor).
     * UYARI: UART'tan okunabilir metin cikmasi BAUDDIV'in dogru oldugunu
     * KANITLAMAZ; QEMU'nun CMSDK UART modeli BAUDDIV >= 16 disinda
     * zamanlama uygulamaz. Dogrulama kaynagi QEMU model kaynagidir. */
    UART0->BAUDDIV = BOARD_APB_CLK_HZ / BOARD_UART_BAUD;   /* 20e6/115200 = 173 */
    UART0->CTRL    = UART_CTRL_TX_EN;
}

void board_uart_putc(char c)
{
    while (UART0->STATE & UART_STATE_TX_FULL) {
        /* TX FIFO bosalana kadar bekle */
    }
    UART0->DATA = (uint32_t)(unsigned char)c;
}
