#ifndef L16_API_H
#define L16_API_H
#include <stdint.h>
#define L16_DENIED 0xDEAD0001U
uint32_t l16_service(uint32_t value);
void l16_report(uint32_t observations);
uint32_t l16_allowed(uint32_t value);
#endif
