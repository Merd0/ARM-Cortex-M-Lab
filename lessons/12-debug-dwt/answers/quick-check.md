**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 12 — Quick Check Answers

## 1. Why is a watchpoint different from a source breakpoint?

A breakpoint stops at a particular execution address. `watch -l g_l12_watch`
stops when the value in the selected memory changes; you do not need to know
the writing function in advance. `rwatch` watches read accesses. GDB's "hardware"
label describes QEMU TCG gdbstub support; it does not establish the number of
physical DWT comparators.

## 2. What do CTRL.NOCYCCNT=0 and CYCCNT=0 prove?

They do not by themselves prove a working cycle counter. In the QEMU 11.1.0 test,
the enable bits written to CTRL and DEMCR did not read back either; CYCCNT stayed
zero before/after the workload. The register snapshot and observed behavior are
recorded separately. This does not mean "the work took zero cycles."

## 3. What is the difference from `0xFFFFFFF0` → `0x10`?

32 with `uint32_t` modular subtraction. For this to be the actual elapsed count,
the interval must be shorter than `2^32` increments. Equal endpoints cannot
distinguish no progress from a full wrap. This experiment does not accept equal
endpoints as a usable measurement.

## 4. If the O2 workload is smaller, is it faster on hardware?

This experiment does not prove that. 70/32 bytes and 35/13 static disassembly
instructions describe this GCC build's code; dynamic loop execution counts and
hardware instruction latency are separate questions. Converting the QEMU SysTick
iteration ratio into a physical Cortex-M33 speed ratio is also invalid.

## 5. Why is console output inside the measurement interval a problem?

The UART driver and output waits are added to the workload. `12-profile-io` does
not corrupt the result checksum, but `CHECK_CLEAN_WINDOW` FAILs. Do not write a
test that expects fewer iterations under every condition: host load and emulator
timing affect results. The change in measurement scope is itself the diagnosis.

## 6. Why are `bt` and a stack dump not the same thing?

`bt` reconstructs frames using debug/unwind information and prologue analysis.
Not every Flash-like word on the stack is a return address; it may be data.
A leaf function may never write LR to the stack. In this lesson, the prologues/
epilogues of `l12_middle` and `l12_outer` are read through disassembly, and saved
LRs are compared with call sites. Offsets from one build do not carry over to another.

## 7. Is the teaching header a complete equivalent of CMSIS?

No. Identical machine code from two selected read functions establishes only
those functions, these flags, and this toolchain. Differences such as SAU guards,
missing registers, and macros remain.

[← Lesson](../README.md)
