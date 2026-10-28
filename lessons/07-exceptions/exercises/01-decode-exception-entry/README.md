**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Decode an Exception Entry

**Goal:** work backwards from exception entry to answer "what happened?"

## Task

Complete `decode_exception_entry()` in `main.c`. It takes three inputs and fills six fields:

```c
static void decode_exception_entry(const exception_frame_t *frame,
                                   uint32_t exc_return,
                                   uint32_t ipsr,
                                   entry_info_t *out);
```

| Field | What to write | Source |
|---|---|---|
| `interrupted_mode` | `"Thread"` / `"Handler"` | `EXC_RETURN` |
| `frame_stack` | `"PSP"` / `"MSP"` | `EXC_RETURN` |
| `exception_number` | Number of the executing exception | `ipsr` |
| `stacked_r0` | Frame's `R0` slot | `frame` |
| `stacked_pc` | Frame's `PC` slot | `frame` |
| `stacked_xpsr` | Frame's `xPSR` slot | `frame` |

## Scope

Only the subset supported by this repository: **standard 8-word frame, one
security state**. You are not asked to reimplement every `EXC_RETURN` rule —
two bits suffice.

## ⚠️ Pitfall

`frame_stack` and *"which stack does the handler use?"* are **not the same question**.

- The first comes from `EXC_RETURN` bit 2 and says **where the frame was stacked**.
- The answer to the second is **always MSP**.

## Build & Run

```bash
cmake --build --preset debug --target 07-exercise-decode
./scripts/run-qemu.sh build/debug/lessons/07-exceptions/07-exercise-decode.elf
```

Actual starter output:

```text
[4] Exception girisi cozumu
[  INTERRUPTED_MODE] ?
[  FRAME_STACK] ?
[  EXCEPTION_NUMBER] 0
[  STACKED_R0] 0x00000000
[  CHECK_DECODE_MODE] FAIL
[  CHECK_DECODE_STACK] FAIL
[  CHECK_DECODE_EXCNUM] FAIL
[  CHECK_DECODE_R0] FAIL
[  CHECK_DECODE_PC] FAIL
[  CHECK_DECODE_XPSR] FAIL
```

The firmware runs correctly; only decoding is empty. This is the **expected
starting state**, not a bug.

## Verification

All six `CHECK_DECODE_*` checks must become `PASS` **and** every other `CHECK_*`
line must remain `PASS`.

Expected values (`svc #7`, PSP-origin):

```text
INTERRUPTED_MODE  Thread
FRAME_STACK       PSP
EXCEPTION_NUMBER  11
STACKED_R0        0x11111111
```

## Verify with your own measurement

```gdb
break svc_dispatch
continue
next 3
cm-exc-return g_entry_lr
cm-frame g_frame_addr g_entry_lr
```

Do your function's results match what `cm-frame` prints?

## Optional (DEEP) — Exercise 02

Classify these `EXC_RETURN` values: Thread or Handler, MSP or PSP, basic or
extended, and whether this lab's `cm-frame` helper supports them.

```text
0xFFFFFFF9    0xFFFFFFFD    0xFFFFFFED    0xFFFFFFF1    0xFFFFFFF5
```

Be careful with the last one. Answers are at the end of `answers/quick-check.md`.

## If you are stuck

Reference solution: [`../../solutions/01-decode-exception-entry/`](../../solutions/01-decode-exception-entry/)

---

[← Lesson 07](../../README.md)
