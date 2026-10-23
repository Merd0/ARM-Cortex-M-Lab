#if defined(L12_TEACHING)
#include "core/cm33_teaching.h"
#else
#include "platform/mps2_an505.h"
#endif

uint32_t l12_read_cycle(void) { return DWT->CYCCNT; }
uint32_t l12_read_ipsr(void) { return __get_IPSR(); }
