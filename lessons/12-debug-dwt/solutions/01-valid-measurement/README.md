**Language:** English | [Türkçe](README.tr.md)

# Reference Solution — Measurement Validity

[`measurement.c`](measurement.c) implements the [same contract](../../measurement.h).
It is linked separately from the starter; it does not make the starter complete.

I/O is checked first. Then the absence of a cycle counter, counting being disabled
in Secure state, enable bits, and equal endpoints are examined. Only suitable input
receives `L12_COUNTER_DELTA`. `end - start` gives a modular difference because of
unsigned arithmetic. You cannot recover multiple wrap counts from two endpoints;
for example, equal endpoints can also hide one complete wrap.

```bash
cmake --preset debug
cmake --build --preset debug --target 12-solution-measurement
./scripts/run-qemu.sh build/debug/lessons/12-debug-dwt/12-solution-measurement.elf
```

Run on debug/release/minsize: **12/12 PASS**, END.
Forward/wrap differences are 32; stopped counters at zero and nonzero values,
disabled TRCENA, disabled CYCCNTENA, NOCYCCNT, CYCDISS, and I/O cases are correctly distinguished.

These results are for synthetic inputs. They do not mean DWT works in QEMU.
Actual firmware snapshots are in the [lesson's measurement section](../../README.md).
