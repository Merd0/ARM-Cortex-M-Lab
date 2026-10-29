**Language:** English | [Türkçe](README.tr.md)

# Lesson 15 — Educational Mini Kernel

`[DEEP]` · **STATUS: on main; Debug/Release/MinSize CI PASS (11fde900)**

## TEACH — What must be saved so a task can resume?

Two privileged Thread tasks use separate PSP stacks. Handlers run on MSP.
Hardware saves the basic frame (R0–R3, R12, LR, PC, xPSR), while PendSV saves
R4–R11 and EXC_RETURN. After a fixed round-robin selection, exception return uses
the new PSP. This educational kernel does not provide FPU task context, MPU task
isolation, unprivileged tasks, or production RTOS guarantees.

Read the [context assembly](example/context.S), [kernel](example/main.c), and
[initial frame](example/frame.c) together. The software frame is 9 words and the
hardware frame 8 words. The software SP can be 4 mod 8; the hardware SP at the
public exception boundary is 8-byte aligned. Adding an arbitrary padding word to
eliminate this difference breaks the register mapping.

## SHOW — Startup and switching boundaries

The first SVC sets up CONTROL.SPSEL/PSP/PSPLIM without calling C, loads the first
register set, and sets MSP to the stack top. The old main/MSP frame is permanently
abandoned. The first task starts from its hardware frame; the old C frame is not
continued using PSP. Execution does not return to the bootstrap code after SVC.

PendSV accepts only the 0xFFFFFFFD basic Secure Thread/PSP return. FPU or other
frame types are not silently treated as supported. Outgoing stack bounds are
checked **before** STM; PSPLIM does not protect an STM addressed through R0.
The scheduler is called on MSP with 8-byte alignment. The short selection section
preserves the old PRIMASK value; the new context is not loaded before validation.

## RUN

```bash
cmake --preset debug
cmake --build --preset debug --target 15-mini-kernel 15-kernel-broken 15-exercise-frame 15-solution-frame
./scripts/run-qemu.sh build/debug/lessons/15-mini-kernel/15-mini-kernel.elf
python tests/development-smoke.py 15
```

## OBSERVE

In the Debug/QEMU run, the nine CHECK fields in the example and solution were PASS.
One recorded sample: TASK0_RUNS=719, TASK1_RUNS=1123, TICKS=4,
SWITCHES=20, ASYNC_SWITCHES=4. Task counters/model timing can vary;
the test does not fix these incidental numbers.

Tasks yield through SVC #0 for the first eight rounds. They then busy-loop without
yielding: real SysTick/PendSV preemption is required for the second task to progress.
ASYNC_SWITCHES counts switches where the saved PC is not the yield return point.
Each of R4–R11 holds a task-specific canary. Each task's lower/upper stack guards,
progress, yields, and timer switches are checked at completion.
Task observations also check CONTROL/IPSR, the PSP range, PSPLIM, and the current
TCB identity; Handler/MSP is tested separately.

Optional GDB observation (the comprehensive GDB gate was not run at this stage):

```text
break PendSV_Handler
print/x $psp
print g_l15_current
x/17wx g_l15_current->sp
print g_l15_runs
print g_l15_bad
```

At handler entry, the TCB's SP may not yet have been updated. Dump the TCB after
the assembly store or inside the scheduler. Breakpoints affect timing.

## MODIFY → BREAK → DEBUG

`15-kernel-broken` allocates the software context area but does not write R4–R11.
CHECK_REGISTERS=FAIL; IRQs are masked throughout the tasks' C observation call.
This makes the intentionally broken restore visible in the assembly canary check
instead of corrupting live C pointers and crashing unpredictably. Masking a C call
does not remove a production scheduler's obligation to preserve registers.

The initial-frame builder is incomplete in the
[exercise](exercises/01-initial-frame/README.md). NULL is rejected before PSP is
set up. The [solution](solutions/01-initial-frame/README.md) is in a separate file.

## VERIFY

The Debug build and four QEMU scenarios were run: example/solution PASS, incomplete
starter only CHECK_INITIAL_FRAME=FAIL, broken save only CHECK_REGISTERS=FAIL.
These tests do not establish WCET, fair time allocation, indefinite execution, or
hardware validation. The three-preset comparison, broad negative set, and
independent audit are to be performed at the combined gate.

## Quick Check

1. Why does exception entry not save R4–R11?
2. Why does PSPLIM not remove the need to validate PendSV's STM address separately?
3. Why is returning to the MSP frame abandoned by the first SVC an error?
4. Which observation distinguishes a switch outside yield?

[Answers](answers/quick-check.md)

## References

- [Cortex-M33 Devices Generic User Guide](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d): basic frames, PSP/MSP, exception return.
- [AAPCS32](https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst): callee-saved registers and public stack alignment.
- [Lesson 04](../04-stack-msp-psp/): the fresh C frame boundary.
- [Lesson 13](../13-sync-fpu-dsp/): FP frames unsupported by this kernel.

**Previous:** [14](../14-systick-pendsv/) · **Next:** [16](../16-trustzone-advanced/)

## Final combined gate

Three-preset clean builds/four QEMU scenarios PASS; GDB compared public MSP
alignment and saved/restored R4–R11/PSP/PSPLIM/EXC_RETURN.
Project hooks default to no-op/ready. The final system uses the same assembly in
an NS and unprivileged variant; switching is deferred while Secure context is
active. The earlier Debug-only record describes a historical stage;
[Progress](../../docs/PROGRESS.md) contains current results.
