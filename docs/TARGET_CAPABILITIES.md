**Language:** English | [Türkçe](TARGET_CAPABILITIES.tr.md)

# Target Capabilities

The table contains information measured with `tools/capability-probe` firmware
or verified from tool output (`qemu-system-arm -machine help`).
Features not yet checked are marked `OLCULMEDI`.

## Measurement method

```bash
cmake --preset debug
cmake --build --preset debug --target capability-probe
./scripts/run-qemu.sh build/debug/tools/capability-probe/capability-probe.elf
```

The probe reads ID registers (CPUID, MPU_TYPE, SAU_TYPE, MVFR0-2, ID_ISAR,
DWT_CTRL) and tests whether the DWT cycle counter counts.

## Capability matrix

| Feature | QEMU M33 (mps2-an505) | QEMU M55 (mps3-an547) | QEMU M85 | Arm FVP | Real hardware |
|---|---|---|---|---|---|
| Machine available? | YES (measured) | YES (`-machine help`) | **NO** (absent in this QEMU) | OLCULMEDI | - |
| Programmer model / registers | YES (measured) | OLCULMEDI | - | OLCULMEDI | YES |
| Thread / Handler Mode | YES (measured) | OLCULMEDI | - | OLCULMEDI | YES |
| MSP / PSP | YES (measured) | OLCULMEDI | - | OLCULMEDI | YES |
| Privilege (CONTROL.nPRIV) | YES (measured) | OLCULMEDI | - | OLCULMEDI | YES |
| MPU | YES - 8 regions (measured) | OLCULMEDI | - | OLCULMEDI | YES |
| TrustZone / SAU | YES - 8 regions (measured) | OLCULMEDI | - | OLCULMEDI | Device-dependent |
| Exceptions / NVIC / faults | YES (measured, see below) | OLCULMEDI | - | OLCULMEDI | YES |
| FPU (single precision) | PRESENT but CPACR disabled (measured) | OLCULMEDI | - | OLCULMEDI | YES |
| DSP extension | PRESENT (ID_ISAR, measured) | OLCULMEDI | - | OLCULMEDI | YES |
| MVE / Helium | ABSENT (architecturally absent on M33) | Present in architecture, OLCULMEDI | - | OLCULMEDI | Device-dependent |
| **DWT cycle counter (CYCCNT)** | **DOES NOT COUNT** (measured) | OLCULMEDI | - | OLCULMEDI | YES |
| Cache | Absent on M33 (architecture) | OLCULMEDI | - | OLCULMEDI | Device-dependent |
| PACBTI | OLCULMEDI | OLCULMEDI | - | OLCULMEDI | Device-dependent |
| Realistic timing | **NO** | NO | - | Better | YES |
| Trace (ITM/ETM) | OLCULMEDI | OLCULMEDI | - | OLCULMEDI | Device-dependent |

`OLCULMEDI` = not yet tested. This table is updated as testing proceeds.
`-` = not applicable because the machine is absent in this QEMU version.

## Measured raw values (QEMU 8.2.2, mps2-an505, cortex-m33)

`capability-probe` output:

```text
===== TARGET CAPABILITY PROBE =====
[CPUID] 0x410FD213
[CPUID.PARTNO] 0x00000D21
[CPUID.VARIANT] 0x00000000
[CPUID.REVISION] 0x00000003
[MPU_TYPE] 0x00000800
[MPU_REGIONS] 8
[SAU_TYPE] 0x00000008
[SAU_REGIONS] 8
[MVFR0] 0x10110021
[MVFR1] 0x11000011
[MVFR2] 0x00000040
[MVFR1.MVE] 0x00000000
[CPACR] 0x00000000
[ID_ISAR0] 0x01101110
[ID_ISAR1] 0x02212000
[ID_ISAR2] 0x20232232
[ID_ISAR3] 0x01111131
[ID_ISAR4] 0x01310132
[ID_ISAR5] 0x00000000
[DWT_CTRL] 0x00000000
[DWT_CYCCNT_IMPLEMENTED] YES
[DWT_CYCCNT_T0] 0
[DWT_CYCCNT_T1] 0
[DWT_CYCCNT_COUNTING] NO
[SYSTICK_CALIB] 0x80000000
===== PROBE COMPLETE =====
```

Interpretation:

- `CPUID.PARTNO = 0xD21` -> Cortex-M33, revision r0p3.
- `MPU_TYPE = 0x800` -> DREGION field (bits 15:8) = 8 -> **8 MPU regions**.
- `SAU_TYPE = 8` -> 8 SAU regions -> TrustZone present.
- `MVFR1.MVE = 0` -> no MVE/Helium. This is not a QEMU omission: **the Cortex-M33 architecture does not include MVE.**
- `CPACR = 0` -> FPU disabled at reset. CP10/CP11 must be enabled to use it (Lesson 13).

## DWT: the most important warning

Reading `DWT_CTRL` shows `NOCYCCNT` clear: the register says "cycle counter present."
However, **CYCCNT stays at 0** even after enabling `CYCCNTENA`.

```text
[DWT_CYCCNT_T0] 0
[DWT_CYCCNT_T1] 0
[DWT_CYCCNT_COUNTING] NO
```

Profiling with the DWT cycle counter is therefore **not possible** on this target.

### This is an observation, not an absolute verdict

Every line above is **behavior measured on this machine**: QEMU 8.2.2,
`-M mps2-an505 -cpu cortex-m33`. Results apply to this configuration.

Limits on interpretation:

- Other QEMU versions and machine/cpu combinations can exhibit different
  cycle-counter behavior.
- This observation alone does not establish a QEMU bug. QEMU is a **functional**
  emulator: it executes instructions, but does not aim to model pipelines and
  clock cycles. The cycle counter falls outside that modeling scope.

