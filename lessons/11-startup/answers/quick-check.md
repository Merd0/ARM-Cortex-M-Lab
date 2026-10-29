**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 11 — Quick Check Answers

> Write your own answer first, then open this. Numbers are from the `debug`
> build; every claim is labeled.

---

### 1. What are `vector[0]` and `vector[1]` used for at reset?

- **`vector[0]`**: initial MSP value. An **address value**, not a function.
  In this repository, `&__stack_top` = `0x30008000`. (**ARCHITECTURE** —
  Arm 107565: the vector table's first word is MSP's initial value.)
- **`vector[1]`**: reset vector. PC is loaded with the reset handler's
  start address (DDI 0553 `RPLNS`), and `EPSR.T` comes from this value's
  bit 0 (`RKSCH`). Here, `Reset_Handler`, stored as `0x10000335` in the table.

The rest of the table (2–15 in this repository) contains two distinct types:

- **Handler entries** — 2 NMI, 3 HardFault, 4 MemManage, 5 BusFault,
  6 UsageFault, 7 SecureFault, 11 SVCall, 12 DebugMonitor, 14 PendSV,
  15 SysTick. Each carries a handler **symbol**'s address.
- **Reserved entries** — 8, 9, 10, 13. **Constant `0`** in this table;
  there are no underlying symbols.

Handler behavior belongs to Lesson 07.

---

### 2. (Reasoning) `Reset_Handler` begins with `push`. Why is it safe?

Because hardware loaded MSP from `vector[0]` **before the first instruction**
(**ARCHITECTURE**). GDB stop A: `MSP = 0x30008000 =
vector[0] = &__stack_top`, while PC is still at `Reset_Handler`'s first instruction.

`Reset_Handler` is a C function and the compiler supplied a prologue
(`push {r3,
lr}` — **TOOLCHAIN OBSERVATION**). Using the stack is safe; relying on
`.data`/`.bss` would **not** be safe — they are not ready yet. The stack
is not a section: it occupies the top of RAM, separate from `.data`/`.bss`.

---

### 3. `vector[1] = 0x10000335`, instruction address `0x10000334`. What differs?

Bit 0. The **entry value** stored in the table carries Thumb state; the
instruction itself occupies an **aligned** address (`0x10000334`). Reset
writes the address with bit 0 cleared to PC and bit 0 to `EPSR.T` (`RKSCH`).

If bit 0 = 0, then `EPSR.T = 0`. DDI 0553 `RSQLX`: attempting to execute
any instruction in this state causes an **INVSTATE UsageFault** on a PE
with the Main Extension. At reset this is unrecoverable — the program
never starts.

Saying “Reset_Handler has just one address” would be **incorrect**.

---

### 4. (Reasoning) Would deleting `ENTRY(Reset_Handler)` change reset?

**No.** `ENTRY` sets `e_entry` in the ELF header (**ELF / LINKER**).
Debuggers and some loaders read it. At reset, the Cortex-M processor
**does not read** `e_entry`; it takes PC from the vector table
(`vector[1]`) (**ARCHITECTURE**).

Deleting `ENTRY` may only change `e_entry` (GNU ld applies its default
rule). As long as `KEEP` retains the vector table at `0x10000000`, reset
behavior remains the same. Measurement: `readelf -h` → `Entry point address`
= `Reset_Handler` symbol value; the processor never reads it.

---

### 5. Why copy `.data` and zero `.bss`?

C assumes:

- `uint32_t g_l11_init = 0x11C0FFEE;` → value `0x11C0FFEE` when the program starts.
- `uint32_t g_l11_zero;` → `0` when the program starts.

Variables live in RAM so they can be written. Initial values live in FLASH
so they survive power loss. **Hardware does not connect the two**, and
RAM contents after reset are undefined on physical hardware.

Startup therefore:

- copies `.data` from `__etext` (LMA) to `__data_start`..`__data_end` (VMA),
- zeros `.bss` between `__bss_start`..`__bss_end`.

Shared `Reset_Handler` does this here (**REPOSITORY DESIGN**).

---

### 6. (Reasoning) `g_l11_zero` is zero in `main`. Is that proof?

**No.** QEMU starts RAM at zero (**QEMU OBSERVATION**): the value would be
zero even if the zeroing loop never ran. `tests/build-probes.sh` (L11b)
permanently demonstrates this: startup **without** a `.bss` loop reports
`CHECK_BSS_ZERO PASS` when poisoning is disabled.

Two ways to establish actual proof:

1. **GDB (shared startup):** inject `g_l11_zero = 0xCAFEBABE` before the
   loop, `watch g_l11_zero`, continue. Measured: `Old value =
   3405691582 → New value = 0`, written by `Reset_Handler+28: str.w r2,
   [r3], #4`.
2. **Poisoning (lesson-local startup):** first fill RAM with `0xA5A5A5A5`.
   If the loop is missing, `g_l11_zero = 0xA5A5A5A5` → FAIL.

