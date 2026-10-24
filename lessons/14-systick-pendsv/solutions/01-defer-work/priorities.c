#include "platform/mps2_an505.h"
void l14_priorities(void)
{
    NVIC_SetPriority(SysTick_IRQn, 64U);
    NVIC_SetPriority(PendSV_IRQn, 255U);
}
