**Language:** English | [Türkçe](README.tr.md)

# Reference Solution — Critical Update

[`critical.c`](critical.c) saves the previous PRIMASK, masks, performs the
read–pend–write steps, and restores **the saved mask**. It is not linked into the starter.

If interrupts were unmasked on entry, pending SysTick runs when they are unmasked
again. If already masked, the function leaves them masked; the handler runs after
the driver unmasks. The PRIMASK critical section covers maskable exceptions in this
Secure single-core lab; it does not provide a lock against NMI/HardFault, DMA, or
multiple cores. `volatile` keeps accesses visible here; it is not claimed as a
replacement for C11 thread atomics.

```bash
cmake --preset debug
cmake --build --preset debug --target 13-solution-critical
./scripts/run-qemu.sh build/debug/lessons/13-sync-fpu-dsp/13-solution-critical.elf
```

Local debug/release/minsize: **6/6 PASS**, END. The entry mask and total are tested
separately. Only `counter++`, or unconditional IRQ enabling, does not meet this contract.

[← Exercise](../../exercises/01-critical-update/)