Independent support: the RTIC (cortex-m-rtfm) project's documentation states
that timing examples will not work correctly in QEMU because the Cortex-M cycle
counter functionality is not implemented in `qemu-system-arm`
(<https://docs.rs/crate/cortex-m-rtfm/latest/source/book/en/src/by-example/timer-queue.md>).
Other QEMU-based projects record the same complaint. This **supports** our
measurement — but our own measurement remains the primary source.

No explicit statement about DWT cycle-counter support for mps2 boards was found
in official QEMU documentation. Since "undocumented" and "unsupported" are not
the same, this is recorded as a search result rather than a claim.

### Result

```text
DWT CYCCNT PROFILING: NOT SUPPORTED ON CURRENT TARGET (QEMU mps2-an505)
```

The profiling lesson and `projects/performance-profiler/` were built **around**
this constraint: static code size, static instruction count, and SysTick-based
**target-local** interval/counting experiments. These measurements do not provide
cycle counts and **do not establish hardware performance rankings** - functional-
emulator time is not hardware cycle time.

Future paths (both documented, neither tried):

| Path | What it provides | Status |
|---|---|---|
| Arm FVP (Fixed Virtual Platform) | Functional modeling; timing information **only** in models explicitly documented to provide it | Planned — `docs/REAL_HARDWARE_MIGRATION.md` |
| Real hardware (e.g. Nucleo, MPS2+) | Actual CYCCNT + SWO/ETM trace (varies BY BOARD) | Planned — same file |

A warning about the last row: **there is no rule that the DWT cycle counter works
on every board.** DWT itself is optional; the cycle counter is separately optional
inside DWT (`DWT_CTRL.NOCYCCNT`), and a manufacturer may restrict debug
infrastructure on some devices. This repository's probe already asks the right
question: "does the register exist?" and "does it count?" are different.
Run the same probe when moving to real hardware.

These simulator limitations must be considered when planning experiments.

## Note: how SAU was read

`tools/capability-probe` reads SAU_TYPE from its fixed architectural address
(`SCS_BASE + 0x0DD4`), not through CMSIS. CMSIS-Core exposes SAU definitions only
when the compiler is invoked with `-mcmse`, and this repository currently does
not use `-mcmse`. This will change when secure/non-secure separation is established
in Lesson 16. Details: [`docs/ARCHITECTURE.md`](ARCHITECTURE.md).

## Exception/fault behavior (measured)

The deliberately broken Lesson 02 firmware (`02-programmer-model-broken`)
produced these values in QEMU + GDB:

```text
IPSR  = 3            -> HardFault
CFSR  = 0x00009200   -> BFSR = 0x92 (BFARVALID + STKERR + PRECISERR)
HFSR  = 0x40000000   -> FORCED (escalation)
BFAR  = 0xFFFFFFF8
LR    = 0xFFFFFFFD   -> EXC_RETURN
```

Fault escalation, fault status registers, and EXC_RETURN therefore work on this
target. Lesson 07/08 experiments use these behaviors.

## Why are M55 / M85 / FVP empty?

- **M55 (mps3-an547):** The machine exists in this QEMU. However, it uses a different
  memory map (ITCM/DTCM, different UART address), requiring a separate platform
  port. Until the port is written, the table remains `OLCULMEDI`. A DEEP TRACK task.
- **M85:** This QEMU version (8.2.2) has **no** cortex-m85 in `-cpu help` output,
  and no corresponding MPS3 machine is listed.
- **Arm FVP:** Requires separate installation/licensing and was not installed
  during bootstrap. Also note: "FVP" does not automatically mean a timing-accurate
  or cycle-approximate model. An ordinary FVP is functional. A model has timing
  capability only if its own documentation explicitly says so; no FVP has been
  verified in this repository.

Check commands:

```bash
qemu-system-arm -machine help | grep mps
qemu-system-arm -cpu help | grep cortex-m
```

## Lesson 12 remeasurement — 2026-10-23

Windows / QEMU `11.1.0 (v11.1.0-12130-ge470268ff4)`, Cortex-M33 AN505:
after attempts to write DEMCR.TRCENA and DWT.CTRL.CYCCNTENA, both register
readbacks were `0x00000000`; CYCCNT start/end `0/0`.
GDB write/read watchpoints worked through the gdbstub. This is not proof of
physical DWT comparator support. Details and reproduction commands:
[Lesson 12](../lessons/12-debug-dwt/).


## Lesson 13 functional recheck — 2026-10-24

Local Windows / GCC 15.2.1 / QEMU 11.1.0, `mps2-an505 / cortex-m33`:

- With CPACR=0, the first FP instruction caused NOCP UsageFault (CFSR=0x00080000,
  HFSR=0, IPSR=6). After CP10/CP11 were enabled, FP addition produced 3.5.
- A basic 32-byte frame without active FP context; with active context and TS=0,
  a 104-byte extended frame was measured in the Secure→Secure/MSP experiment.
- Lazy LSPACT 1→0, raw-stack-slot filling at the first handler FP instruction,
  and subsequent S0–S2 restoration were verified in GDB.
- The caller's FZ=1 FPSCR control state was preserved across worker/exception return.
  Denormal arithmetic behavior was not measured in this experiment.
- SMLAD matched the bounded C reference; QADD produced saturation and the Q flag
  in both directions. This is functional verification, not cycle/speed measurement.
- Exception-disrupted exclusive reservation and retry, and PRIMASK restoration
  to its previous value, were verified across three presets. DMA/global-monitor
  or multicore atomicity was not verified.

Full commands, scope, and records: [Lesson 13](../lessons/13-sync-fpu-dsp/).
These results do not change the earlier ID snapshot; measurements must be repeated
on another QEMU version or physical target. Lesson 13's remote CI result is tracked separately.
