**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — `sub_numbers`

Exercise: [`../../exercises/01-sub-numbers/`](../../exercises/01-sub-numbers/)

## Build & Run

```bash
cmake --build --preset debug --target 03-solution-sub
./scripts/run-qemu.sh build/debug/lessons/03-c-and-assembly/03-solution-sub.elf
```

## Actual output

```text
===== LESSON 03 SOLUTION 01 =====
[SUB_NUMBERS(12,5)] 7
[SUB_NUMBERS(5,12)] 4294967289
[CHECK_12_MINUS_5] PASS
[CHECK_5_MINUS_12] PASS
===== END =====
```

## Generated code (measured)

```text
10000040 <sub_numbers>:
10000040:  1a40    subs  r0, r0, r1
10000042:  4770    bx    lr
```

Two instructions, four bytes. Exactly the same structure as `add_numbers` — only
the arithmetic instruction changed. Nothing differs in AAPCS: arguments in
`r0`/`r1`, result in `r0`, return through `lr`.

## Why `4294967289`?

`4294967289` = `0xFFFFFFF9`. This is the 32-bit two's-complement representation
of `-7`. The register is not holding an incorrect value; it appears this way
because we print it as **unsigned** using `console_kv_u32()`.

The real lesson: a register holds a 32-bit pattern. Being "signed" or "unsigned"
is a property of **the code reading it**, not the register. The processor does
not distinguish the two when executing `subs`; branch instructions such as
`BGE` (signed) and `BHS` (unsigned) make the distinction.

## `subs` or `sub`?

Both produce the correct result; the difference is whether they update APSR flags
(N/Z/C/V). Nobody reads the flags here, so both work.

`subs` was chosen because it fits a 16-bit encoding with low registers (`0x1a40`,
2 bytes). But this is not a rule; it is a measured result for these operands —
different operands can lead the assembler to choose a different encoding.
The right habit is to inspect with `objdump` rather than memorize.

---

[← Lesson 03](../../README.md)
