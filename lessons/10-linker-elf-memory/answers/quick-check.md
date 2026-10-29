**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 10 — Quick Check Answers

> Write your own answer before opening this file. Numbers come from the
> `debug` build (**OBSERVATION**); rules are labeled.

---

### 1. What distinguishes `.o` from `.elf`? What does a symbol value in `.o` mean?

**ELF:** both are ELF files, but of different types:

- **Relocatable (`.o`, `ET_REL`):** contains sections, symbols, and
  relocations. Sections have no final addresses yet.
- **Executable (`.elf`):** sections have actual addresses, symbols are
  resolved, relocations applied, and **program headers** (segments) added
  for the loader.

A symbol value in `.o` belongs to one of three cases:

| Case | Value | Measured (`readelf -s l10_asm.S.obj`) |
|---|---|---|
| Undefined | Section `UND`, value `0` — no definition yet | `text_fn  00000000  NOTYPE  GLOBAL  UND` |
| Defined | **Offset from the beginning** of its defining section | `local_helper  0000000f  FUNC  LOCAL  Ndx 1` |
| In final ELF | Section's placed address + offset | `local_helper  1000004f` |

“Addresses in `.o` are always `0`” is **wrong**. `local_helper` lies at
`.text` offset `0x0e` (`nm`: `0e`; `readelf -s`: `0f`, because bit 0 is
Thumb state). In ELF, the `.text` input section is at `0x10000040`:
`0x10000040 + 0x0e`.

Many values here are `0` because of `-ffunction-sections -fdata-sections`:
each function/object starts its own section. In `l10_asm.S`, two functions
share `.text`, so the second is at `0x0e`.

Measurement: in `main.c.obj`, `bl 0 <text_fn>` (offset `2a4` in `.text.main`);
in ELF, the same instruction is `10000334: bl 10000494 <text_fn>`.

---

### 2. How do a symbol and a relocation differ?

- **Symbol:** a **name** for an address, plus binding/status: LOCAL / GLOBAL,
  defined / undefined.
- **Relocation:** an instruction to **patch** bytes at a location using a
  symbol's address and a specified formula.

In `main.o`:

- `U text_fn` → “I use something named `text_fn`, but do not define it.”
- `R_ARM_THM_CALL text_fn` @ `0x2a4` → “Once the address of `text_fn` is known,
  fill the `BL` at offset `0x2a4` in `.text.main` using AAELF32's
  `((S + A) | T) – P` formula.”

One identifies **what** is missing; the other specifies **where and how** to fill it.

---

### 3. How do sections and segments differ?

| | Section | Segment |
|---|---|---|
| Whose view | Linker (linking view) | Loader (loading view) |
| In ELF | Section header table | Program header table |
| Example | Separate `.data`, `.bss` | Both in one `LOAD` segment |

Measurement (`readelf -l`): two `LOAD` segments. The first contains
`.vectors .text .rodata` (FLASH, `R E`); the second `.data .bss`
(`VirtAddr 0x30000000`, `PhysAddr 0x100009a8`, `RW`). `.o` files have no segments.

---

### 4. (Reasoning) Why do `.data`'s VMA and LMA differ?

`.data` must satisfy two conflicting needs:

- **Persistent initial value** → FLASH, surviving power loss
  (**LMA** `0x100009A8`).
- **Writable variable** → RAM (**VMA** `0x30000000`).

Code always accesses a `.data` variable via **VMA**. Startup copies from
LMA to VMA at the beginning of execution (Lesson 11).

If both were **identical**, for example `.data` in FLASH, writes would be
impossible (ordinary stores cannot program FLASH). If both were in RAM,
initial values would not be present at power-on.

Here, QEMU loaded `.data` **only at LMA**: at the first `Reset_Handler`
instruction, measurements found `g_init = 0` in RAM and `0xC0DEFACE` in FLASH.

---

### 5. How do `.data > RAM AT > FLASH` and `.data : AT(__etext) > RAM` differ?

**LINKER (GNU ld):**

- `AT > FLASH`: assigns LMA by **allocating within the FLASH region**;
  these bytes count toward FLASH usage. Overflow → `region FLASH overflowed`.
- `AT(__etext)`: specifies LMA as **an address**, charged to no region.
  Linking may **silently succeed** even if `.text` + `.rodata` + the `.data`
  copy exceeds FLASH.

**`AT > FLASH` catches overflow.** An earlier repository version used
`AT(__etext)`; an independent audit found this (I5), and `tests/build-probes.sh`
now tests it permanently. The script's second `ASSERT`
(`__flash_load_end <= ORIGIN(FLASH) + LENGTH(FLASH)`) expresses the same
condition with an explicit message.

---

### 6. (Reasoning) Why is “`.bss` is absent from ELF” misleading?

Because `.bss` is **described** in ELF; only its **contents are not stored**.

Measurement:

- A section header exists: `.bss  NOBITS  30000008  size 8  WA`.
  `NOBITS` means “no file contents”; `A` means “occupies memory.”
