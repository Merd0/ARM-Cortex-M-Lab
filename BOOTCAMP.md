**Language:** English | [Türkçe](BOOTCAMP.tr.md)

# ARM Cortex-M — 7 Day Practical Bootcamp

This file contains the daily lesson plan and study checklist.

**Labels:** `[QUICK]` 20–35 min · `[STANDARD]` 45–75 min · `[DEEP]` 90 min+
If time is short, work through `QUICK` and `STANDARD`; leave `DEEP` items
for the DEEP TRACK.

**Status:** `[ ]` not done · `[~]` in progress · `[x]` done

These boxes are the learner's personal study record; they are not checked
automatically when the repository is developed. Repository technical delivery:
16 lessons, five projects, and the final sensor node were implemented; local
three-preset/QEMU and critical GDB gates passed.
Lesson 13 was merged through PR #9 and Lessons 14–16 through PR #10 after
three-preset CI. Project/final PR #11 was also merged; three-preset CI PASS for
11fde900 main. Current publication and remote verification links are in
[Progress](docs/PROGRESS.md). Student study boxes are not filled automatically
on behalf of technical delivery. Current technical status: [Progress](docs/PROGRESS.md).

---

## Day 0 — Setup (once)

- [ ] `[QUICK]` Environment check — `./scripts/check-environment.sh` or `.\scripts\check-environment.ps1`
- [ ] `[QUICK]` First build — `cmake --preset debug && cmake --build --preset debug`
- [ ] `[QUICK]` First run — `./scripts/run-qemu.sh build/debug/tools/capability-probe/capability-probe.elf`
- [ ] `[QUICK]` First GDB connection — [`docs/DEBUGGING.md`](docs/DEBUGGING.md)
- [ ] `[QUICK]` Smoke test — `./tests/smoke-test.sh` (expected behavior of normal/broken/incomplete targets must be verified)
- [ ] `[QUICK]` Record your own tool versions — [`docs/TOOL_VERSIONS.md`](docs/TOOL_VERSIONS.md)

---

> **Start:** Follow the lessons in order, starting with `lessons/01`.
> `README.md` shows the same order.

## Day 1 — Cortex-M Fundamentals

- [ ] `[QUICK]` [01 — Cortex-M Family & Processor Selection](lessons/01-cortex-m-family/)
- [ ] `[STANDARD]` [02 — Programmer Model & Registers](lessons/02-programmer-model/)
- [ ] `[QUICK]` Complete the `decode_apsr_flags()` TODO in Lesson 02
- [ ] `[QUICK]` Lesson 02 "Break It" + "Debug It" — diagnose the HardFault yourself
- [ ] `[QUICK]` Fill in the "What I Learned" sections of Lessons 01 and 02

**End-of-day goal:** I can see R0–R12/SP/LR/PC/xPSR in GDB;
I can explain CONTROL's `nPRIV` and `SPSEL` fields; I can identify the source
of a HardFault from CFSR/HFSR.

---

## Day 2 — Functions, Assembly & Stack

- [ ] `[STANDARD]` [03 — C and Assembly](lessons/03-c-and-assembly/)
- [ ] `[STANDARD]` [04 — Stack, MSP & PSP](lessons/04-stack-msp-psp/)
- [ ] `[QUICK]` Write your own assembly function and call it from C
- [ ] `[QUICK]` Switch Thread Mode to PSP and verify it in GDB

**End-of-day goal:** I can follow a C function's assembly equivalent line by line;
I can see how the stack grows in memory; I can deliberately switch between MSP
and PSP.

---

## Day 3 — Privilege, SVC & Protection

- [ ] `[STANDARD]` [05 — Privilege & SVC](lessons/05-privilege-svc/)
- [ ] `[STANDARD]` [06 — MPU](lessons/06-mpu/)
- [ ] `[QUICK]` Enter unprivileged Thread Mode and test the boundary
- [ ] `[QUICK]` Call a privileged service through SVC
- [ ] `[STANDARD]` Configure an MPU-protected region and verify the violation with MemManage

**End-of-day goal:** I can establish the privileged/unprivileged distinction in
code; I can explain an MPU violation using fault status registers, rather than
merely saying "it crashed."

---

## Day 4 — Exceptions, Faults & Build Pipeline

