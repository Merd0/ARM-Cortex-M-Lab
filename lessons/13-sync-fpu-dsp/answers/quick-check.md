**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 13 — Quick Check Answers

## 1. Why does incrementing a volatile counter lose an update?

Load and store are separate instructions. The Handler changes 0→1 between them;
the 1 produced from the 0 previously read by Thread overwrites the handler's result.
Volatile does not make these operations atomic. In the experiment, DSB/ISB do not
prevent the lost update either.

## 2. Why does STREX fail, and what does retry change?

The exception entry/exit process clears the local exclusive monitor. The first
STREX returns 1 and does not write. Retry reads the new value again with LDREX;
a successful STREX preserves both increments. Retrying only the store is insufficient.
If the lab's bounded attempt count is exhausted, the function returns failure;
there is no general progress or WCET guarantee. DMA/global-monitor behavior is a
separate system concern; this test does not verify it.

## 3. Why is the previous PRIMASK restored?

If the caller was already masked, unconditional enable opens the outer critical
section. The driver's second call catches this. PRIMASK does not mask NMI or
HardFault. A memory barrier does not provide atomicity or mutual exclusion: DMB
addresses memory ordering, DSB completion, and ISB the use of the current execution
context by subsequent instructions. Failure to observe reordering after removing
a barrier in this model does not prove the barrier is architecturally unnecessary.

## 4. Does enabling the FPU alone produce an extended frame?

No. In the basic example, CPACR is enabled, but no FP instruction precedes SVC:
FPCA=0, EXC_RETURN=0xFFFFFFF9, 32 bytes. In the Thread that uses FP, measured values
were FPCA=1, EXC_RETURN=0xFFFFFFE9, 104 bytes. These sizes apply to this experiment:
Secure→Secure, MSP, TS=0, 8-byte-aligned entry, and no padding.

## 5. Why is allocated lazy-frame space not yet FP data?

With LSPACT=1, hardware has allocated space but may not yet have written the
registers. In the experiment, the raw S0 slot remains 0xA5A5A5A5. The Handler's
first FP instruction triggers the save; LSPACT becomes 0, and the slot shows the
old Thread S0=0x3FC00000. The Handler's live S0=8.0 differs from the 1.5 on the stack;
1.5 is restored on exception return. The reserved word is not valid FP data.

## 6. NOCP or FORCED HardFault?

UsageFault is enabled in this experiment. At the first unauthorized FP instruction,
CFSR=0x00080000 (UFSR.NOCP), HFSR=0, IPSR=6. PC points to that instruction.
Automatically calling it HardFault/FORCED is wrong; escalation depends on
enable/priority conditions.

## 7. Are soft, softfp, and hard the same thing?

Soft uses software helpers for C FP operations; softfp permits hardware FP and
preserves the base AAPCS calling convention. Hard passes FP parameters/results
using the VFP convention. CPACR permission is a separate runtime condition.
FPSCR control bits are also part of the calling contract: in the lab, an environment
support function selects FZ=1; the arithmetic worker does not change it, and
stacked/call-return FPSCR values are compared. This lab explicitly selects the two
conventions in its assembly adapter; it does not make arbitrary mixed objects safe.
Link probes reject incompatibility in both directions. S16–S31 are callee-saved
under normal AAPCS; this assembly uses only S0–S2. A TS=0 hardware frame does not
mean that all FP registers have been saved for the scheduler. Context switching
in Lesson 15 is a separate acceptance gate.

## 8. Why is DSP not proof of speed?

SMLAD sums the products of two signed 16-bit lanes; QADD can saturate overflow and
set the sticky Q bit. The plain C reference does not produce signed overflow in
these four bounded examples. Both paths produce -36000; QADD is tested in both
directions. Disassembly shows that the instructions were emitted. No cycle
measurement was made in QEMU; no conclusion of greater speed or real-time
performance follows.

[← Lesson](../README.md)
