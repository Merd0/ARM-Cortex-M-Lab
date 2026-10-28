**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — `decode_apsr_flags()`

**Goal:** turn a raw `APSR` value into a readable line of flags.

## Task

Complete `decode_apsr_flags()` in `main.c`. It must produce this line:

```text
[FLAGS] N=0 Z=1 C=1 V=0
```

Bit positions (Armv8-M ARM, "APSR"):

| Bit | Flag | Meaning |
|---|---|---|
| 31 | N | Negative — the result is negative |
| 30 | Z | Zero — the result is zero |
| 29 | C | Carry — "no borrow" for subtraction |
| 28 | V | Overflow — signed overflow |

## Build & Run

```bash
cmake --build --preset debug --target 02-exercise-apsr
./scripts/run-qemu.sh build/debug/lessons/02-programmer-model/02-exercise-apsr.elf
```

## Verification

| Operation | Expected |
|---|---|
| `5 - 5` | `Z = 1` |
| `5 - 7` | `N = 1` |

Write down your expectations for `C` and `V` **before running**. If the result
differs, investigate how the corresponding flag is generated.

## Hint

`console_puts()` and `console_putc()` suffice. `'0' + bit` converts a bit directly
to a character. No new helper function is needed.

## If you are stuck

Reference solution: [`../../solutions/01-decode-apsr-flags/`](../../solutions/01-decode-apsr-flags/)

---

[← Lesson 02](../../README.md)
