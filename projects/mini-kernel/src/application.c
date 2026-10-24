#include "common/console.h"
#include <stdint.h>
/* Called by the Lesson 15 kernel with its short observation critical section.
 * Bounded work only: one producer/consumer operation, no UART or blocking. */
static uint32_t ring[8], head, tail, produced, consumed, sum;
void l15_application_step(uint32_t id)
{
    if(id==0U && produced<16U && head-tail<8U) {
        ring[head%8U]=produced; ++head; ++produced;
    } else if(id==1U && head!=tail) {
        sum+=ring[tail%8U]; ++tail; ++consumed;
    }
}
void l15_application_report(void)
{
    console_kv_u32("PRODUCED",produced); console_kv_u32("CONSUMED",consumed);
    console_kv_u32("QUEUE_SUM",sum);
    console_kv_str("CHECK_QUEUE",produced==16U && consumed==16U && head==tail && sum==120U?"PASS":"FAIL");
}
uint32_t l15_application_ready(void) { return produced==16U && consumed==16U; }
