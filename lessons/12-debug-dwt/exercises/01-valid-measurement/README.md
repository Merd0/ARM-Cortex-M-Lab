**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Is This Measurement Usable?

Run the [lesson](../../README.md) first. This exercise does not make a real QEMU
cycle measurement; it interprets synthetic register snapshots. Complete the code
only through the two `TODO(student)` markers in [`measurement.c`](measurement.c).

## Contract

[`measurement.h`](../../measurement.h) defines the inputs and enum values.
All snapshots are assumed to have been taken in Secure state; the pointer is valid.

1. If there is I/O in the measurement interval, return `L12_CONTAMINATED` regardless of other fields.
2. Return `L12_UNAVAILABLE` if any of these hold: `NOCYCCNT=1`,
   `CYCDISS=1`, `CYCCNTENA=0`, `DEMCR.TRCENA=0`, or equal endpoints.
3. Otherwise return `L12_COUNTER_DELTA`. This indicates only that the counter
   difference is usable; it is not approval of a hardware benchmark or WCET.
4. Make `l12_delta()` calculate the modular difference between two `uint32_t`
   counter values. The caller must separately guarantee an interval shorter than
   `2^32` increments. For start `0xFFFFFFF0` and end `0x10`, the result must be 32.

This API returns enums; acceptance is not based on a string's first character.
Do not change the driver's expected results or platform code.

## Run

```bash
cmake --preset debug
cmake --build --preset debug --target 12-exercise-measurement
./scripts/run-qemu.sh build/debug/lessons/12-debug-dwt/12-exercise-measurement.elf
```

The starter reports **0/12**; all twelve checks FAIL and it reaches END.
Once complete, the same driver must report **12/12**. Repeat with `release` and
`minsize`. Remember that the main lesson test deliberately expects 0/12 for the
starter: we do not deliver the student solution over the main repository's starter.

## Diagnosis

- Did you check only for a zero counter? Try equal but nonzero endpoints.
- What happens if counter values differ but the enable bits are off?
- Can wrap be calculated correctly using absolute values or signed subtraction?
- Do equal endpoints mean zero work, or could they indicate a stopped counter or a full wrap?

For bit names, see vendored CMSIS `core_cm33.h`; consult the
[separate reference solution](../../solutions/01-valid-measurement/) only after your own attempt.
