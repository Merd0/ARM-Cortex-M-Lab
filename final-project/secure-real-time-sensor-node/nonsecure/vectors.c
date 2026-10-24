#include <stdint.h>
extern uint32_t __stack_top;
void NS_Reset_Handler(void);
void SVC_Handler(void); void PendSV_Handler(void); void SysTick_Handler(void);
void node_fault_entry(void);
typedef void (*handler_t)(void);
__attribute__((section(".vectors"),used)) const handler_t node_vectors[16]={
    (handler_t)&__stack_top,NS_Reset_Handler,node_fault_entry,node_fault_entry,
    node_fault_entry,node_fault_entry,node_fault_entry,0,0,0,0,SVC_Handler,
    node_fault_entry,0,PendSV_Handler,SysTick_Handler
};