---

### 7. Why can startup copy word by word?

The linker script aligns the start and end of `.data` and `.bss` to 4
(`. = ALIGN(4)` immediately before assignments to `__data_start`, `__data_end`,
`__bss_start`, `__bss_end`). Thus:

- start addresses are word-aligned,
- lengths are multiples of 4,
- `dst < __data_end` / `dst < __bss_end` stops exactly at the boundary.

Without aligned boundaries, a word loop would skip the last 1–3 bytes or
overrun into neighboring memory. This is part of the startup/linker-script
contract (**REPOSITORY DESIGN** + **LINKER**). Word copying is not universal.

---

### 8. What does `SystemInit` do here? Does it change VTOR?

The **strong** `SystemInit` in `platform/mps2-an505/src/system_init.c`
replaces startup's empty **weak** definition (map: weak definition *Discarded*).
It does two things:

1. `SCB->VTOR = 0x10000000`. Measured before and after: `0x10000000`.
   **No visible change**; the address is explicitly established.
2. `board_uart_init()`. Measured: UART `BAUDDIV` `0` → `173`. This is
   the observable effect; `SystemInit` is the only caller of `board_uart_init`.

No clock/PLL setup.

---

### 9. (Reasoning) SP mod 8 = 0 and `SP = __stack_top − 8`: which is a rule?

- **SP mod 8 = 0** when calling `main`: **ABI** rule (AAPCS32: SP aligned
  to 8 at a public interface). `Reset_Handler` is a C function and calls
  `main` following AAPCS.
- **`SP = __stack_top − 8`**: **TOOLCHAIN OBSERVATION**. GCC 13.2.1 gave
  `Reset_Handler` a `push {r3, lr}` (8 bytes). Another compiler or preset
  may generate a different prologue; ABI requires only alignment.

Measurement used the `main_entry.S` shim, before the C prologue executed.

---

### 10. What happens if `main` returns?

Execution enters the infinite loop at the end of `Reset_Handler`:

```text
0x10000362 <Reset_Handler+46>:  wfi
0x10000364 <Reset_Handler+48>:  b.n  0x10000362
```

`main`'s LR (`0x10000363`) points exactly at this `wfi`. `11-startup`
deliberately returns; GDB stop G demonstrates this path. No `exit()`,
no `argc/argv` — this is not hosted C (**REPOSITORY DESIGN**).

---

### 11. Why does `NMI_Handler` share `Default_Handler`'s address?

Startup defines it with `__attribute__((weak, alias("Default_Handler")))`:
a second, **weak** name for the same code. With no other definition of
`NMI_Handler`, the linker uses the weak alias: `nm` → `W NMI_Handler`,
the same address as `Default_Handler`.

In `11-startup`, `main.c` defines a **strong** `SysTick_Handler`. The linker
selects it: `nm` → `T SysTick_Handler`, a different address, which
`vector[15]` carries with its Thumb bit. This is a **link-time** mechanism;
there is no `if` in the code.

This mechanism applies only to **handler entries** (2–7, 11, 12, 14, 15).
Reserved entries (8, 9, 10, 13) are constant `0` in the table; without
underlying symbols there is neither a weak alias nor anything to override.
`vector[0]` (a stack-pointer value) and `vector[1]` (`Reset_Handler`, a strong
definition) are also outside this group.

Measurement (`objdump -s -j .vectors`, three presets): `vector[8]`, `[9]`,
`[10]`, `[13]` = `0x00000000`.

---

### 12. (Reasoning) `LR = 0xFFFFFFFF` and `MSPLIM = 0`: which is architectural?

- **`LR = 0xFFFFFFFF`**: **ARCHITECTURE**. DDI 0553 `RPLRT`: with the Main
  Extension, LR becomes `0xFFFFFFFF` on Warm reset. Cortex-M33 has the
  Main Extension. QEMU also shows this — but Arm is the source.
- **`MSPLIM = 0`**: **QEMU OBSERVATION**. The sections read for this lesson
  do not specify MSPLIM's reset value; the lesson therefore does not present
  it as architectural. This repository never programs MSPLIM either —
  `__stack_limit` is only a linker symbol.

Rule: a value measured in an emulator is not labeled “architectural” until
confirmed in a primary source.

---

## References

- Armv8-M ARM (Arm DDI 0553A.g) — `RPLNS`, `RKSCH`, `RSQLX`, `RCMQP`,
  `RHWFV`, `RMDXK`, `RPLRT`, `RXPWM`
- Arm 107565 — vector table and initial MSP
- AAPCS32 — SP alignment at a public interface
- Repository: `startup_mps2_an505.c`, `system_init.c`, `mps2-an505-secure.ld`

---

[← Lesson 11](../README.md) · [Lesson 12 →](../../12-debug-dwt/)
