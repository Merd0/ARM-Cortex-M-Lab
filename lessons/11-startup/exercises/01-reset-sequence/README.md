**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Complete the Reset Sequence

**Goal:** Establish the environment C assumes on the path from reset to `main`.

## Task

[`l11_reset.c`](l11_reset.c) is a `Reset_Handler` skeleton. Already provided:

- the vector table (`example/l11_vectors.c`),
- poisoning (`l11_poison_ram()` — **educational test instrument**: fills RAM
  with `0xA5A5A5A5` to imitate “undefined RAM” on physical hardware;
  do not remove it),
- phase recording and calls to `SystemInit()` and `main()`.

Two steps are missing — the `TODO(student)` lines in the file:

1. Move `.data`'s initial values into place.
2. Zero `.bss`.

Use only the linker symbols declared in `fixtures.h`:
`__etext`, `__data_start`, `__data_end`, `__bss_start`, `__bss_end`.
**Do not hard-code addresses.** Do not change the phase records' positions or order.

## Build & Run

```bash
cmake --build --preset debug --target 11-exercise-startup
./scripts/run-qemu.sh build/debug/lessons/11-startup/11-exercise-startup.elf
```

Initial output (actual):

```text
===== LESSON 11 EXERCISE 01 =====
[  g_l11_init] 0xA5A5A5A5
[  g_l11_zero] 0xA5A5A5A5
[  g_l11_phase] 0x00000123
[  vector[0]] 0x30008000
[  CHECK_DATA_INIT] FAIL
[  CHECK_BSS_ZERO] FAIL
[  CHECK_PHASE_ORDER] PASS
[  CHECK_VECTOR0_EQ_STACK_TOP] PASS
===== END =====
```

This is the **expected initial state**. The console works even with both
steps missing: UART and console do not depend on `.data`/`.bss` (measured).

## Verification

All four checks must `PASS`: `g_l11_init = 0x11C0FFEE`, `g_l11_zero = 0`,
phase order `0x123`.

## Questions to Answer

1. In which memory and at which address are `.data`'s initial values?
   Where is the variable itself?
2. Which symbol means “where the initial values begin”? Why is its name
   misleading? (See Lesson 10.)
3. How many bytes must be copied? Which two symbols give this number?
4. Why is it safe to work in words (4 bytes)? How does the linker script
   guarantee this?
5. Does it matter whether `.data` is copied before `.bss` is zeroed, or vice versa?
6. Without poisoning, what would a solution omitting the `.bss` step report
   in QEMU? Why?

## If You Are Stuck

Reference solution: [`../../solutions/01-reset-sequence/`](../../solutions/01-reset-sequence/)

---

[← Lesson 11](../../README.md)
