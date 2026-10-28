**Language:** English | [Türkçe](PROGRESS.tr.md)

# Progress

This section shows current progress. Only completed and verified work is marked.
For a student to complete a lesson:

1. Build the firmware.
2. Run it in QEMU and record its output in the README.
3. Make the GDB observation.
4. Try the Experiment and Break It activities.
5. Complete "What I Learned".

"What I Learned" is the student's personal record; it is not filled in as part of a technical delivery.
2026-10-24 technical delivery: 16 lessons, five projects, and the final system were implemented.
Lesson 13 was merged through [PR #9](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/9);
[run 40079336979](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40079336979)
passed debug/release/minsize. Merge commit:
`161d93d94b2aad62f5a31b3454c9d61f76b561d1`.
Lessons 14–16 were merged through [PR #10](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/10)
after CI passed for all three presets
([run 40101724183](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40101724183));
merge commit: `669257b11a672bbcd5abf718e0360b891c5e9547`.
The project/final work was merged through [PR #11](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/11):
`11fde900835d4443a55d0e7da64438ea4da4c690`.
[Main CI 40105930630](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40105930630)
passed debug/release/minsize. The technical record of the 2026-10-27 internal engineering audit is
in a [separate report](ENGINEERING_AUDIT.md); publication/CI/merge status is tracked in
[PR #12](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/12). Local testing/technical review and CI do not constitute an independent
third-party audit or hardware certification. The student's personal
completion criteria remain separate. The phased plan is in the [Roadmap](ROADMAP.md).

## Infrastructure

| Item | Status |
|---|---|
| CMake + Ninja + arm-none-eabi build system | ✅ |
| CMSIS-Core 6.3 integration (baseline core API) | ✅ (verified with a clean build + QEMU + GDB) |
| Teaching header `cm33_teaching.h` (for the lessons) | ✅ |
| Markdown link check (`tests/check-links.py`) | ✅ latest run count below |
| CMake presets (debug/release/minsize) | ✅ |
| MPS2-AN505 platform port (startup, linker, UART) | ✅ |
| Firmware running in QEMU | ✅ (verified) |
| GDB connection + breakpoint | ✅ (verified) |
| Cortex-M GDB helper commands | ✅ (verified) |
| Environment check scripts | ✅ Bash and PowerShell environment checks verified (2026-10-27) |
| Target capability probe | ✅ (measured) |
| Documentation foundation | ✅ |
| Smoke test (`tests/smoke-test.sh`) | ✅ behavioral regression; current result below |
| CI (`.github/workflows/ci.yml`) | ✅ three presets; build/regression/debugger/project/final gates; remote results at the Actions link above |
| VS Code tasks + launch | ✅ written / ⬜ not tested on Windows |
| Project and final-project definitions | ✅ on main; three-preset CI PASS (11fde900) |

## Lessons

| # | Lesson | Explanation | Code | Run | GDB | What I Learned |
|---|---|---|---|---|---|---|
| 01 | Cortex-M Family | ✅ | — | ✅ (probe) | ✅ | ⬜ |
| 02 | Programmer Model | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 03 | C and Assembly | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 04 | Stack, MSP & PSP | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 05 | Privilege & SVC | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 06 | MPU | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 07 | Exceptions & NVIC | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 08 | Faults | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 09 | Compiler & AAPCS | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 10 | Linker, ELF & Memory | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 11 | Startup (Reset → main) | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 12 | Debug & DWT | ✅ | ✅ | ✅ main/CI | ✅ three presets | ⬜ |
| 13 | Sync, FPU & DSP | ✅ | ✅ | ✅ PR #9 CI | ✅ three presets | ⬜ |
| 14 | SysTick & PendSV | ✅ | ✅ | ✅ three presets | ✅ three presets | ⬜ |
| 15 | Mini Kernel | ✅ | ✅ | ✅ three presets | ✅ three presets | ⬜ |
| 16 | TrustZone & Advanced | ✅ | ✅ | ✅ three presets | ✅ shared boot/final; no separate standalone GDB claim | ⬜ |

## Verification Record

Previous independent audit fixes are preserved in Git history. The baseline
regression passed again on Windows on 2026-10-23: three presets, 48 existing
ELFs/preset, smoke 369/0, and probes 47/0.

Lesson 12 delivery, 2026-10-23 Windows / GCC 15.2.1 / QEMU 11.1.0:

| Check | Result |
|---|---|
| Clean build, three presets | ✅ 55 ELFs / preset, 0 warnings / 0 errors |
| `tests/smoke-test.sh` | ✅ 454 PASS / 0 FAIL, three presets |
| `tests/build-probes.sh` | ✅ 47 PASS / 0 FAIL, three presets |
| `tests/check-links.py` | ✅ 337 links, 0 broken |
| `tests/lesson12-check.py` | ✅ three presets; write/read watchpoint, backtrace, single-step, stopped-timer and I/O negative checks, header/optimization comparison |
| Lesson 04 PSP solution (debug/release/minsize) | ✅ SPSEL + REGION + GUARD all PASS |
| GDB frame decoding (MSP/PSP origins + rejected cases) | ✅ |

## Historical Delivery Records

The dated records below describe their delivery point; current publication status is above.

## Lesson 13 Local Delivery — 2026-10-24

Windows / GCC 15.2.1 / QEMU 11.1.0, debug/release/minsize:

- Clean build: **64 ELFs/preset**, 0 warnings / 0 errors.
- Full smoke/regression: **566 PASS / 0 FAIL**; the previous 454 checks were preserved.
- Existing build probes: **47 PASS / 0 FAIL / 0 SKIP**.
- `lesson12-check.py`: all three presets passed again.
- `lesson13-check.py`: nine targets, six GDB scenarios/preset;
  R4–R11/SP, NOCP PC, eager/lazy raw frame, S0–S2 and FPSCR control state,
  and DSP/ABI metadata checks PASS. Two mixed ABI link probes and two
  incorrect critical-section solutions were rejected as expected.
- Relative link check: **358 links, 0 broken**; `git diff --check` PASS.

The FP worker's unnecessary FPSCR reset was removed during the final AAPCS review.
FZ=1, selected by the caller through an explicit environment support function, is preserved;
the saved FPSCR and post-call value are compared in GDB. Preserving this control state
does not verify denormal arithmetic or hardware performance.

Lesson 13 was subsequently pushed to PR #9; all three remote CI jobs passed.
PR #9 was then merged with user approval. This does not claim that an independent audit
or hardware verification has been completed.

### Lessons 14–16 — Fast Development Gate, 2026-10-24

Windows / GCC 15.2.1 / QEMU 11.1.0, local branch:
- Debug build of the entire repository PASS; 80 ELFs in total (16 new ELFs).
- `python tests/development-smoke.py all`: all 12 QEMU scenarios
  produced their expected results; the full set of CHECK fields and expected FAILs are verified.
- 14: SysTick/PendSV order 1,2,3; high-priority PendSV 1,3,2;
  two masked pending writes produce a single dispatch; timer runs for four ticks.
- 15: two tasks on PSP, real timer preemption, R4–R11 canaries,
  stack guards, SVC yield; safe rejection of an incomplete frame, expected FAIL for broken save.
- 16: four separate Secure/NS image pairs; valid scalar NSC calls,
  rejection of out-of-bounds inputs, Secure read AUVIOL=0x08, and bypassing SG INVEP=0x01.
  SFAR is not interpreted as an address when SFARVALID is clear.
- Selected older Debug scenarios were rerun: Lesson 04 PSP solution,
  Lesson 08 BusFault snapshot/clear, Lesson 11 startup, and Lesson 13 lazy FP PASS.
  This is not a rerun of the entire older broad smoke suite.
- The new fast gate was added to the CI workflow; the branch had not yet
  been pushed during this initial local gate. The current publication record is at the top of this document.
- The broad smoke/probe suite, extensive GDB checks, Release/MinSize comparisons,
  broad negative tests, and independent architectural audit were not run at this stage.
  The combined quality gate will cover these; project/final integration has not started.

## Exercise / Solution / Answers

| Lesson | Exercise | Solution | Answer Key |
|---|---|---|---|
| 01 | scenario writing (no code) | — | `lessons/01-cortex-m-family/answers/reference-answers.md` |
| 02 | `exercises/01-decode-apsr-flags/` | `solutions/01-decode-apsr-flags/` (run ✅) | `lessons/02-programmer-model/answers/quick-check.md` |
| 14 | `exercises/01-defer-work/` (priority missing) | `solutions/01-defer-work/` (Debug PASS) | `lessons/14-systick-pendsv/answers/quick-check.md` |
| 15 | `exercises/01-initial-frame/` (frame incomplete, no launch) | `solutions/01-initial-frame/` (Debug PASS) | `lessons/15-mini-kernel/answers/quick-check.md` |
| 16 | `exercises/01-secure-service/` (fail-closed) | `solutions/01-secure-service/` (Debug PASS) | `lessons/16-trustzone-advanced/answers/quick-check.md` |
| 13 | `exercises/01-critical-update/` (2/6, incomplete) | `solutions/01-critical-update/` (6/6 PASS) | `lessons/13-sync-fpu-dsp/answers/quick-check.md` |
| 12 | `exercises/01-valid-measurement/` (0/12, incomplete) | `solutions/01-valid-measurement/` (12/12 PASS) | `lessons/12-debug-dwt/answers/quick-check.md` |
| 11 | `exercises/01-reset-sequence/` | `solutions/01-reset-sequence/` (run ✅) | `lessons/11-startup/answers/quick-check.md` |
| 10 | `exercises/01-locate-your-data/` | `solutions/01-locate-your-data/` (run ✅) | `lessons/10-linker-elf-memory/answers/quick-check.md` |
| 09 | `exercises/01-callee-save/` | `solutions/01-callee-save/` (run ✅) | `lessons/09-compiler-aapcs/answers/quick-check.md` |
| 08 | `exercises/01-fault-report/` | `solutions/01-fault-report/` (run ✅) | `lessons/08-faults/answers/quick-check.md` |
| 07 | `exercises/01-decode-exception-entry/` | `solutions/01-decode-exception-entry/` (run ✅) | `lessons/07-exceptions/answers/quick-check.md` |
| 06 | `exercises/01-thread-mpu-policy/` | `solutions/01-thread-mpu-policy/` (run ✅) | `lessons/06-mpu/answers/quick-check.md` |
| 05 | `exercises/01-add-svc-service/` | `solutions/01-add-svc-service/` (run ✅) | `lessons/05-privilege-svc/answers/quick-check.md` |
| 04 | `exercises/01-activate-psp/` (+ optional DEEP) | `solutions/01-activate-psp/` (run ✅) | `lessons/04-stack-msp-psp/answers/quick-check.md` |
| 03 | `exercises/01-sub-numbers/` | `solutions/01-sub-numbers/`, `solutions/02-multiply-add/` (both run ✅) | `lessons/03-c-and-assembly/answers/quick-check.md` |

## Projects

| Project | Status |
|---|---|
| Protected Application | ✅ main three-preset CI PASS (11fde900) |
| Fault Analyzer | ✅ main three-preset CI PASS (11fde900) |
| From Reset to Main | ✅ main three-preset CI PASS (11fde900) |
| Performance Profiler | ✅ main three-preset CI PASS (11fde900) |
| Mini Kernel | ✅ main three-preset CI PASS (11fde900) |
| Final Project | ✅ main three-preset CI PASS (11fde900) |

## Learning Objective Checklist

Work intended to be completed by the end of the project:

- [ ] I inspected the Cortex-M register model in the debugger.
- [ ] I followed the assembly corresponding to C code.
- [ ] I applied the AAPCS calling convention rules.
- [ ] I switched between MSP and PSP.
- [ ] I observed Thread Mode and Handler Mode behavior.
- [ ] I used privileged and unprivileged execution.
- [ ] I called a privileged service with SVC.
- [ ] I established memory protection with the MPU.
- [ ] I examined MemManage, UsageFault, BusFault, and HardFault behavior.
- [ ] I decoded the stack frame created during an exception.
- [ ] I performed my own linker/memory layout experiments.
- [ ] I analyzed ELF and map files.
- [ ] I implemented the startup flow from reset to main.
- [ ] I debugged Cortex-M with GDB.
- [ ] I tried profiling with DWT (and documented its limitation on this target).
- [ ] I used SysTick.
- [ ] I performed a context switch using PendSV.
- [ ] I developed a small educational scheduler/kernel.
- [ ] I implemented TrustZone Secure/Non-secure separation.
- [ ] I experimentally examined advanced features such as DSP/FPU/MVE.

## Updating

When a lesson is finished, update this table, the `README.md` progress section,
and the `BOOTCAMP.md` checkboxes together.

## Latest Combined Local Delivery — 2026-10-24

- Debug/Release/MinSize: clean build, 0 warnings / 0 errors in each.
- Existing broad smoke suite: **566 PASS / 0 FAIL**, every preset.
- Build probes: **47 PASS / 0 FAIL**, every preset. The existing libgcc/FLASH LMA
  and linker/startup negative checks were not reduced.
- Lessons 14–16: **12 QEMU scenarios**, PASS with the expected starter/broken FAILs.
- Five projects: **15 independent QEMU scenarios**, PASS in every preset.
- Final: **7 Secure/NS image pairs**, PASS in every preset. 32 samples, sum 298;
  four telemetry records were compared with a separate integer oracle.
- Lesson 12 and 13 debugger/ABI gates: three presets PASS.
- integration-check.py: **9 GDB sessions** per preset. Deferred PendSV;
  Secure/NS R4–R11 save/restore, MSP public alignment and PSP/PSPLIM;
  Secure DSP/FP result, FPSCR and FPCA/SFPA restore, S0/S1 clearing;
  MPU/cross-task/STKOF/SecureFault and Secure-call defer/continuation were verified.
- Static profiler: five real optimization variants; the measurement is not presented as cycle/throughput/WCET.
  Latest report: build/<preset>/system-evidence/profile.json.
- Shared platform, CMSIS, debugger helpers, and Lessons 01–13 sources
  have not changed since the start of the Lesson 13 PR.

Fixes made at the gate: SEV/WFE instead of Lesson 14's optimization-dependent short polling budget;
preventing the kernel project from exiting before the queue completes;
rejecting bootstrap requests from tasks; diagnostic privilege policy;
returning without a context switch when Secure Thread is interrupted; a
Handler-only policy for the limited FP service and explicit FP payload/control clearing.

Results are from the local GCC 15.2.1 / QEMU 11.1.0 / Arm GDB 16.3.90 environment.
This section records the local gate; GitHub Ubuntu/QEMU 8.2.2 project/final CI
results are tracked separately in the relevant PR/commit's Actions record.
PR #9's earlier three-preset CI success is not presented as the new branch's CI result.
No new tag/Release was created. No independent third-party audit,
real-board validation, or general Secure/FP task-switching validation was performed.
