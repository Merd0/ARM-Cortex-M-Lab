#include "../../sync.h"
#include "platform/mps2_an505.h"

void l13_protected_increment(volatile uint32_t *counter)
{
    uint32_t saved = __get_PRIMASK();
    __disable_irq();
    uint32_t value = *counter;
    l13_pend_update();
    *counter = value + 1U;
    __set_PRIMASK(saved);
    __ISB();
}
