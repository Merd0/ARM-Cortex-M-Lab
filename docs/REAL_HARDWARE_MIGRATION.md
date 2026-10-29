**Language:** English | [Türkçe](REAL_HARDWARE_MIGRATION.tr.md)

# Real Hardware Migration

What changes when you obtain a Cortex-M33 board (for example, an MCU board
supporting Armv8-M/TrustZone)?

## What stays the same (most of the learning in this repository)

- Programmer model, registers, xPSR
- MSP/PSP, Thread/Handler Mode, CONTROL, privilege
- Exception model, EXC_RETURN, NVIC, fault status registers
- MPU programming concepts
- AAPCS, assembly, linker/ELF concepts
- Context-switching concepts (SysTick + PendSV)
- GDB commands and diagnostic flow

These belong to the **architecture**, not the board.

## What changes

### 1. Linker script and memory map

| Now (QEMU AN505) | Real MCU |
|---|---|
| FLASH `0x10000000` (SRAM, writable) | Real flash, usually `0x00000000`/`0x08000000` |
| RAM `0x30000000`, 32 KB | Device-specific address and size |
| No flash programming | A flash-programming step is required |

File to change: `platform/<board>/linker/*.ld`

### 2. Clock setup

In QEMU, `SystemInit()` is almost empty. A real MCU requires PLL configuration,
flash wait states, and voltage scaling.
Wrong wait states = faults you cannot explain.

### 3. Console

Now: CMSDK APB UART, `0x50200000`.
Real hardware: device-specific UART + pin (GPIO alternate function) configuration.
File to change: `platform/<board>/src/uart_*.c`

### 4. Loading and debug connection

| Now | Real hardware |
|---|---|
| `qemu-system-arm -kernel fw.elf` | `openocd` / `pyocd` / vendor tool + SWD probe |
| `-gdb tcp::3333` | OpenOCD/pyOCD GDB server (usually port 3333) |

GDB commands work unchanged - only the connected server changes.
This is why `scripts/gdb/cortex-m.gdb` is portable.

### 5. Adding a new platform

```text
platform/
    mps2-an505/        <- existing
    <new-board>/       <- new
        include/platform/board.h
        src/startup_<board>.c
        src/uart_<board>.c
        linker/<board>.ld
        CMakeLists.txt
```

`core/`, `common/`, and `lessons/` do not change. This is precisely why the
architectural layer is separated from the board (`docs/ARCHITECTURE.md`).

## New possibilities

| Topic | Now | On real hardware |
|---|---|---|
| **DWT cycle counter** | Does not count | Works - real profiling is possible |
| **Timing measurement** | Meaningless | Real; can be verified with an oscilloscope/GPIO |
| **Cache** | Absent in M33 | Observable on M7/M55-class devices |
| **Power consumption** | None | Effects of WFI/sleep modes can be measured |
| **Real peripherals** | Very limited | ADC/SPI/I2C/DMA |
| **Electrical problems** | None | Noise, brown-out, signal integrity |

## Migration checklist

- [ ] Create a new `platform/<board>/` directory
- [ ] Write the linker script for the device's flash/RAM addresses
- [ ] Add clock and flash wait-state setup to startup
- [ ] Write the UART driver and pin configuration
- [ ] Add an OpenOCD/pyOCD config file
- [ ] Add a flash-loading script under `scripts/`
- [ ] Run `capability-probe` and add a **measured** column to `TARGET_CAPABILITIES.md`
- [ ] Verify that DWT actually counts and update the profiling lessons
