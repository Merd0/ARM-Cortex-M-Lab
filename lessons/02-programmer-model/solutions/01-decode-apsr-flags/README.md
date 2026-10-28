**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — `decode_apsr_flags()`

Reference solution. Exercise: [`../../exercises/01-decode-apsr-flags/`](../../exercises/01-decode-apsr-flags/)

## Build & Run

```bash
cmake --build --preset debug --target 02-solution-apsr
./scripts/run-qemu.sh build/debug/lessons/02-programmer-model/02-solution-apsr.elf
```

## Actual output

```text
--- xpsr flags ---
[APSR_AFTER_5_MINUS_5] 0x60000000
[FLAGS] N=0 Z=1 C=1 V=0
[APSR_AFTER_5_MINUS_7] 0x80000000
[FLAGS] N=1 Z=0 C=0 V=0
```

## Reading the output

`5 - 5 = 0` → `Z = 1` is expected. But `C = 1` also appears, surprising many
learners: in Arm subtraction, `C` means **"no borrow occurred"**, not "carry."
Since `5 >= 5`, there is no borrow and `C = 1`.

`5 - 7 = -2` → `N = 1` is expected. `C = 0` because a borrow was needed this time
(`5 < 7`). `V = 0` because there is no signed overflow — `-2` easily fits in the
32-bit signed range.

Confusing `V` and `C` is common: `C` is meaningful for unsigned comparisons,
`V` for signed comparisons. Branches such as `BHI`/`BLS` inspect `C`, whereas
`BGT`/`BLT` inspect `N` and `V`.

## Design note

Flags were extracted using shifts + masks, not C bitfields. The reason: the C
standard does not guarantee bitfield layout in memory (implementation-defined).
Masking + shifting is the portable method for reading hardware registers.

---

[← Lesson 02](../../README.md)
