**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Activate PSP Correctly

**Goal:** move Thread Mode to its own stack — what RTOSes do every day.

## Task

The file you complete is **assembly, not C**:

```text
exercises/01-activate-psp/psp_start.S
```

```asm
psp_start:                      @ r0 = psp_top, r1 = entry  — DONMEZ
    /* TODO(student): 1) msr psp  2) CONTROL.SPSEL = 1  3) isb */

    ldr     lr, =psp_entry_trap
    orr     lr, lr, #1
    bx      r1
```

`main.c` is ready: it initializes the stack block and guard words, calls
`psp_start()`, and does not return. `thread_entry()` performs the checks.
You write only the three transition steps.

### Why not C?

Because changing `CONTROL.SPSEL` midway through an ordinary C function is
**invalid**. The compiler allocated its frame on MSP and emitted local accesses
using `[sp, #offset]`; changing `SPSEL` makes those offsets refer to another
address. See the explanation at the top of `psp_start.S`.

### Questions to answer

- For a full-descending stack, which address belongs in PSP — `&block[0]`,
  `&block[127]`, or `&block[128]`? Why?
- How do you update `CONTROL` without overwriting `nPRIV` (bit 0)?
- Which instruction is required afterwards, and why **here**?
  (Hint: calling `__set_CONTROL()` from C would be different.)

## Build & Run

```bash
cmake --build --preset debug --target 04-exercise-psp
./scripts/run-qemu.sh build/debug/lessons/04-stack-msp-psp/04-exercise-psp.elf
```

## Verification

| Check | What it examines |
|---|---|
| `CHECK_SPSEL` | Are we in Thread Mode, and is `CONTROL.SPSEL` really 1? |
| `CHECK_REGION` | Are local variables now inside `task_stack_block`? |
| `CHECK_STACK_GUARD` | Are the guard words above the allocated area intact? |

All three must say `PASS`. Initially, the first two report `FAIL` — the
**expected** starter state, not a bug.

Limit of `CHECK_STACK_GUARD`: it establishes only that **those two monitored
locations** were not overwritten. It does not catch overflow below the lower
bound or stray writes elsewhere. Check the stack pointer separately with GDB:

```gdb
p/x $sp     p/x $msp     p/x $psp     p/x $control
p/x task_stack_block[128]
```

Which value equals `$sp`? Does `$psp` always remain within
`0x30000000 .. 0x30000200`?

## Common mistake

Reversing the two steps. What happens if you set `SPSEL = 1` before setting
PSP? Predict before trying — the hint is in Lesson 02's Break It.

## Optional (DEEP) — Exercise 02

With PSP active, make three nested function calls and record `PSP` at each
level. How many bytes did each level consume? What changes in `release`?
There is no reference solution — your measurement is the answer.

## If you are stuck

Reference solution: [`../../solutions/01-activate-psp/`](../../solutions/01-activate-psp/)

---

[← Lesson 04](../../README.md)
