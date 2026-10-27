**Language:** English | [Türkçe](README.tr.md)

# Project — Fault Analyzer

**STATUS: on main; Debug/Release/MinSize CI PASS (11fde900)**

[diagnostic.c](diagnostic.c) reuses Lesson 08's reference decoder.
It adds a conservative frame-capture boundary in front of it: trusted stack bounds,
supported EXC_RETURN, minimum length, alignment, xPSR.T/exception mode, and padding.
On MSTKERR, STKERR, STKOF, unstack/lazy-save errors, **the pointer is not dereferenced**.
Basic same-state Secure and Non-secure frames are supported. FP and cross-security
frames are rejected instead of silently decoded incorrectly.

## Critical correction

In this basic Secure context, `0xFFFFFFFD` is a **Thread Mode / PSP / basic frame**
return. It is not a Handler/MSP return. Its Non-secure same-state counterpart is
`0xFFFFFFBC`; the same constant cannot be used while ignoring the security bits.

For example, CFSR=0x9200 contains PRECISERR, STKERR, and BFARVALID.
HFSR.FORCED=1 indicates an escalated fault. BFAR is address evidence only with its
validity bit; since stacking failed, the stacked PC is **not trustworthy**.
The coexistence of these bits alone does not prove the causal order of events.

## Running

```bash
cmake --preset debug
cmake --build --preset debug
./scripts/run-qemu.sh build/debug/projects/fault-analyzer/project-fault-bus.elf
python tests/system-smoke.py
```

Real firmware scenarios: project-fault-udf, -unaligned, -bus, -mpu, -escalated.
The separate project-fault-cases runs synthetic negative snapshot tests.
These are not presented as if real stacking faults had been generated.

In Debug/QEMU, CAUSE/FRAME/PRIVILEGE/CLEAR PASS in five real fault scenarios;
synthetic checks PASS for MSP/PSP/NS basic frames, stack-error rejection,
FP/cross-state rejection, bounds, and MMFAR/BFAR validity. HFSR clearing preserves
the Cortex-M33 rule to write DEBUGEVT=1; the generic CMSIS IMPRECISERR constant is
not interpreted as evidence that M33 BFSR bit2 is implemented. Raw CFSR/HFSR are
retained; the primary classification is not the entire root-cause chain.

Handler privilege is not inferred from CONTROL.nPRIV. Handler is always privileged.
Capture is not a safe emergency-stack mechanism on a corrupt MSP;
exception entry/reporting may already be unable to work in that situation.

The expanded negative set and GDB frame-boundary tests were added to the internal
audit gate; [results](../../docs/ENGINEERING_AUDIT.md) are recorded separately.
The student starter and reference decoder remain separate in
[Lesson 08](../../lessons/08-faults/).

[Projects](../README.md)
