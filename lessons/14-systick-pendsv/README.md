**Language:** English | [Türkçe](README.tr.md)

# Lesson 14 — SysTick & PendSV

`[STANDARD]` · **STATUS: on main; Debug/Release/MinSize CI PASS (11fde900)**

## TEACH — A tick is not a scheduler

SysTick counts time; PendSV runs deferred work. The pending bit is not an event
queue: setting the same bit twice while masked does not accumulate two calls.
A scheduler is built on top of these mechanisms.

A smaller numerical priority means a higher priority. The tested AN505/QEMU target
has eight priority bits. Selecting SysTick=64 and PendSV=255 prevents PendSV from
preempting SysTick. These are not universal M33 constants. SysTick_Config also
writes the priority; the lesson reapplies its own setting after this call.

## SHOW → RUN

[main.c](example/main.c) demonstrates controlled exception ordering, pending-bit
coalescing, and actual timer progress in separate experiments. The
[priority configuration](example/priorities.c) is separate. Handlers do not print
to UART; results are printed in Thread mode.

```bash
cmake --preset debug
cmake --build --preset debug --target 14-systick-pendsv 14-pendsv-broken 14-exercise-priority 14-solution-priority
./scripts/run-qemu.sh build/debug/lessons/14-systick-pendsv/14-systick-pendsv.elf
python tests/development-smoke.py 14
```

## OBSERVE

Main results measured in Debug/QEMU:

```text
[EVENT0] 1
[EVENT1] 2
[EVENT2] 3
[CHECK_DEFERRED] PASS
[CHECK_PENDING_COALESCES] PASS
[TICKS] 4
[CHECK_TICK_RELOAD] PASS
[CHECK_TICK_PROGRESS] PASS
```

A 20 MHz platform clock / 1000 gives LOAD=19999. This does not claim wall-clock
accuracy measured on hardware. The experiment waits for four ticks; the PendSV
invocation count need not be exactly one per tick. Pending events can coalesce.
The ISR issues SEV, and Thread waits with WFE: even if the last tick occurs between
the condition check and sleep, the sticky event is not lost. A spin-iteration
budget that depends on optimization or host speed is not used as timer duration;
a stopped timer is treated as a failure by the external test timeout.

Optional GDB inspection (the detailed GDB gate was not run at this stage):

```text
break SysTick_Handler
break PendSV_Handler
print g_l14_phase
print g_l14_inside_tick
print g_l14_events
x/wx 0xE000E014
```

Breakpoints change timer timing. The timer is disabled in the first controlled
experiment; event 1 marks SysTick entry, 2 the end of SysTick, and 3 PendSV.

## MODIFY → BREAK → DEBUG

`14-pendsv-broken` sets PendSV to 0. The measured order is 1,3,2 and
CHECK_DEFERRED=FAIL: PendSV entered while SysTick was running. Inspect the event
array and `g_l14_nested`; reading only the final counter hides the error.

The [exercise](exercises/01-defer-work/README.md) omits the priority configuration.
Equal priorities at reset can pass the ordering experiment; this is why an
explicit configuration check is also necessary. The
[reference solution](solutions/01-defer-work/README.md) is separate.

## VERIFY

The Debug build and four QEMU scenarios were run. The example/solution pass all
checks; the starter produces the expected FAIL only for CHECK_PRIORITY, and the
broken version for CHECK_PRIORITY and CHECK_DEFERRED. Broad regression testing and
a detailed three-preset comparison belong to the combined quality stage.

## Quick Check

1. Why are two PENDSVSET writes not two work records?
2. Why can the starter still produce the correct event order?
3. Why do we configure the priority again after SysTick_Config?
4. Did this lesson switch tasks?

[Answers](answers/quick-check.md)

## References

- [Cortex-M33 Devices Generic User Guide](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d): ICSR, SysTick, exception priorities.
- [CMSIS implementation](../../external/CMSIS/Core/Include/core_cm33.h): SysTick_Config.

**Previous:** [13](../13-sync-fpu-dsp/) · **Next:** [15](../15-mini-kernel/)

## Final combined gate

Debug/Release/MinSize clean builds and four QEMU scenarios PASS. GDB confirmed that
SysTick had finished/inside_tick=0 at PendSV entry and that the first events were
1,2. The earlier Debug record is above; broader repository verification is in
[Progress](../../docs/PROGRESS.md).
