**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — Complete the Reset Sequence

Exercise: [`../../exercises/01-reset-sequence/`](../../exercises/01-reset-sequence/)

Code: [`l11_reset.c`](l11_reset.c)

## Actual Output

```text
===== LESSON 11 SOLUTION 01 =====
[  g_l11_init] 0x11C0FFEE
[  g_l11_zero] 0x00000000
[  g_l11_phase] 0x00000123
[  vector[0]] 0x30008000
[  CHECK_DATA_INIT] PASS
[  CHECK_BSS_ZERO] PASS
[  CHECK_PHASE_ORDER] PASS
[  CHECK_VECTOR0_EQ_STACK_TOP] PASS
===== END =====
```

Identical in all three: `debug`, `release`, `minsize`.

## 1) `.data`

```c
const uint32_t *src = &__etext;          /* LMA: FLASH'taki ilk degerler */
uint32_t *dst = &__data_start;           /* VMA: RAM'deki degiskenler   */
while (dst < &__data_end) {
    *dst++ = *src++;
}
```

- The name `__etext` suggests “end of `.text`,” but its linker-script
  definition is `LOADADDR(.data)`: `.data`'s **load address**. Measured in Lesson 10.
- The linker places `.data` in FLASH and RAM with **the same internal layout**;
  a single linear copy therefore suffices.
- Pointer arithmetic here is **deliberately in words**: `dst++` advances
  4 bytes; `dst < &__data_end` is a pointer comparison. This is correct only
  because boundaries are aligned to 4, guaranteed by the linker script's
  `ALIGN(4)`. (Do not confuse this with Lesson 10's byte/element trap:
  there we calculated a **size**; here we **walk** word by word.)

## 2) `.bss`

```c
uint32_t *dst = &__bss_start;
while (dst < &__bss_end) {
    *dst++ = 0u;
}
```

`.bss` has no FLASH copy: all we need to do is write zeros.

## Order

`.data` and `.bss` are disjoint ranges; neither touches the other. Their
relative order therefore does not technically matter. What matters is
finishing **both before** the C code (`SystemInit`, `main`):

- `SystemInit` is a C function; it may depend on a variable in `.data`/`.bss`.
- `main` certainly does.

This solution follows the shared startup's order: `.data` → `.bss` →
`SystemInit` → `main`. This is **repository design**, not an architectural rule.

## Why Is the Phase Record Safe Despite Being in `.bss`?

`g_l11_phase` is in `.bss`. Its value accumulates in a **local** variable
in Reset_Handler, and is written to `g_l11_phase` only **after** both loops
and `SystemInit`. Neither poisoning nor `.bss` zeroing can erase it — even
with a missing `.bss` step (starter and broken target).

## Why Poison RAM?

QEMU starts RAM at zero. Without poisoning, a solution missing the `.bss`
step would also obtain `CHECK_BSS_ZERO PASS` — L11b case C in
`tests/build-probes.sh` measures this on every build. `l11_poison_ram()`
is an **educational instrument**; it is absent from shared startup and
should remain absent.

---

[← Lesson 11](../../README.md)
