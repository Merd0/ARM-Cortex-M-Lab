#include "../../kernel.h"

uint32_t *l15_initial_frame(uint32_t *top, uint32_t id, uintptr_t entry, uintptr_t exit)
{
    /* TODO: build software R4-R11/EXC_RETURN and a basic hardware frame.
     * Keep the public exception boundary 8-byte aligned. No scheduler changes. */
    (void)top; (void)id; (void)entry; (void)exit;
    return NULL; /* Rejected safely before installing a task PSP. */
}
