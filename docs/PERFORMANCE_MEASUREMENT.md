**Language:** English | [Türkçe](PERFORMANCE_MEASUREMENT.tr.md)

# Performance Measurement Methodology

## Limit on this target

The DWT cycle counter **does not count** on this target (measured; see
`docs/TARGET_CAPABILITIES.md`). The most common Cortex-M profiling method
therefore cannot be used here.

```text
DWT CYCCNT PROFILING: NOT SUPPORTED ON CURRENT TARGET (QEMU mps2-an505)
```

We therefore use the following measurement methods.

## Valid measurements in QEMU

### 1. Code size (reliable)

```bash
arm-none-eabi-size build/debug/.../firmware.elf
arm-none-eabi-nm --size-sort -S build/debug/.../firmware.elf | tail -20
```

The difference between `-O0` and `-Os` is real and reproducible here.

### 2. Generated instruction count and instruction form (reliable)

```bash
arm-none-eabi-objdump -d -S firmware.elf > firmware.lst
```

Counting instructions in a function's `-O0` and `-O2` versions is the most direct
way to understand what optimization does.

### 3. Static instruction counting

```bash
arm-none-eabi-objdump -d firmware.elf | grep -c "^\s*1[0-9a-f]*:"
```

### 4. QEMU `-d in_asm` — recording translated blocks

```bash
qemu-system-arm -M mps2-an505 -kernel firmware.elf -nographic \
                -d in_asm -D trace.log
```

`-d in_asm` logs disassembly of the basic blocks (translation blocks) QEMU
**translates**; it does not give the executed instruction count. QEMU translates
a block once and **does not log it again**, however many times it subsequently
executes it. Therefore:

- Even if a loop runs 1.000.000 times, it appears once in the log.
- The number of log lines is not proportional to the executed instruction count.
- A code path never translated never appears.

`-d in_asm` is a tool for seeing **which code actually ran** (coverage).
It does not answer "how many instructions executed?"

QEMU has separate mechanisms for dynamic instruction counting (such as TCG
plugins). These are outside this repository's scope and are not installed.
If used, the measurement's purpose must be defined beforehand. QEMU is a
functional emulator; instruction count is not cycle count.

### 5. SysTick — uses and limits

SysTick works in QEMU and can establish a **local interval/counting experiment**
on the target: "how many iterations ran within this many ticks" or "did this
operation finish between two ticks?" This requires a suitable timer model and
knowing what is being counted.

Such a measurement cannot establish that one algorithm is faster than another
on physical Cortex-M hardware:

- Functional-emulator time is not hardware cycle time. QEMU executes instructions
  correctly but does not model pipeline/memory latency/cache.
- A SysTick result therefore establishes neither hardware performance ranking nor WCET.
- Even when comparing two paths in the same firmware, the resulting ratio cannot
  be assumed to hold on hardware; memory latency and instruction timings there
  are completely different.

SysTick here is an **experimental tool**, not a **benchmarking tool**.

## Seven measurement concepts

The seven concepts below often appear in the same sentence but are not
interchangeable. State explicitly which you measured when sharing a result.

| # | What | What it provides | In this repository |
|---|---|---|---|
| 1 | Static instruction/disassembly analysis | Code size, instruction count (static) | ✅ used |
| 2 | Translated-block logging (`-d in_asm`) | Which code was translated | ⚠️ NOT a counter |
| 3 | Dynamic executed-instruction counting | How many instructions ran | ⬜ not set up |
| 4 | Functional-emulator time (wall clock) | Nothing (host-dependent) | ❌ not used |
| 5 | Explicit timing/cycle-approximate model | Approximate cycles | ⬜ absent |
| 6 | Cycle measurement on real hardware | Actual cycles | ⬜ no hardware |
| 7 | Worst-case execution-time analysis (WCET) | Proven upper bound | ⬜ outside scope |

Two common mistakes:

- **Observed maximum ≠ WCET.** Measurement covers only the inputs you ran.
  You can write "observed maximum X," not "WCET is X." Real WCET analysis requires
  static-analysis tools and usually certification.
- **An FVP is not automatically cycle-approximate.** An ordinary Fixed Virtual
  Platform is a **functional** model and does not provide cycle counts.
  Timing information comes only from models explicitly documented to provide it.
  Verify what each model offers in its product documentation.

## What to avoid when measuring

1. **Do not perform console I/O inside the measurement region.** The UART wait loop overwhelms everything.
2. **Do not let the compiler discard the result.** Write it to a `volatile`
   variable; otherwise `-O2` can eliminate the entire calculation.
3. **Warm-up/repetition.** Repeat several times rather than measuring once.
4. **Compare with the same flags.** When comparing one firmware at `-O0` with
   another at `-O2`, ensure that only optimization changes.
5. **Do not report absolute timing.** Do not describe a QEMU result as "this many microseconds."

## Report format

When sharing a measurement, include at least:

```text
Target      : QEMU 8.2.2 mps2-an505 / cortex-m33
Toolchain   : arm-none-eabi-gcc 13.2.1
Optimization: -O0 / -O2
Olcu        : kod boyutu (byte) + komut sayisi
Gecersiz    : cycle / wall-clock (bu target'ta anlamsiz)
```

## What changes on real hardware?

If access permissions, the presence of DWT/a cycle counter, and actual counter
progress are separately verified on the board, `DWT->CYCCNT` allows:

- cycle-based measurement
- observation of flash wait-state, cache, and branch-prediction effects
- different rankings of the same code than in QEMU

Details: `docs/REAL_HARDWARE_MIGRATION.md`

## Working experiment

[Lesson 12](../lessons/12-debug-dwt/) runs readback, the nonworking cycle counter,
a SysTick local interval, I/O-contaminated measurement, and O0/O2 comparison of
only the workload. `tests/lesson12-check.py` repeats disassembly and actual GDB
watchpoint checks across three presets.
