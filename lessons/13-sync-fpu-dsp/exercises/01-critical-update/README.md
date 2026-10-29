**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Preserve the Critical Section

First run the [lesson's](../../README.md) lost-update experiment.
Complete the single TODO in [`critical.c`](critical.c). The starter intentionally
does no work; the driver reports **2/6 PASS**. Merely leaving the mask intact is insufficient.

## Contract

`l13_protected_increment(counter)` is called in Secure privileged Thread/MSP with
a valid, word-aligned Normal SRAM counter:

1. Save the current PRIMASK and mask maskable exceptions.
2. Read the counter; **call `l13_pend_update()` between the read and write**.
3. Add 1 to the value read and write it to the counter.
4. Restore the entry PRIMASK. Do not use unconditional `__enable_irq()`.

`l13_pend_update()` makes one SysTick exception pending; it does not configure a
timer. If needed, the caller unmasks and waits for the deferred exception to run.
Waiting for an IRQ while masked in this function would deadlock. Do not change the
driver, handler, or expected results. NMI/HardFault are outside this protection's scope.

```bash
cmake --preset debug
cmake --build --preset debug --target 13-exercise-critical
./scripts/run-qemu.sh build/debug/lessons/13-sync-fpu-dsp/13-exercise-critical.elf
```

The driver tests two cases: PRIMASK=0 on entry and PRIMASK=1 on entry.
In both, Thread and handler must each increment once, giving a total of 2.
On the second call, PRIMASK must still be 1 and the handler count 0 when the
function returns; the driver unmasks. A complete solution must give **6/6** in all
three presets. Do not mark the starter as solved; the main test expects 2/6 for it.

## Break / Debug

- If you mask only after the update, how is the stale read lost?
- When do you expect a pending exception to run while masked?
- Why does unconditional enable in nested protection violate the caller's contract?
- Does adding DMB alone make read–modify–write atomic?

[Separate reference solution](../../solutions/01-critical-update/).
