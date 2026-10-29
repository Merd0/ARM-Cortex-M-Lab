**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Locate Your Data

**Goal:** Calculate memory placement solely from linker symbols and object
addresses, without consulting the map file.

## Task

Implement the four functions in [`locate.c`](locate.c):

| Function | Must return |
|---|---|
| `flash_used_bytes()` | **Entire** FLASH load image size in bytes |
| `ram_static_bytes()` | **Static** RAM usage in bytes, excluding stack |
| `data_lma_of(p)` | `p` is an object in `.data`; return the FLASH address of its **initial value** |
| `in_region(p, origin, length)` | Whether `p` lies in `[origin, origin + length)` |

Available symbols are declared in `example/l10.h` and defined in
`platform/mps2-an505/linker/mps2-an505-secure.ld`.

**Rule:** do not copy numbers from the map. Your code must remain correct
in another build — the reference solution already has different numbers
from the main example.

## Build & Run

```bash
cmake --build --preset debug --target 10-exercise-linker
./scripts/run-qemu.sh build/debug/lessons/10-linker-elf-memory/10-exercise-linker.elf
```

Initial output (actual):

```text
[  FLASH_USED_BYTES] 0
[  RAM_STATIC_BYTES] 0
[  G_INIT_LMA] 0x00000000
[  CHECK_FLASH_USED] FAIL
[  CHECK_RAM_STATIC] FAIL
[  CHECK_LMA_HOLDS_INIT] FAIL
[  CHECK_REGIONS] FAIL
===== END =====
```

This is the **expected initial state**.

## Verification

All four checks must be `PASS`.

Checks do not rely on fixed addresses:

- `CHECK_FLASH_USED` compares your result with a symbol calculated by the linker.
- `CHECK_LMA_HOLDS_INIT` **reads** your returned address and looks for the
  initial value of `g_init` (`0xC0DEFACE`) — but only if the address lies
  inside the FLASH load image, is at least 4 bytes before its end, and is
  aligned to 4 bytes. Otherwise, it **does not read**. The check is safe
  against 32-bit overflow, including values such as `0xFFFFFFFC`: a wrong
  answer produces FAIL, not a fault.

## Questions to answer

1. Which bytes occupy FLASH? Only code?
2. Does `.data` also occupy FLASH? Why?
3. Does `.bss` occupy FLASH?
4. What does the name `__etext` suggest, and what is its actual definition?
   (Inspect the linker script.)
5. How does an object's position within RAM `.data` relate to its position
   in the FLASH copy?
6. Why is stack excluded from `ram_static_bytes()`?

## If you are stuck

Reference solution: [`../../solutions/01-locate-your-data/`](../../solutions/01-locate-your-data/)

---

[← Lesson 10](../../README.md)
