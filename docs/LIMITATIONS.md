**Language:** English | [Türkçe](LIMITATIONS.tr.md)

# Limitations - Simulator vs Real Hardware

QEMU is a **functional** emulator: it models instruction results, not hardware
timing. Experiment results must be interpreted within the following limits.

For measured values: `docs/TARGET_CAPABILITIES.md`

## 1. Timing is not real

QEMU runs instructions in translation blocks (TBs). There is no "cycle" concept
in the real sense.

Therefore:

- We **cannot** make claims such as "this function takes 120 cycles" in QEMU
- We compare `-O2` and `-O0` using **code size and generated assembly**, not wall time

## 2. The DWT cycle counter DOES NOT COUNT

Measured: even with `CYCCNTENA` enabled, `DWT->CYCCNT` remains zero.

```text
DWT CYCCNT PROFILING: NOT SUPPORTED ON CURRENT TARGET (QEMU mps2-an505)
```

Result: the classic "count cycles with DWT" profiling method **cannot be run** on
this target. Lesson 12 demonstrates this limitation and uses alternative
measurement methods (`docs/PERFORMANCE_MEASUREMENT.md`).

## 3. No cache

Cortex-M33 architecturally has no cache; this is not a QEMU omission.
Cache behavior (cleaning, invalidation, coherency) **cannot be observed** on this
target. Lesson 16 covers it theoretically; experiments require an M7/M55-class
target or real hardware.

## 4. Memory access errors can differ

On a real MCU, accessing an undefined address produces a bus error.
In QEMU, some addresses can be read/written silently. For example, `0x00000000`
is an actual RAM alias on this board; expecting "I wrote through a null pointer,
so I should immediately fault" is misleading here.

Therefore, in the fault lessons we generate errors by configuring the MPU and
performing a **defined violation**, rather than **inventing an address**.

## 5. The peripheral set is limited

The QEMU AN505 model has basic peripherals such as CMSDK UART, timers, and a
watchdog. It does not provide real-MCU ADC/SPI/I2C driver experience here.
This repository focuses on **core architecture**, not writing drivers.

## 6. Reset and power behavior

Brown-out, power-on reset delay, clock-tree configuration, PLL locking, and similar
topics are not modeled. This is why we do not need to configure clocks inside
`SystemInit()`; on real hardware this step is necessary.

## 7. Are these problems?

For the vast majority of the topics we want to learn, **no**:

| Topic | Can it be learned in QEMU? |
|---|---|
| Programmer model, registers | Yes |
| Stack, MSP/PSP, calling convention | Yes |
| Privilege, CONTROL, SVC | Yes |
| MPU, memory protection | Yes |
| Exceptions, faults, EXC_RETURN | Yes |
| Linker, ELF, startup | Yes |
| Context switching, PendSV | Yes |
| Basic TrustZone separation | Yes |
| **Cycle-accurate performance** | No |
| **Cache behavior** | No |
| **Real peripheral drivers** | No |
| **Electrical/analog behavior** | No |

What changes when you obtain a board: `docs/REAL_HARDWARE_MIGRATION.md`