- Second `LOAD` segment: `FileSiz 0x8`, `MemSiz 0x10`. The 8-byte difference
  is `.bss`: memory with no corresponding content in the file.
- 8 bytes of RAM at runtime.
- **0** bytes of FLASH.

Correct statement: *“`.bss` has a size and address in ELF, but no stored bytes.”*

---

### 7. How does the map reveal where a symbol came from and where it went?

In the map's *Linker script and memory map* section:

```text
.rodata         0x100005e4      0x3c4              <- OUTPUT section
 .rodata.l10_table                                  <- INPUT section
                0x1000097c                k_custom   <- symbol, address
```

Each input-section line names the **contributing object file** (for example,
`.data.g_init ... objects.c.obj`). Three answers at a glance: which output
section, which input section, which `.o`.

Also inspect *Discarded input sections*, *Memory Configuration* (regions),
linker-symbol assignments (`__etext = LOADADDR (.data)`), and `ASSERT` lines.

---

### 8. Why does `KEEP` exist? Why does the vector table need it?

`--gc-sections` retains an input section only if **reachable from a root**
(entry point, references from other retained sections...). `KEEP(...)`
makes an input section **a root**.

No **code** references the vector table — the processor reads it at reset
(**ARCHITECTURE**). The linker cannot know that.

Measured in a temporary copy, not added to the repository: replacing
`KEEP(*(.vectors))` with `*(.vectors)` put `.vectors` in the map's *Discarded*
list, removed the `.vectors` output section from ELF, and the firmware
**never started**.

---

### 9. (Reasoning) Does `__attribute__((used))` protect a section from GC?

**Not with this toolchain (GCC 13.2.1 + binutils 2.42).**

- `used` tells the **compiler** to emit a definition even if it appears unused.
- `KEEP` tells the **linker** not to discard an input section during GC.

Measurement: `k_unused_used` (`used`) existed in `objects.o`
(`nm`: `R k_unused_used`), but `.rodata.k_unused_used` entered the *Discarded*
list during linking. The vector table also carries `used`, yet was discarded
without `KEEP` (question 8).

Both `used` + `KEEP` may be necessary: one prevents compiler removal, the
other linker removal. GCC also has `retain`; it was not tested here.

---

### 10. How effective is `--gc-sections` without `-ffunction-sections`?

Far less effective. GC operates on **input sections**. Without these flags,
a C file's functions share one `.text` input section; using **one** function
from the file retains them all.

Here, `unused_function` could be discarded because `placement.c` was compiled
with `-ffunction-sections`, placing it in `.text.unused_function`.

By contrast, `l10_asm.S` is not C, so `-ffunction-sections` does not apply.
Its code shares one `.text` input section.

---

### 11. What do `MEMORY` and `SECTIONS` do?

- **`MEMORY`:** defines physical regions — name, attributes, origin, size.
  Here: `FLASH (rx) 0x10000000 512K`, `RAM (rwx) 0x30000000 32K`. Overflowing
  a region causes a link error.
- **`SECTIONS`:** defines how input sections are gathered into output
  sections (wildcards), where they are placed (`> FLASH`, `> RAM AT > FLASH`),
  alignment, and linker symbols.

`MEMORY` says “where space exists”; `SECTIONS` says “what goes where.”

---

### 12. (Reasoning) ELF 94 KB, FLASH 2.5 KB — where is the difference? Why is FLASH larger than `.text`?

**ELF file size** (94 316 bytes) includes non-loaded material: `.debug_*`
sections (including `-g3` macro information), symbol and string tables,
section/program headers. None enters FLASH. The same source produces
27 964 bytes with `minsize` (`-g`).

**FLASH usage** (2480 bytes) exceeds `.text` (1444 bytes), because the load
image includes **all** of the following:

```text
.vectors      64
.text       1444
.rodata      964
.data copy  8     <- initial values of .data
-----------------
            2480  = __flash_load_end - 0x10000000
```

Berkeley `size`'s `text` column (2472) is also not `.text`: it totals
`.vectors` + `.text` + `.rodata`.

---

### 13. (Reasoning) “undefined reference to `asm_fn`,” although `asm_fn` is defined. How?

The definition **exists**, but is **LOCAL**.

```text
nm l10_asm_broken.o   ->  00000000 t asm_fn     (lowercase 't')
nm main.o             ->           U asm_fn
```

GAS labels are local unless marked `.global` / `.globl`. A local symbol is
visible only in its own object file. The `U` reference in `main.o` seeks
**a GLOBAL definition in another file**; finding none, the linker reports
“undefined reference.”

First response: inspect the **binding** with `nm` in the defining object.
Uppercase or lowercase?

(Other common causes: file omitted from the link, C++ name mangling,
a `static` definition, or a misspelled name.)

---

## References

- AAELF32 — relocation types and formulas, Thumb symbols, veneers
- GNU ld — *Linker Scripts*, `--gc-sections`, `-Map`, `--print-memory-usage`
- GNU binutils — `readelf`, `nm`, `objdump`, `size`
- System V gABI — sections and program headers

---

[← Lesson 10](../README.md) · [Lesson 11 →](../../11-startup/)
