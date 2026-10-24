#ifndef L13_SYNC_H
#define L13_SYNC_H
#include <stdint.h>
extern volatile uint32_t g_l13_counter, g_l13_irq_count;
extern volatile uint32_t g_l13_retries, g_l13_first_status;
void l13_reset(void);
void l13_pend_update(void);
uint32_t l13_wait_update(void);
void l13_protected_increment(volatile uint32_t *counter);
uint32_t l13_racy_increment(volatile uint32_t *counter);
uint32_t l13_exclusive_increment(volatile uint32_t *counter, uint32_t attempts);
uint32_t l13_dsp_checks(void);
#endif
