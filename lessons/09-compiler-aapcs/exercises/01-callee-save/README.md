**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Repair an ABI-Broken Assembly Function

**Goal:** Repair an assembly function that returns the right result but violates
the AAPCS contract.

## Task

[`scale_and_sum.S`](scale_and_sum.S):

```c
uint32_t scale_and_sum(uint32_t a, uint32_t b);   /* helper_scale(a) + b */
```

`helper_scale` is a C function (`example/helpers.c`). `scale_and_sum` calls it,
so it is **not a leaf**.

The function returns **the right result**, but the harness finds two violations:

| Check | Initial state |
|---|---|
| `CHECK_RESULT` | PASS |
| `CHECK_CALLEE_SAVED` | **FAIL** — the harness found a corrupted register |
| `CHECK_SP_ALIGNED_AT_CALL` | **FAIL** — SP is not a multiple of 8 at `BL` |

**Edit only `scale_and_sum.S`.** Do not change `main.c` or the harness.
Do not remove the SP recording immediately before `BL` (`g_ex_sp_at_call`);
the check uses it.

## Build & Run

```bash
cmake --build --preset debug --target 09-exercise-callee-save
./scripts/run-qemu.sh build/debug/lessons/09-compiler-aapcs/09-exercise-callee-save.elf
```

Initial output (actual, `debug`):

```text
[  CHECK_HARNESS_POSITIVE_CONTROL] PASS
[  RESULT] 36
[  MASK] 0x00000002
[  CORRUPTED] r5
[  SP_AT_BL] 0x30007FAC
[  CHECK_RESULT] PASS
[  CHECK_CALLEE_SAVED] FAIL
[  CHECK_SP_ALIGNED_AT_CALL] FAIL
===== END =====
```

This is the **expected initial state**. Addresses differ in `release` and
`minsize`, but verdicts are identical.

## Verification

All three checks must be `PASS`, with `RESULT` still `36`.

## Questions to answer

1. Which register holds `b` across the call? Under AAPCS, who owns that
   register? Who must preserve it?
2. Why is `SP_AT_BL` not a multiple of 8? SP was aligned to 8 at function
   entry (the harness guarantees this). How many bytes were stacked between
   entry and `BL`?
3. Each register added to a `push` list occupies 4 bytes. If the total must
   be a multiple of 8 and you need to preserve an odd number of registers,
   what do you do? (Hint: examine the harness's own `push` list.)
4. Why does the function save LR? What would happen after `helper_scale`
   if it did not?
5. Why return with `pop {..., pc}` rather than `bx lr`? Are both correct?

Questions 4 and 5 are not tested at runtime — an incorrect return would cause
a crash or infinite loop, not a controlled FAIL. Answer them from disassembly.

## If you are stuck

Reference solution: [`../../solutions/01-callee-save/`](../../solutions/01-callee-save/)

---

[← Lesson 09](../../README.md)
