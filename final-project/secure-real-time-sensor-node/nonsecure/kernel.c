#include "../../../../lessons/15-mini-kernel/kernel.h"
#include "../../../../lessons/16-trustzone-advanced/api.h"
#include "../../../../projects/fault-analyzer/diagnostic.h"
#include "node.h"
#include "platform/mps2_an505.h"
#include "common/console.h"
#include <stddef.h>
#if NODE_TEST == 5
#define TASK_COUNT 4U
#else
#define TASK_COUNT 3U
#endif
#define WORDS 512U
#define GUARD 0xC0DEF00DU
typedef struct { uint32_t lower[8],words[WORDS],upper[8]; } stack_t;
static stack_t stacks[TASK_COUNT] __attribute__((aligned(32)));
static task_t tasks[TASK_COUNT];
task_t *g_l15_current;
volatile uint32_t g_l15_booted;
volatile uint32_t g_l15_secure_deferred;
static volatile uint32_t ticks, switches, preemptions, spin_preemptions, tick_pending;
static uint32_t runs[TASK_COUNT], bad_register, bad_privilege, rejected;
static uint32_t thread_control,last_svc_return;
static volatile uint32_t kernel_private=0x4B45524EU;
typedef struct {uint32_t value,tick;} sample_t;
static sample_t input[8],output[8];
static uint32_t in_head,in_tail,out_head,out_tail,produced,processed,logged;
static uint32_t history[4],history_sum,filtered_sum,max_age;
static uintptr_t attempted_address;
extern uint32_t __stack_top;
extern char node_after_yield,node_spin;
void node_overflow(uint32_t low) __attribute__((noreturn));

