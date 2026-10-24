#include <stdint.h>
extern uint32_t __etext, __data_start, __data_end, __bss_start, __bss_end, __stack_top;
extern int main(void);
void NS_Reset_Handler(void);
static void unexpected(void) { for (;;) __asm volatile("wfi"); }
typedef void (*handler_t)(void);
__attribute__((section(".vectors"),used))
const handler_t ns_vectors[16] = {
    (handler_t)&__stack_top, NS_Reset_Handler, unexpected, unexpected,
    unexpected, unexpected, unexpected, 0, 0, 0, 0, unexpected, unexpected, 0,
    unexpected, unexpected
};
void NS_Reset_Handler(void)
{
    uint32_t *src = &__etext;
    for (uint32_t *p = &__data_start; p < &__data_end; ++p) *p = *src++;
    for (uint32_t *p = &__bss_start; p < &__bss_end; ++p) *p = 0U;
    (void)main();
    unexpected();
}
