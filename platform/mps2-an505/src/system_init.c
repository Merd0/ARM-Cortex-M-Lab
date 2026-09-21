/*
 * SystemInit: main'den once calisan minimum sistem kurulumu.
 * Burada bilerek cok az sey yapiyoruz; her ek adim bir derste eklenir.
 */
#include "platform/mps2_an505.h"
#include "platform/board.h"

/*
 * CMSIS sozlesmesi: SystemCoreClock, cekirdek saat frekansini Hz cinsinden
 * tutar. Bu hedefte deger BOARD_SYSCLK_HZ'den gelir ve QEMU'nun AN505
 * modelinden dogrulanmistir (v8.2.2 hw/arm/mps2-tz.c: sysclk_frq = 20 MHz).
 * Tek kaynak board.h'dir; burada sabit tekrarlanmaz.
 */
uint32_t SystemCoreClock = BOARD_SYSCLK_HZ;

void SystemCoreClockUpdate(void)
{
    /* Sabit saat; konfigurasyon edilebilir bir PLL yok. */
    SystemCoreClock = BOARD_SYSCLK_HZ;
}

void SystemInit(void)
{
    /* Vector table'in yerini acikca bildir.
       Reset'te zaten 0x10000000, ama VTOR'u gormek ogretici. */
    SCB->VTOR = BOARD_CODE_S_BASE;

    board_uart_init();
}