/* UART remains Secure. Only privileged reporting paths use this scalar channel. */
void board_uart_putc(char c) { l16_report(0x55000000U|(uint32_t)(uint8_t)c); }
static void check(const char *key,uint32_t ok) { console_kv_str(key,ok?"PASS":"FAIL"); }
static uint32_t id_of(const task_t *task) { return (uint32_t)(task-tasks); }
static uint32_t guards(void)
{
    for(uint32_t i=0U;i<TASK_COUNT;++i) for(uint32_t j=0U;j<8U;++j)
        if(stacks[i].lower[j]!=GUARD || stacks[i].upper[j]!=GUARD) return 0U;
    return kernel_private==0x4B45524EU;
}
static void mpu_task(const task_t *task)
{
    ARM_MPU_SetRegion(1U, ARM_MPU_RBAR((uint32_t)(uintptr_t)task->low,0U,0U,1U,1U),
        ARM_MPU_RLAR((uint32_t)(uintptr_t)task->high-1U,0U));
    __DSB(); __ISB();
}
static uint32_t valid(const task_t *task)
{
    uintptr_t sp=(uintptr_t)task->sp;
    if(sp<(uintptr_t)task->low || sp>(uintptr_t)task->high-68U || ((sp+36U)&7U)) return 0U;
    /* An interrupt can precede SG: the frame still belongs to NS Thread,
     * but its return PC is a gateway entry outside the ordinary NS image.
     * Accept only the imported SG entry points, never arbitrary Secure code. */
    uint32_t pc=task->sp[15];
    uint32_t entry=pc==((uint32_t)(uintptr_t)&l16_service&~1U) ||
                   pc==((uint32_t)(uintptr_t)&l16_report&~1U);
    return task->sp[8]==L15_EXC_RETURN && (task->sp[16]&(1U<<24)) &&
           !(pc&1U) && ((pc>=0x80000U && pc<0xA0000U) || entry);
}
static void finish(void) __attribute__((noreturn));
static void finish(void)
{
    SysTick->CTRL=0U; __disable_irq();
    SCB->ICSR=SCB_ICSR_PENDSTCLR_Msk|SCB_ICSR_PENDSVCLR_Msk;
    console_line("===== FINAL: SENSOR NODE =====");
    console_kv_u32("PRODUCED",produced); console_kv_u32("PROCESSED",processed);
    console_kv_u32("LOGGED",logged); console_kv_u32("FILTERED_SUM",filtered_sum);
    console_kv_u32("TICKS",ticks); console_kv_u32("SWITCHES",switches);
    console_kv_u32("PREEMPTIONS",preemptions); console_kv_u32("MAX_MODEL_AGE",max_age);
    console_kv_hex("THREAD_CONTROL",thread_control);
    console_kv_hex("HANDLER_CONTROL",__get_CONTROL());
    console_kv_hex("LAST_SVC_RETURN",last_svc_return);
    check("CHECK_PIPELINE",produced==32U && processed==32U && logged==32U &&
          in_head==in_tail && out_head==out_tail);
    check("CHECK_RESULT",filtered_sum==298U);
    check("CHECK_TASK_PRIVILEGE",!bad_privilege);
    check("CHECK_REGISTERS",!bad_register);
    check("CHECK_STACKS",guards());
    check("CHECK_BASIC_CONTEXT",last_svc_return==L15_EXC_RETURN && SCB->CPACR==0U);
    check("CHECK_SCHEDULING",ticks>=4U && switches>=3U && preemptions>0U);
    check("CHECK_MODEL_BUDGET",max_age<=16U);
    check("CHECK_NO_FAULT",SCB->CFSR==0U && SCB->HFSR==0U);
#if NODE_TEST == 5
    check("CHECK_SPIN_PREEMPTED",runs[3]==1U && spin_preemptions>0U);
#endif
#if NODE_TEST == 6
    check("CHECK_BAD_SVC_REJECT",rejected>0U);
    check("CHECK_SECURE_CONTEXT_DEFERRED",g_l15_secure_deferred>0U);
#endif
    l16_report(0x7E000000U);
    for (;;) __WFI();
}
void l15_panic(void)
{
    SysTick->CTRL=0U; __disable_irq();
    console_kv_hex("PANIC_RETURN",(uint32_t)(uintptr_t)__builtin_return_address(0));
    console_kv_hex("PANIC_PSP",__get_PSP());
    console_kv_hex("PANIC_CFSR",SCB->CFSR);
    check("CHECK_KERNEL_STATE",0U);
    console_line("===== END =====");
    for (;;) __WFI();
}
void l15_task_exit(void) { l15_panic(); }
void SysTick_Handler(void)
{
    ++ticks; tick_pending=1U; SCB->ICSR=SCB_ICSR_PENDSVSET_Msk;
}
task_t *l15_schedule(void)
{
    task_t *old=g_l15_current;
    if(!guards() || !valid(old)) l15_panic();
    uint32_t id=id_of(old);
    task_t *next=&tasks[(id+1U)%TASK_COUNT];
    if(!valid(next)) l15_panic();
    if(tick_pending && old->sp[15]!=((uint32_t)(uintptr_t)&node_after_yield&~1U)) {
        ++preemptions;
        if(id==3U) ++spin_preemptions;
    }
    tick_pending=0U; ++switches; g_l15_current=next;
    mpu_task(next);
    return next;
}
static uint32_t service(uint32_t op,uint32_t value)
{
    uint32_t id=id_of(g_l15_current);
    switch(op) {
    case OP_OBSERVE:
        ++runs[id];
        thread_control=value;
        if(value!=3U) bad_privilege=1U;
        return 0U;
    case OP_SAMPLE:
        if(id!=0U) break;
        if(produced>=32U || in_head-in_tail>=8U) return 0U;
        input[in_head%8U]=(sample_t){produced%16U,ticks};
        ++in_head; ++produced; return 1U;
    case OP_PROCESS:
        if(id!=1U) break;
        if(in_tail==in_head || out_head-out_tail>=8U) return 0U;
        {
            sample_t sample=input[in_tail%8U]; ++in_tail;
            uint32_t slot=processed%4U;
            history_sum-=history[slot]; history[slot]=sample.value; history_sum+=sample.value;
            uint32_t count=processed<4U?processed+1U:4U;
            uint32_t result=l16_service(history_sum/count);
            if(result==L16_DENIED) l15_panic();
            output[out_head%8U]=(sample_t){result,sample.tick};
            ++out_head; ++processed;
        }
        return 1U;
    case OP_LOG:
        if(id!=2U) break;
        if(out_tail!=out_head) {
            sample_t sample=output[out_tail%8U]; ++out_tail;
            uint32_t age=ticks-sample.tick;
            if(age>max_age) max_age=age;
            filtered_sum+=sample.value; ++logged;
            if((logged%8U)==0U) {
                console_kv_u32("TELEMETRY_SEQ",logged);
                console_kv_u32("TELEMETRY_VALUE",sample.value);
            }
        }
        if(logged==32U && ticks>=4U && preemptions>0U) finish();
        return 1U;
    case OP_STATUS:
        return produced>=8U && processed>=4U && logged>=4U;
    case OP_TARGET:
#if NODE_TEST == 1
        attempted_address=(uintptr_t)&kernel_private;
#elif NODE_TEST == 2
        attempted_address=(uintptr_t)tasks[1].low;
#else
        attempted_address=(uintptr_t)g_l15_current->low;
#endif
        return (uint32_t)attempted_address;
    default: break;
    }
    ++rejected; return NODE_ERROR;
}
void node_svc_dispatch(exception_frame_t *frame,uint32_t ret)
{
    fault_snapshot_t s={0}; s.exc_return=ret;
    task_t *task=g_l15_current;
    if(ret!=L15_EXC_RETURN || !fa_capture(&s,(uintptr_t)frame,(uintptr_t)task->low,(uintptr_t)task->high) ||
        frame->pc<0x80002U || frame->pc>=0xA0000U) l15_panic();
    last_svc_return=ret;
    uint16_t insn=*(uint16_t *)(uintptr_t)(frame->pc-2U);
    if(insn==0xDF03U) {
        SCB->ICSR=SCB_ICSR_PENDSVSET_Msk;
    } else if(insn==0xDF00U) {
        frame->r0=service(frame->r0,frame->r1);
    } else if(insn==0xDF04U) {
        bad_register=1U;
    } else {++rejected;frame->r0=NODE_ERROR;}
}
static uint32_t call(uint32_t op,uint32_t value)
{
    register uint32_t r0 __asm("r0")=op;
    register uint32_t r1 __asm("r1")=value;
    __asm volatile("svc #0" : "+r"(r0),"+r"(r1) :: "r2","r3","r12","cc","memory");
    return r0;
}
void node_step(uint32_t id,uint32_t registers_ok)
{
    if(!registers_ok) __asm volatile("svc #4" ::: "memory");
    (void)call(OP_OBSERVE,__get_CONTROL());
#if NODE_TEST >= 1 && NODE_TEST <= 4
    if(id==0U && call(OP_STATUS,0U)) {
#if NODE_TEST == 3
        uint32_t value=*(volatile const uint32_t *)0x30000000U;
        (void)value;
#elif NODE_TEST == 4
        node_overflow(call(OP_TARGET,0U));
#else
        *(volatile uint32_t *)(uintptr_t)call(OP_TARGET,0U)=0xBADC0DEU;
#endif
    }
#endif
#if NODE_TEST == 6
    if(call(0xFFFFFFFFU,0U)!=NODE_ERROR) __asm volatile("svc #4" ::: "memory");
    register uint32_t boot_result __asm("r0")=0U;
    __asm volatile("svc #1" : "+r"(boot_result) :: "r1","r2","r3","r12","cc","memory");
    if(boot_result!=NODE_ERROR) __asm volatile("svc #4" ::: "memory");
    /* Unprivileged Thread cannot stop the diagnostic channel or write UART. */
    l16_report(0x7E000000U);
    l16_report(0x55000021U);
    if(l16_service(7U)!=L16_DENIED) __asm volatile("svc #4" ::: "memory");
#endif
    if(id==0U) (void)call(OP_SAMPLE,0U);
    if(id==1U) (void)call(OP_PROCESS,0U);
    if(id==2U) (void)call(OP_LOG,0U);
}
void node_fault(exception_frame_t *frame,uint32_t ret)
{
    SysTick->CTRL=0U; __disable_irq();
    task_t *task=g_l15_current;
    fault_snapshot_t s={0}; fault_decision_t d;
    s.ipsr=__get_IPSR(); s.exc_return=ret; s.cfsr=SCB->CFSR; s.hfsr=SCB->HFSR;
    s.mmfar_raw=SCB->MMFAR; s.bfar_raw=SCB->BFAR;
    fa_capture(&s,(uintptr_t)frame,(uintptr_t)task->low,(uintptr_t)task->high); fa_analyze(&s,&d);
    console_kv_hex("CFSR",s.cfsr); console_kv_hex("HFSR",s.hfsr);
    console_kv_hex("EXC_RETURN",ret); console_kv_str("FAULT_CLASS",d.fault_class);
#if NODE_TEST == 1 || NODE_TEST == 2
    check("CHECK_EXPECTED_MPU_FAULT",s.ipsr==4U && s.cfsr==0x82U && d.frame_safe &&
          d.address_valid && d.fault_address==attempted_address);
#elif NODE_TEST == 4
    check("CHECK_STACK_FAULT_REJECT",(s.cfsr&SCB_CFSR_STKOF_Msk) && !d.frame_safe);
#else
    check("CHECK_UNEXPECTED_FAULT",0U);
#endif
    check("CHECK_FAULT_NOT_ESCALATED",s.hfsr==0U);
    check("CHECK_KERNEL_STACK",__get_MSP()>0x20007000U && __get_MSP()<=0x20008000U);
    check("CHECK_GUARDS",guards());
    check("CHECK_PIPELINE_STARTED",processed>0U && logged>0U);
    l16_report(0x7E000000U);
    for (;;) __WFI();
}
int main(void)
{
    for(uint32_t i=0U;i<TASK_COUNT;++i) {
        for(uint32_t j=0U;j<8U;++j) stacks[i].lower[j]=stacks[i].upper[j]=GUARD;
        tasks[i].low=stacks[i].words; tasks[i].high=stacks[i].words+WORDS;
        tasks[i].sp=l15_initial_frame(tasks[i].high,i,(uintptr_t)&l15_task_entry,(uintptr_t)&l15_task_exit);
        if(!valid(&tasks[i])) l15_panic();
    }
    g_l15_current=&tasks[0];
    __set_MSPLIM(0x20007000U);
    ARM_MPU_Disable(); ARM_MPU_SetMemAttr(0U,0xFFU);
    ARM_MPU_SetRegion(0U,ARM_MPU_RBAR(0x80000U,0U,1U,1U,0U),ARM_MPU_RLAR(0x9FFFFU,0U));
    mpu_task(g_l15_current); ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
    SCB->SHCSR|=SCB_SHCSR_MEMFAULTENA_Msk|SCB_SHCSR_USGFAULTENA_Msk|SCB_SHCSR_BUSFAULTENA_Msk;
    NVIC_SetPriority(SVCall_IRQn,0U); NVIC_SetPriority(SysTick_IRQn,64U);
    NVIC_SetPriority(PendSV_IRQn,255U);
    SysTick->LOAD=19999U; SysTick->VAL=0U; SysTick->CTRL=0U;
    l15_start();
}
