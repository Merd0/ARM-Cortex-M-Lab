#include "platform/mps2_an505.h"
void l14_priorities(void)
{
    NVIC_SetPriority(SysTick_IRQn, 64U);
#if defined(L14_BAD_PRIORITY)
    NVIC_SetPriority(PendSV_IRQn, 0U);
#else
    NVIC_SetPriority(PendSV_IRQn, 255U);
#endif
}
