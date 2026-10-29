**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — `sub_numbers`

**Goal:** implement the AAPCS contract yourself.

## Task

Complete the function in `sub_numbers.S`:

```c
int sub_numbers(int a, int b);    /* a - b */
```

It currently returns a constant `0`. Questions to answer:

- Which registers carry the arguments?
- Where should you put the result?
- How do you return?

## Build & Run

```bash
cmake --build --preset debug --target 03-exercise-sub
./scripts/run-qemu.sh build/debug/lessons/03-c-and-assembly/03-exercise-sub.elf
```

## Verification

| Test | Expected |
|---|---|
| `sub_numbers(12, 5)` | `7` |
| `sub_numbers(5, 12)` | `-7` |

Both `CHECK` lines must say `PASS`. The console prints unsigned values in the
second test — **predict what you will see before running**.

## Hint

One arithmetic instruction + one return instruction suffice. For the subtraction
instruction's name, see "SUB (register)" in the Armv8-M ARM.

After writing your code, inspect what it produces:

```bash
arm-none-eabi-objdump -d build/debug/lessons/03-c-and-assembly/03-exercise-sub.elf \
    | grep -A2 "<sub_numbers>:"
```

## If you are stuck

Reference solution: [`../../solutions/01-sub-numbers/`](../../solutions/01-sub-numbers/)

---

[← Lesson 03](../../README.md)
