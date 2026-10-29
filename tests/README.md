**Language:** English | [Türkçe](README.tr.md)

# tests/

This directory is not for **unit testing**. Embedded unit testing is a separate
subject (requiring compilation and execution on the host) and is outside this
repository's scope.

Here we test firmware behavior, link contracts, debugger observations, and documentation links:

```bash
./tests/smoke-test.sh          # davranışsal regresyon (preset: debug)
./tests/build-probes.sh        # link/linker sözleşme probe'ları
python3 tests/check-links.py   # Markdown göreli link kontrolü
python3 tests/lesson12-check.py debug  # GDB / ölçüm sınırları
python3 tests/lesson13-check.py debug  # synchronization / FP frame / ABI
```

All run in CI for three presets (`.github/workflows/ci.yml`).

## `smoke-test.sh` — target classification

An independent audit showed that the old test missed a serious error: under
`-O2`/`-Os`, the Lesson 04 PSP solution produced **CHECK_REGION FAIL**, but the
test passed because it checked only text presence. Expected **behavior** is now
verified according to target type:

| Target type | Expected |
|---|---|
| Normal example | Correct result + completion marker, no `FAIL` |
| Reference solution | All `CHECK_*` lines `PASS` |
| Deliberately broken | **Declared** incorrect result/FAIL or fault; END depends on the contract |
| Incomplete assignment | Missing behavior; must not be reported as solved |

Seeing `===== END =====` **alone** never counts as success.

Scope: capability-probe (target identity, MPU/SAU regions, NVIC priority-bit
consistency), Lesson 02 (arithmetic + flags + intentional fault), Lesson 03
(declared wrong result of an ABI violation), Lesson 04 (three phases, push/pop
delta, stack guard, PSP solution).

Lesson 12 main closure record: **454 PASS / 0 FAIL**, all three presets
(GitHub Actions, Ubuntu 24.04 / QEMU 8.2.2). Lesson 13's new targets have also
been added; current local results are in [Progress](../docs/PROGRESS.md).

## `build-probes.sh` — link contracts

Permanently detects two audit findings:

- **I4 — libgcc ordering:** if `-lgcc` precedes object files, `__aeabi_uldivmod`
  remains unresolved. The probe checks that the wrong order fails and the right
  order succeeds, and that the library appears **after** objects in the
  repository's actual link command.
- **I5 — FLASH load-image boundary:** an image must produce a **link error** if
  `.text` + `.rodata` fit in FLASH but adding the FLASH copy of `.data` overflows it.
  QEMU also verifies that a within-bounds image's `.data` actually reaches RAM
  at runtime and that `.bss` is cleared.

Main closure record: **47 PASS / 0 FAIL**, three presets. Lesson 10/11
linker/startup probes are also in this script; earlier checks are not reduced.

## Lesson-local debugger and ABI checks

`lesson12-check.py`: real write/read watchpoints, backtrace, single-step,
rejection of I/O inside the measurement window, a stopped timer, and static
optimization/header comparisons. `lesson13-check.py`: controlled SysTick,
R4–R11/SP preservation, basic/eager/lazy FP frames, NOCP fault PC, lazy-slot
materialization, S0–S2 restore, caller FPSCR control-state preservation,
matching/mixed float-ABI link probes, and emitted DSP/FP instructions.

Both use stdlib Python and Arm GDB (`arm-none-eabi-gdb` or `gdb-multiarch`);
the GDB Python extension is not required. Missing tools are not presented as SKIP.
Lesson 13's negative targets catch missing retry and disabled FPU permission;
link probes catch softfp/hard mixing in both directions. Two temporarily compiled
incorrect critical-section solutions also verify that the appropriate check
rejects unprotected updates and unconditional IRQ enabling.

Raw evidence remains under ignored `build/<preset>/lesson12-evidence/` and
`build/<preset>/lesson13-evidence/`; the workflow uploads it. GDB's QEMU watchpoint
is not considered physical DWT comparator/cycle support.

## Why so few?

Deliberately. The lessons' main verification is performed **manually** with GDB,
because that is the learning objective. Automated tests exist to ensure the
infrastructure has not broken, not to replace learning.

Because firmware ends in infinite loops, QEMU is stopped with `timeout`
(`QEMU_TIMEOUT`, default 8 s). The test terminates only processes it started.

---

[← Home](../README.md)

## Lessons 14–16 — fast development gate

```bash
cmake --preset debug
cmake --build --preset debug
python tests/development-smoke.py all
# Tek ders: python tests/development-smoke.py 15
```

Stdlib Python; checks each image's END marker, the complete set of CHECK keys,
and expected PASS/FAIL results. 12 scenarios: example, deliberate error,
incomplete starter, and separate solution for each lesson. Lesson 16 loads
Secure kernel ELF + Non-secure loader ELF together each time; one image is insufficient.

Basic scope includes PendSV ordering/pending coalescing, real timer preemption,
R4–R11 canaries, stack guards, initial-frame rejection, NSC returns, AUVIOL, and
INVEP. SFAR is used as an address only when SFARVALID is set. Raw records are kept
under ignored `build/<preset>/development-evidence/`. Processes are stopped only
using the PID of the QEMU instance started by the test.

The gate is attached to all three CI presets; three-preset CI passed for main
11fde900. `--preset release` / `--preset minsize` run the same gate with the other
optimizations. Existing smoke, build-probes, and GDB scripts were not reduced.
This fast test is not a full architecture/security audit or hardware test.

## Projects and final integration gate

```bash
python tests/system-smoke.py --preset debug
python tests/integration-check.py --preset debug
python tests/profile-report.py --preset debug
```

The same commands were run for release/minsize. System gate: 15 project + 7 final
scenarios. GDB gate: 13 sessions (including frame boundaries and three SG entry/
return tests); R4–R11 save/restore and EXC_RETURN, PSP/PSPLIM/MSP alignment,
FP payload/control, MPU/STKOF, and Secure transitions. Controlled NS SysTick tests
preservation of the Secure Thread continuation without changing its owner;
GDB does not mistake tail-chaining for an immediate return to Thread.

The diagnostic API privilege test and rejection of Thread-origin FP service calls
measure the expected policy. Queue completion does not pass by reducing the
sample count. CI steps are attached to all three presets. Local audit-fix results
are kept separate from PR/main CI success; see the [audit record](../docs/ENGINEERING_AUDIT.md).
Local gates are not an independent third-party audit or board test.

Frame regressions: 8-byte M33 alignment, padding bounds, Thread/Handler IPSR
consistency, T/PC, and seven stacking/unstack/lazy error bits. Error bits are
tested with a valid buffer; using only an invalid pointer lets bounds checks
hide a faulty status check. A STKERR-veto mutation makes the new test FAIL.
