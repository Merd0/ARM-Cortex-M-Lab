**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — Locate Your Data

Exercise: [`../../exercises/01-locate-your-data/`](../../exercises/01-locate-your-data/)

Code: [`locate.c`](locate.c)

## Actual output

| Preset | `FLASH_USED_BYTES` | `RAM_STATIC_BYTES` | `G_INIT_LMA` | Checks |
|---|---|---|---|---|
| `debug` | 1424 | 8 | `0x1000058C` | 4/4 PASS |
| `release` | 1584 | 8 | `0x1000062C` | 4/4 PASS |
| `minsize` | 1184 | 8 | `0x1000049C` | 4/4 PASS |

Three presets, three numbers, same code. A hard-coded solution would pass
at most one of them.

Together, in order, all code blocks below form a compilable `locate.c`;
it reproduces the table's values in all three presets with 0 warnings
(`-Wall -Wextra`).

## First: why use `A()` for address arithmetic?

Linker symbols are declared in C as `extern uint32_t __data_start;`, making
`&__data_start` a `uint32_t *`. Direct subtraction of two `uint32_t *`
values (`&__data_end - &__data_start`) counts **elements, not bytes**
(4-byte words) — a result 4 times too small. `&__etext + n` likewise advances
by `n` **words**.

Each calculation therefore converts addresses to integers first. The code
below exactly matches [`locate.c`](locate.c):

```c
#include "l10.h"
#include "locate.h"

static uint32_t A(const void *p) { return (uint32_t)(uintptr_t)p; }
```

## `flash_used_bytes()`

```c
uint32_t flash_used_bytes(void)
{
    uint32_t ro_part   = A(&__etext) - A(&__vectors_start);   /* byte */
    uint32_t data_copy = A(&__data_end) - A(&__data_start);   /* byte */
    return ro_part + data_copy;
}
```

FLASH load image:

```text
__vectors_start (0x10000000)
    .vectors  .text  .rodata  (+ alignment)
__etext = LOADADDR(.data)
    initial values of .data
__flash_load_end
```

- `__etext` sounds like “end of `.text`,” but means `LOADADDR(.data)`:
  the end of the read-only portion **including** `.rodata` and alignment,
  where the `.data` copy begins.
- `.data` **also** occupies FLASH, which stores its initial values.
- `.bss` **occupies no FLASH**: its initial value is zero, with nothing to store.

The result matches the linker's own calculation:
`__flash_load_end − ORIGIN(FLASH)`. The linker script already defines
this symbol for an `ASSERT`.

## `ram_static_bytes()`

```c
uint32_t ram_static_bytes(void)
{
    return (A(&__data_end) - A(&__data_start)) +
           (A(&__bss_end)  - A(&__bss_start));
}
```

`.data` also occupies RAM (VMA). Stack is **excluded**: it is not a section,
but linker-script accounting (`__stack_size = 4K`, `__stack_top`,
`__stack_limit`). `--print-memory-usage` does not count it either.

## `data_lma_of(p)`

```c
uint32_t data_lma_of(const void *p)
{
    return A(&__etext) + (A(p) - A(&__data_start));
}
```

`p` is a `const void *`; `&__data_start` is a `uint32_t *`. Direct subtraction
does not even compile. Obtain the byte offset using integer addresses.

The linker places `.data` in RAM (VMA) and FLASH (LMA) with **the same
internal layout**: an object's offset from the `.data` start is identical
in both places. That is exactly why startup needs only one linear copy loop
(`__etext` → `__data_start`, through `__data_end`).

The check **reads** this address and finds `0xC0DEFACE`, independently
verifying the calculation.

## `in_region()`

```c
int in_region(const void *p, uint32_t origin, uint32_t length)
{
    uint32_t a = A(p);
    return (a >= origin) && ((a - origin) < length);
}
```

`(a − origin) < length` replaces `a < origin + length`: it remains correct
for a region whose `origin + length` could overflow 32 bits (for example,
`0xF0000000` + `0x20000000`). It makes no difference for this repository's
regions, but is the right habit.

---

[← Lesson 10](../../README.md)
