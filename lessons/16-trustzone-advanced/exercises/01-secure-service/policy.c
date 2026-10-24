#include "../../api.h"
uint32_t l16_allowed(uint32_t value)
{
    /* TODO: permit only the documented closed interval [0,15].
     * Keep starter fail-closed; do not change attribution or gateway code. */
    (void)value;
    return 0U;
}
