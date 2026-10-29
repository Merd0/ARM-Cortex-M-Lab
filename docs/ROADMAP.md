**Language:** English | [Türkçe](ROADMAP.tr.md)

# Roadmap

## Current status — 2026-10-27

16 lessons, five projects, and the final system are on main. PRs #9–11 were merged;
three-preset CI PASS for 11fde900. The [internal engineering audit](ENGINEERING_AUDIT.md)
records fixes and release preparation. The audit publication/CI/merge record is in
[PR #12](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/12). No new tag/Release was created.

## Historical completion order — 2026-10-23

Starting point: `main` = `266642efb55e881a275a1034a01923ecbd17a3fa`.
Lessons 01–11 and earlier audit fixes are preserved. No new curriculum or platform
is being designed; the existing scope below will be completed.

1. **Lesson 12 — Debugging & DWT:** completed on main through PR #8;
   post-merge three-preset CI PASS for `01bc73c51046d09169f68c8cae009df4deb8e433`
   (2026-10-24). No cycle results were fabricated.
2. **Lesson 13 — Sync/FPU/DSP:** nine firmware targets on this branch; controlled
   lost-update/exclusive retry, PRIMASK exercise, NOCP, eager/lazy frames, DSP,
   and positive/negative softfp/hard ABI link checks implemented. Local three-preset
   QEMU/GDB and PR #9 three-preset CI PASS; PR #9 merged.
3. **Lesson 14 — SysTick/PendSV:** implemented; Debug build and four QEMU scenarios
   PASS (including expected starter/broken outcomes). Local commit.
4. **Lesson 15 — Mini Kernel:** PSP/MSP, synthetic initial frame, assembly
   R4–R11/EXC_RETURN preservation, stack bounds, scheduler regressions.
   Implemented; Debug and four QEMU scenarios PASS. Local commit.
5. **Lesson 16 — TrustZone:** actual Secure/Non-secure images on AN505,
   SAU/NSC/gateway and unauthorized-access experiments; advanced features will
   not be considered working without capability evidence. Two images, NSC scalar
   service, AUVIOL, and INVEP implemented; Debug/four QEMU pairs PASS. Local commit.
6. **Integration projects:** From Reset to Main (10–11), Fault Analyzer (07–08),
   Protected Application (04–08), Performance Profiler (09/12), Educational
   Mini Kernel (14–15). Each is a separate acceptance gate.
7. **Final sensor node:** integration of the five projects and TrustZone boundary;
   synthetic sensor, data processing, controlled services. Security claims are
   reported with negative tests, real-time claims with target/model limitations.

User decision on 2026-10-24: 14 → 15 → 16 proceeded on the same local development
branch, each with a separate commit and basic build/QEMU/critical-correctness gate.
Then the **combined local quality gate** completed: detailed GDB, three-preset
comparison, and broad smoke/build probes/negative tests PASS. PendSV stack bounds,
context integrity, and cross-security exception/NSC boundaries were reviewed.
This is not an independent third-party audit or board verification.

Remote push/PR/merge requires separate user approval; no merge before CI passes.
Main is protected. The five projects and final integration also passed local gates.
Checkpoint tags and Release are decided separately.

Preserved acceptance criteria from earlier audits: fresh PSP frame/stack guard,
EXC_RETURN and fault-frame rejection, M33 HFSR/BFSR rules, full answer equality,
AAPCS/SP recovery, effective -O2, libgcc order, FLASH LMA bounds, reserved vectors
8/9/10/13. Existing tests will not be reduced.

## Phase 0 - Bootstrap (COMPLETE)

- Repository skeleton, CMake + Ninja + Arm GNU Toolchain build system
- MPS2-AN505 (Cortex-M33) platform port: startup, linker script, UART
- Working firmware in QEMU + GDB connection verified
- Target capability probe and measured capability matrix
- Lesson 02 (Programmer Model) ready as the full-quality reference lesson

## Phase 1 - FAST TRACK (~1 week, main learning path)

Day-by-day plan: [`BOOTCAMP.md`](../BOOTCAMP.md)

| Day | Topic | Lessons |
|---|---|---|
| 1 | Fundamentals and programmer model | 01, 02 |
| 2 | Assembly, function calls, stack | 03, 04 |
| 3 | Privilege, SVC, MPU | 05, 06 |
| 4 | Exceptions, faults, build pipeline | 07, 08, 09, 10 |
| 5 | Startup and debugging | 11, 12 |
| 6 | Synchronization, FPU/DSP, SysTick | 13, 14 |
| 7 | Context switching, mini kernel, TrustZone | 15, 16 |

At the end of the FAST TRACK: a working mini kernel, a usable fault analyzer,
and your own observations on every topic.

## Phase 2 - DEEP TRACK (optional, later)

Each lesson includes its own DEEP TRACK experiments. Additionally:

- **M55 platform port** - a second platform for `mps3-an547`; actually measure
  MVE/Helium capability (currently `OLCULMEDI`)
- **Arm FVP integration** - for behavior not observable in QEMU
- **Expand the fault analyzer to a full version** - decode all CFSR bits
- **MPU task isolation** - on top of the mini kernel
- **PACBTI and cache** - after target support is verified

## Phase 3 - After the specialization

A separate phase, independent of this repository:

```text
FreeRTOS or Zephyr  ->  micro-ROS  ->  ROS 2
```

The mini kernel is educational, not a production RTOS. A real project uses a
mature RTOS. At that point, this repository's knowledge answers "what happens
underneath the RTOS?"

## Real hardware

When a board is available: `docs/REAL_HARDWARE_MIGRATION.md`

## Local completion and publication order — 2026-10-24

Five projects and the final sensor pipeline were implemented. Final local
three-preset gates are in [Progress](PROGRESS.md): clean build, smoke/probes,
34 new runtime scenarios, critical GDB observations PASS. Student TODOs remained
separate in the lessons.

The user approved sequential publication. No new curriculum is added:
1. PR #9 merged after three-preset CI; commit history preserved.
2. Lessons 14–16 merged through
   [PR #10](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/10) after three-preset CI.
3. Project/final integration merged through PR #11; main `11fde900`,
   [post-merge three-preset CI PASS](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40105930630).

Current remote results are on [GitHub Actions](https://github.com/Merd0/ARM-Cortex-M-Lab/actions).
Local review and CI are not presented as an independent third-party audit.
Tag/Release will be decided separately.