- [ ] `[STANDARD]` [07 — Exceptions & NVIC](lessons/07-exceptions/)
- [ ] `[STANDARD]` [08 — Faults & Fault Analyzer](lessons/08-faults/)
- [ ] `[STANDARD]` [09 — Compiler, Optimization & AAPCS](lessons/09-compiler-aapcs/)
- [ ] `[STANDARD]` [10 — Linker, ELF & Memory Layout](lessons/10-linker-elf-memory/)
- [ ] `[DEEP]` Write the fault analyzer's CFSR decoding yourself

**End-of-day goal:** I can decode an exception stack frame in memory by hand;
I know what EXC_RETURN tells me; I can locate a global variable in the ELF using
the map file.

---

## Day 5 — Startup & Debugging

- [ ] `[STANDARD]` [11 — Startup: Reset to main](lessons/11-startup/)
- [ ] `[STANDARD]` [12 — Debugging & DWT Profiling](lessons/12-debug-dwt/)
- [ ] `[DEEP]` Write your own `Reset_Handler` from scratch
- [ ] `[QUICK]` Go from Reset to `main` using only `stepi`
- [ ] `[QUICK]` Verify for yourself that DWT does not count on this target

**End-of-day goal:** I can explain step by step what happens before `main` runs;
I can justify why a measurement is invalid.

---

## Day 6 — Advanced Execution

- [ ] `[DEEP]` [13 — Synchronization, FPU & DSP](lessons/13-sync-fpu-dsp/)
- [ ] `[STANDARD]` [14 — SysTick & PendSV](lessons/14-systick-pendsv/)
- [ ] `[QUICK]` Produce a race condition, then fix it with exclusive access
- [ ] `[QUICK]` Configure a 1 ms tick and verify that it works

**End-of-day goal:** I can both produce and fix a race condition;
I can explain when a memory barrier is necessary.

---

## Day 7 — Scheduler & Security

- [ ] `[DEEP]` [15 — Educational Mini Kernel](lessons/15-mini-kernel/)
- [ ] `[DEEP]` [16 — TrustZone & Advanced Features](lessons/16-trustzone-advanced/)
- [ ] `[DEEP]` Run a context switch between two tasks
- [ ] `[STANDARD]` Establish Secure/Non-secure separation using the SAU
- [ ] `[QUICK]` **Measure** target support for Cache / PACBTI / MVE and record the result
- [ ] `[STANDARD]` Write the final integration plan — [`final-project/`](final-project/)

**End-of-day goal:** Two tasks run with the context switch I wrote;
I can demonstrate experimentally what the TrustZone boundary prevents.

---

## DEEP TRACK — return to these later

These activities are not prerequisites for the FAST TRACK; they can be completed later:

- [ ] M55 (`mps3-an547`) platform port — measure MVE capability
- [ ] Arm FVP setup — for what cannot be observed in QEMU
- [ ] Extend the fault analyzer to a complete version (all CFSR bits)
- [ ] Task isolation with the MPU — on top of the mini kernel
- [ ] Cache behavior (when a suitable target is available)
- [ ] PACBTI demo (after target support is verified)
- [ ] Move to real hardware — [`docs/REAL_HARDWARE_MIGRATION.md`](docs/REAL_HARDWARE_MIGRATION.md)

---

## Daily routine

1. Open the day's lessons from this file
2. Read the lesson → build → run → observe in GDB
3. Run the **Experiment** and **Break It** experiments
4. Record your observations in **What I Learned**
5. Check the box and update [`docs/PROGRESS.md`](docs/PROGRESS.md) and the README progress

## When you get stuck

| Problem | See |
|---|---|
| Build does not work | [`docs/ENVIRONMENT.md`](docs/ENVIRONMENT.md) |
| I got a fault and do not understand it | [`docs/DEBUGGING.md`](docs/DEBUGGING.md) |
| Why does this feature not work? | [`docs/TARGET_CAPABILITIES.md`](docs/TARGET_CAPABILITIES.md) |
| Where do the addresses come from? | [`docs/MEMORY_MAP.md`](docs/MEMORY_MAP.md) |
| I do not understand a term | [`docs/GLOSSARY.md`](docs/GLOSSARY.md) |
