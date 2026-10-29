**Language:** English | [Türkçe](README.tr.md)

# Solution 02 — `multiply_add` (reference solution for Experiment 1)

```c
int multiply_add(int a, int b, int c);    /* a * b + c */
```

## Build & Run

```bash
cmake --build --preset debug --target 03-solution-mla
./scripts/run-qemu.sh build/debug/lessons/03-c-and-assembly/03-solution-mla.elf
```

## Actual output

```text
===== LESSON 03 SOLUTION 02 =====
[MULTIPLY_ADD(3,4,5)] 17
[MULTIPLY_ADD(7,7,1)] 50
[CHECK_3x4p5] PASS
===== END =====
```

## Generated code (measured)

```text
10000040 <multiply_add>:
10000040:  fb00 2001   mla  r0, r0, r1, r2
10000044:  4770        bx   lr
```

The answered question: **the third argument arrives in `r2`.** AAPCS places the
first four arguments in `r0`, `r1`, `r2`, `r3`, in order. `mla r0, r0, r1, r2`
uses this in one instruction: `r0 = r0 * r1 + r2`.

## Two different approaches

| Approach | Instructions | Size | Architecture |
|---|---|---|---|
| `muls` + `adds` | 2 instructions | 2 + 2 bytes | Everywhere, including Armv6-M |
| `mla` | 1 instruction | 4 bytes | Armv7-M and later |

`MLA` was chosen deliberately to make the Lesson 01 question concrete:
`MLA` exists on Cortex-M33 but **not** on Cortex-M0/M0+ (Armv6-M). Compiling the
same C code for M0+ forces the compiler to produce the two-instruction approach.

A notable detail: `mla` has a **32-bit** encoding (`fb00 2001`), whereas `adds`
is 16-bit (`1840`). So "one instruction" does not always mean "fewer bytes."
Here 1 instruction = 4 bytes, 2 instructions = 4 bytes — the same size, with
the difference on the execution side.

`MLA` does not update flags and has no version with an `s` suffix.

---

[← Lesson 03](../../README.md)
