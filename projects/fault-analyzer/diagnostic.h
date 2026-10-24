#ifndef LAB_DIAGNOSTIC_H
#define LAB_DIAGNOSTIC_H
#include "../../lessons/08-faults/example/fault_snapshot.h"
/* This conservative capture supports same-state basic frames only.
 * low/high are trusted exclusive bounds from the kernel, never from a task. */
int fa_capture(fault_snapshot_t *snapshot, uintptr_t address, uintptr_t low, uintptr_t high);
void fa_analyze(const fault_snapshot_t *snapshot, fault_decision_t *decision);
#endif
