**Language:** English | [Türkçe](ARCHITECTURE.tr.md)

# Repository Architecture

Design rule: **every separation must have a real reason.**
This is an embedded learning repository, not an enterprise application.

## Layers

```text
lessons/  projects/  final-project/      <- ogrenme ve uygulama
        |
        v
common/        (console, tiny libc)      <- target'tan bagimsiz yardimcilar
        |
        v
platform/mps2-an505/                     <- board'a ozel: startup, UART, linker
        |                                   + mps2_an505.h (CMSIS device header)
        v
external/CMSIS/Core/Include/             <- resmi CMSIS-Core (BASELINE)
        |
        |   (yan yolda, yalnizca dersler icin)
        +-- core/include/core/cm33_teaching.h
        |
        v
cmake/  scripts/                         <- build ve calistirma altyapisi
```

## Why this separation?

**`core/` vs `platform/`**
`core/` contains everything belonging to the Armv8-M **architecture** (SCB, NVIC,
MPU, SAU, DWT). It does not change when moving to another Cortex-M33 board.
`platform/` belongs to the **board** (UART address, memory map, linker script).
Adding an M55 port only expands `platform/`.

**Separate `common/`**
Console and tiny libc are board-independent; `platform` provides the UART,
and `common` handles formatting.

**CMSIS vs teaching header**

The repository has two separate paths for accessing Cortex-M33 registers.
This is a deliberate distinction, not an inconsistency:

| | Baseline | Teaching |
|---|---|---|
| File | `external/CMSIS/Core/Include/core_cm33.h` (Arm, Apache-2.0) | `core/include/core/cm33_teaching.h` (handwritten, ~200 lines) |
| Entry point | `platform/mps2_an505.h` (CMSIS device header) | direct include |
| Users | startup, system init, drivers, `tools/`, projects, final project | only lesson firmware where the register itself is the subject |
| Purpose | Correctness, maintainability, real-world compatibility | Visibility |

**Why CMSIS as the baseline?**
CMSIS-Core is Arm's official Cortex-M abstraction layer. Every silicon vendor,
RTOS, and commercial SDK assumes it. Over time, a handwritten header breaks in
two ways: a register field is incorrect/missing, or it fails to keep up with
architectural updates. The infrastructure therefore uses official CMSIS.

**Why is there still a handwritten header?**
`core_cm33.h` alone is 3284 lines (measured). We use a smaller teaching header
to examine the CONTROL register's address and how `__get_CONTROL()` works.

`cm33_teaching.h` shows that the address of `SCB->CFSR` is `0xE000ED28` and how
`MRS`/`MSR` wrappers are written with two lines of inline assembly. Names match
CMSIS so the same calls can be used with either header.

**Practical rule**

```text
lessons/          -> register'in kendisi konuysa teaching header kullanilabilir
diger her sey     -> CMSIS (platform/mps2_an505.h)
```

One Lesson 12 assignment is to compile the same firmware first with the teaching
header, then with CMSIS, and compare the generated machine code. If there is no
difference, the abstraction is free — a concrete lesson in when abstraction is safe.

**CMSIS version and updates**
The version used and complete file list: [`external/README.md`](../external/README.md).
Files are kept unchanged (vendored) so builds do not depend on network access.

**Platform scope**
This repository currently assumes one security state (Secure) and one target:
QEMU `mps2-an505` / `cortex-m33`. Clock frequency, NVIC priority-bit count, and
reset VTOR value are TARGET-SPECIFIC contracts, NOT ARCHITECTURAL CONSTANTS;
their sources are recorded in `platform/mps2-an505/include/platform/board.h`
and `platform/mps2_an505.h`, with references to QEMU source.
Physical-board values have not been independently verified.

**Known limit: SAU and `-mcmse`**
CMSIS-Core exposes `SAU` definitions only when the compiler is invoked with
`-mcmse` (`__ARM_FEATURE_CMSE == 3`). This repository currently does not use
`-mcmse`, so `tools/capability-probe` reads SAU_TYPE from its fixed architectural
address and explicitly states this in the source. `-mcmse` will be enabled
when secure/non-secure separation is established in Lesson 16.

## Build flow

```text
CMakePresets.json
   |  (toolchainFile)
   v
cmake/arm-none-eabi.cmake        <- -mcpu/-mthumb/-mfloat-abi TEK yerde
   |
   v
CMakeLists.txt
   |
   +-- platform/mps2-an505/CMakeLists.txt   -> platform-mps2-an505 (INTERFACE)
   |
   +-- cmake/firmware.cmake                 -> add_firmware()
   |
   +-- lessons/*/CMakeLists.txt             -> her ders kendi firmware'ini tanimlar
```

**Why is the platform an INTERFACE library?**
Some lessons use different compiler flags (for example, Lesson 13 enables the FPU
with hard-float). Linking one precompiled static library with a different ABI
causes silent errors. With INTERFACE, each firmware compiles platform sources
**with its own flags**. The cost is recompiling a few files; the benefit is ABI safety.

**Why does `add_firmware()` exist?**
Every lesson wants the same 6 things: ELF, map, bin, disassembly, size, run/debug
targets. One function instead of copying this 16 times.

## Student code vs infrastructure code

A clear separation:

| Infrastructure (provided) | Written by the student |
|---|---|
| CMake, presets, toolchain file | `TODO(student)` blocks within lessons |
| QEMU/GDB scripts | Fault analyzer decoding logic |
| `platform/` startup (working version) | Startup from scratch in Lesson 11 |
| Console/UART driver | MPU configuration, SVC handler |
| `core/cm33_teaching.h` register definitions | Context-switch code, scheduler |

Startup under `platform/` **deliberately** works: firmware must run before the
learner reaches Lesson 11. In Lesson 11, the student writes startup from scratch
and compares the two.

## Directory responsibilities

| Directory | Contents |
|---|---|
| `core/` | Armv8-M register definitions and intrinsics |
| `platform/mps2-an505/` | Startup, linker script, UART, board addresses |
| `common/` | Console, minimal libc pieces |
| `lessons/NN-*/` | Lesson explanation (README) + working example |
| `projects/` | Larger work combining several lessons |
| `final-project/` | End-of-specialization integration project |
| `tools/` | Non-teaching helper firmware (capability probe) |
| `cmake/` | Toolchain file and build helpers |
| `scripts/` | Environment check, QEMU, GDB |
| `docs/` | Lasting reference documents |
| `tests/` | Smoke test |
| `external/` | Vendored third-party sources (CMSIS-Core) |

## Deliberate NON-GOALS

- No mandatory Docker
- No HAL/driver framework (this repository focuses on core architecture)
- No code generation
- No dependency manager (only external dependency: toolchain + QEMU)
- No gratuitous abstraction layers: every `platform_*` function has a real reason

Rationale: each extra layer moves the hardware we want to learn one step further away.
