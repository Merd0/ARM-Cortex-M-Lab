#include "../../sync.h"

void l13_protected_increment(volatile uint32_t *counter)
{
    /* TODO(student): save PRIMASK, mask, read, l13_pend_update(), update,
     * restore the saved mask. The caller handles deferred exception delivery. */
    (void)counter;
}
