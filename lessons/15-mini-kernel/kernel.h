#ifndef L15_KERNEL_H
#define L15_KERNEL_H
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t *sp;
    uint32_t *low;
    uint32_t *high;
} task_t;
_Static_assert(offsetof(task_t, low) == 4 && offsetof(task_t, high) == 8, "ASM TCB offsets");
enum { L15_WORDS = 512, L15_SAVED_WORDS = 9, L15_CONTEXT_WORDS = 17 };
#ifdef L15_NONSECURE
#define L15_EXC_RETURN 0xFFFFFFBCU
#else
#define L15_EXC_RETURN 0xFFFFFFFDU
#endif
uint32_t *l15_initial_frame(uint32_t *top, uint32_t id, uintptr_t entry, uintptr_t exit);
void l15_task_entry(void);
void l15_task_exit(void);
void l15_start(void) __attribute__((noreturn));
void l15_panic(void) __attribute__((noreturn));
void l15_finish(void) __attribute__((noreturn));
void l15_yield(void);
uint32_t l15_observe(uint32_t id, uint32_t registers_ok);
task_t *l15_schedule(void);
extern task_t *g_l15_current;
extern char l15_after_yield;
#endif
