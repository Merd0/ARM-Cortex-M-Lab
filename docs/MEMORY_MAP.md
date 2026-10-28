**Language:** English | [Türkçe](MEMORY_MAP.tr.md)

# Memory Map - MPS2-AN505 (Cortex-M33)

The addresses here were verified against two sources:

1. QEMU's own model: `qemu-system-arm -M mps2-an505 -monitor stdio` -> `info mtree -f`
2. Arm Application Note AN505 (DAI0505) and the Arm IoT Kit memory map

## Secure / Non-secure aliases

In Armv8-M, one memory region can appear at two different addresses.
In the IoT Kit, the distinction is **bit 28**:

```text
0x0_______   bit28 = 0   ->  Non-secure alias
0x1_______   bit28 = 1   ->  Secure alias     (same physical memory)
```

Firmware **boots in Secure state**, so this repository uses Secure aliases
(`0x10000000`, `0x30000000`, `0x50200000`).

## Regions actually present in QEMU

| Address (NS) | Address (S) | Size | What |
|---|---|---|---|
| `0x00000000` | `0x10000000` | 4 MB | ZBT SSRAM1 - code region |
| `0x20000000` | `0x30000000` | 32 KB | IoT Kit internal SRAM |
| `0x28000000` | `0x38000000` | 2 MB | ZBT SSRAM2 |
| `0x28200000` | `0x38200000` | 2 MB | ZBT SSRAM3 |
| `0x40200000` | `0x50200000` | 4 KB | CMSDK APB UART0 |
| `0x40080000` | `0x50080000` | 4 KB | IoT Kit Security Controller |
| - | `0xE0000000` | 1 MB | Private Peripheral Bus (SCS, NVIC, SCB, MPU, SAU, DWT) |

## Linker layout in this repository

`platform/mps2-an505/linker/mps2-an505-secure.ld`:

```text
FLASH : ORIGIN = 0x10000000, LENGTH = 512K
RAM   : ORIGIN = 0x30000000, LENGTH = 32K
```

Why 512 KB of FLASH? Physically there are 4 MB. We deliberately constrain it to
behave like a real MCU and make "region overflow" errors meaningful.

## Layout diagram

```text
0x10000000  +----------------------------+  <-- FLASH ORIGIN
            |  .vectors                  |      [0] = initial MSP
            |    [0] __stack_top         |      [1] = Reset_Handler
            |    [1] Reset_Handler       |      Secure VTOR points here at reset
            |    ...                     |
            +----------------------------+
            |  .text     (code)           |
            +----------------------------+
            |  .rodata   (constants)      |
            +----------------------------+
            |  FLASH copy of .data   |  <-- __etext (LMA)
            +----------------------------+
0x10080000  |  (unused)           |      512 KB boundary
            +----------------------------+


0x30000000  +----------------------------+  <-- RAM ORIGIN
            |  .data  (copied to RAM) |  <-- __data_start .. __data_end
            +----------------------------+
            |  .bss   (zeroed)       |  <-- __bss_start .. __bss_end
            +----------------------------+
            |  .heap  (currently 0 bytes)     |
            +----------------------------+
            |                            |
            |     FREE SPACE               |
            |                            |
            +----------------------------+  <-- __stack_limit
            |  STACK                     |
            |     |                      |      full descending:
            |     v  grows downward         |      grows as SP decreases
0x30008000  +----------------------------+  <-- __stack_top = initial MSP
```

## How do I verify it?

```bash
# Linker'in gercekte nereye koydugunu gor
arm-none-eabi-nm -n build/debug/lessons/02-programmer-model/02-programmer-model.elf | head -20

# Section'lar ve adresleri
arm-none-eabi-objdump -h build/debug/lessons/02-programmer-model/02-programmer-model.elf

# Map file icinde sembol ara
grep -n "__stack_top\|__data_start\|__bss_start" \
    build/debug/lessons/02-programmer-model/02-programmer-model.map
```

In GDB:

```text
p/x &__stack_top
p/x &__bss_start
x/4wx 0x10000000        # first 4 entries of the vector table
```

## Important detail: where does the initial MSP come from?

At Cortex-M33 reset:

1. The Secure VTOR reset value is `0x10000000` (set this way by QEMU's IoT Kit model)
2. Hardware reads the word at `[0x10000000]` -> **MSP**
3. It reads the word at `[0x10000004]` -> **PC** (Reset_Handler)

Measured (`arm-none-eabi-objdump -s -j .vectors`):

```text
Contents of section .vectors:
 10000000 00800030 21020010 1d020010 1d020010  ...0!...........
```

Little-endian decoding:

```text
vector[0] = 0x30008000   -> initial MSP  (= __stack_top)
vector[1] = 0x10000221   -> Reset_Handler
```

`0x10000221` -> lowest bit 1: **Thumb state indicator**.
The actual function address is `0x10000220`. If this bit were 0, the processor
would immediately generate UsageFault (INVSTATE) at reset.

Verification through symbols:

```text
10000220 T Reset_Handler
30007000 A __stack_limit
30008000 R __stack_top
```

---

## Scope notes (added after repair)

**The Secure VTOR at reset is not an architectural constant.** On this target its
value is `0x10000000`, from the line `mmc->init_svtor = 0x10000000;` in QEMU v8.2.2
`hw/arm/mps2-tz.c`. Another Cortex-M33 device can differ. Startup therefore
explicitly writes VTOR in all cases rather than relying on its reset value.

**Space reserved by the linker for the stack is not runtime protection.**
`__stack_size = 4K` and its `ASSERT` check only the **link-time** layout:
does the stack region overlap `.bss`/`.heap`? Nothing here prevents the stack
crossing this boundary at runtime. Runtime protection comes from separate mechanisms:

| Mechanism | What it does | Lesson |
|---|---|---|
| `MSPLIM` / `PSPLIM` | UsageFault if a stack-pointer update crosses the limit | 04 |
| MPU | Region-based access control (general memory) | 06 |

**External interrupts (external IRQs) must currently remain DISABLED.**
Device-specific IRQ entries have not yet been defined in the vector table
(see `IRQn_Type` in `platform/mps2_an505.h`). If a peripheral interrupt is enabled,
the processor branches to an undefined vector entry. IRQ entries will be added
in Lesson 07 after verification against the AN505 application note.

## Lesson 16: two separate image layouts

The 32 KiB Secure RAM layout above is the assumption of the shared single-image
platform. [Lesson 16](../lessons/16-trustzone-advanced/) uses its own linker scripts
to split physical SRAM into Secure/NS halves of 16 KiB each; NS code is at
0x00080000 and NSC veneers at 0x10070000. The MPC block size is read at runtime.
Earlier lessons' shared linker/startup remains unchanged; non-overlapping
placement of the two images in the same physical memory and attribution are
established separately.
