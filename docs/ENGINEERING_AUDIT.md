**Language:** English | [Türkçe](ENGINEERING_AUDIT.tr.md)

# Internal engineering audit — 2026-10-27

Starting main: `11fde900835d4443a55d0e7da64438ea4da4c690`.
Local branch: `codex/final-engineering-audit`. This is an internal engineering
audit conducted without automatically accepting previous development decisions;
it is not an independent third-party audit or security certification.
Code/test fix: `142b3b9059c646cf27d57c12929f4f0fb028fbaa`.
The subsequent documentation commit does not change the tested firmware sources.

## Actual findings and fixes

### H1 — High: a valid NS task interrupted at SG entry caused a scheduler panic

[kernel.c](../final-project/secure-real-time-sensor-node/nonsecure/kernel.c),
return-PC check in `valid()` (lines 51–64). The first clean matrix's Debug bad-svc
run panicked after 24 samples. The same error was caught naturally in one of
120 parallel targeted repetitions: saved EXC_RETURN `0xFFFFFFBC`, stacked PC
`0x10070008`, CFSR zero. This address is the `l16_service` SG entry. If interrupted
before SG executes, the context still belongs to NS Thread; this valid return
point must be possible in addition to the NS flash range. The old check rejected it.

Only the imported `l16_report` and `l16_service` SG addresses were added;
arbitrary Secure/NSC addresses were not opened. PC bit0 is also checked.
A controlled GDB fixture setting a real SysTick basic frame's PC to SG entry
FAILed with a panic on the old code and PASSed with restore after the fix.
Both SG entries and rejection of SG+4 are permanent tests. This fixture does not
claim to generate an actual pre-SG interruption; the natural panic record is
separate evidence.

### M1 — Medium: M33 frame alignment was insufficiently validated

[diagnostic.c](../projects/fault-analyzer/diagnostic.c), `fa_capture` address check
(lines 21–24); [cortex-m.gdb](../scripts/gdb/cortex-m.gdb), `cm-frame` address check
(lines 297–301), padding/mode checks (lines 350–362). Both paths checked only word
alignment. Cortex-M33 CCR[9] is RES1, so the exception-frame base is 8-byte aligned.
A synthetic buffer at an address 4 mod 8 was accepted by both C and GDB before
the fix. This can present incorrect crash data as a plausible frame; it is not
evidence of memory corruption in the live system.

The check was changed to 8-byte alignment; the fixture was explicitly aligned.
The GDB helper also validates that padding fits in the RAM window and that
EXC_RETURN Mode agrees with stacked IPSR. A new GDB session accepts valid
MSP/PSP/NS/Handler frames while rejecting wrong alignment, missing padding, and
mode mismatches in both directions. These checks do not prove a RAM buffer's
actual exception history.

### M2 — Medium: the stack-error negative test could pass for the wrong reason

[cases.c](../projects/fault-analyzer/src/cases.c), `faults` loop (lines 20–35);
the old test used only an out-of-bounds pointer. Since bounds checking already
rejected it, the capture test could pass even if the status veto were removed.
Three missing bits were added, and seven stack/unstack/lazy error bits are now
tested with a valid buffer; `fa_analyze` must also reject despite the caller's
`frame_valid=1` claim. A mutation temporarily removing the STKERR veto made the
new test FAIL; the mutation was reverted. No actual hardware stacking fault is
claimed to have been generated.

### M3 — Medium: published work had outdated status descriptions

For example, final-project/README.md line 3 and projects/README.md lines 22–26:
README, BOOTCAMP, PROGRESS, ROADMAP, Lesson 13–16, and project/final introductions
still said completed PRs #9–11 awaited CI/merge. Current status was tied to
[main CI 40105930630](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40105930630).
Historical local records were not deleted. ENVIRONMENT's Linux-only validation
claim and empty Windows version record were also corrected to reflect the actual
environment. This CI result is not attributed to the new local audit changes.

No Critical finding was confirmed within the reviewed scope. High finding H1 was
fixed; this is not a guarantee that all possible defects are absent or that the
system withstands attacks.

## Architectural review

- Lesson 15/final PendSV: bounds before the first indirect store; R4–R11 and
  EXC_RETURN save/restore; 8-byte MSP at C calls; restoring prior PRIMASK;
  no return to the old MSP C frame during bootstrap. Normal scheduling is limited
  to basic frames; interruption of Secure Thread uses the defer path with the
  same owner/token.
- Final MPU: 32-byte-aligned task-stack mappings, privileged kernel RAM,
  unprivileged NS tasks, Handler-only Secure processing. Queues are accessed
  serially inside SVC; SysTick touches only tick/pending counters. General RTOS/FP
  context switching or arbitrary Secure callbacks are not claimed.
- Lesson 13 and Secure FP leaf: integer public ABI, callee-saved registers,
  FPSCR controls; leaf S0/S1 clearing and FPCA/SFPA restoration. NS FPU permission
  is disabled. Eager/lazy experiments are observed in the separate Lesson 13 gate.
- Lesson 16: SAU/IDAU/MPC boundaries, NSC veneer, CMSE entry/call types, valid NS
  reset vector, Secure fault-address validity. Non-secure input payload is not
  accepted as a trusted pointer; the gateway uses a scalar API.
- Linker/startup: RAM/FLASH regions, `.data > RAM AT > FLASH`, NSC bounds,
  `.bss`/stack separation; libgcc order and effective Release `-O2` are preserved.
  Linker symbols/function addresses are GNU Arm bare-metal ABI contracts;
  this code is not presented as a host-portable ISO C program.
- Test contracts: expected FAIL sets for starters and intentionally broken
  firmware; complete CHECK key sets/END, independent integer pipeline oracle,
  real MMFAR/CFSR/GDB context observations. Existing assertions were not relaxed.

## Primary sources

- [Arm Cortex-M33 Devices Generic User Guide](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d),
  CCR tables 4-13/4-14: bit 9 RES1. This is M1's M33-specific basis.
- [Arm AAPCS32](https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst):
  public-interface SP alignment, callee-saved core/FP registers, FPSCR rules.
- [Armv8-M Architecture Reference Manual, SG](https://community.arm.com/cfs-file/__key/communityserver-discussions-components-files/471/DDI0553B_5F00_y_5F00_armv8m_5F00_arm.pdf),
  C2.4.180: the NS call target is SG; transition to Secure state occurs while SG
  executes. In H1, the PC address alone did not determine execution security state.
- [CMSIS 6.3 Armv8-M MPU API](https://arm-software.github.io/CMSIS_6/v6.3.0/Core/group__mpu8__functions.html):
  RBAR/RLAR 32-byte bounds and RO/NP/XN parameters compared with the vendored header.
- [GCC ARM options](https://gcc.sourceware.org/onlinedocs/gcc/ARM-Options.html):
  `-mgeneral-regs-only` does not constrain the assembler; the assembly FP leaf was reviewed separately.
- [QEMU NS alias fix](https://github.com/qemu/qemu/commit/4a934d284dfac9fa19b0f47874f12d9b3519c21c):
  the existing QEMU 8.2.2 workaround on main was not interpreted as an architectural address prohibition.

## Final quality gate

**Final local gate PASS.** Windows / xPack GCC 15.2.1 / QEMU 11.1.0 /
Arm GDB 16.3.90 / CMake 4.3.2 / Ninja 1.13.2; Debug, Release, and MinSize:

- Clean build: 109 ELF/preset, 0 warnings, 0 errors.
- Smoke: 566 PASS / 0 FAIL, build probes: 47 PASS / 0 FAIL, each preset.
- Lesson 12/13 runtime/GDB/ABI gates; Lesson 14–16's 12 scenarios PASS.
- The five projects' 15 scenarios and final system's 7 scenarios PASS; normal
  pipeline 32 samples / sum 298, telemetry matched the independent integer oracle.
- 13 GDB sessions/preset: the previous nine context/fault/FP/security sessions,
  frame boundaries, and three SG entry/return scenarios. When the SG single-step
  assertion was added last in Debug, only the two relevant sessions were rerun.
- Post-H1 Debug bad-svc stress: 120/120 PASS, six parallel QEMU processes.
- 402 relative Markdown links; `git diff --check` PASS. The CI matrix invokes all
  these test scripts in three presets; the workflow was not relaxed/changed.
- PowerShell environment check found all tools. Platform/CMSIS and Lesson 01–12
  runtime code did not change.

The initial gate's Debug system test FAILed due to H1; that record was retained.
After H1, all three presets were clean-built, and all affected system/GDB and
Lesson 12–16 gates were rerun. Earlier Release/MinSize smoke/probe evidence was
retained only after SHA-256 hashes matched exactly for the 95 rebuilt non-final
ELFs; identical tests were not repeated unnecessarily. Debug smoke/probes ran
with the final fix. The failed initial run is not presented as final success.

Raw records are under ignored `build/audit-final-2026-10-27/` (first run),
`build/audit-final-2026-10-27/after-sg-fix/` (result JSON/logs and ELF hashes),
`build/debug/audit-evidence/` (mutation, natural panic, stress), and
`build/<preset>/{system,development,lesson12,lesson13}-evidence/`.
These files were not added to Git; permanent reproduction tools are under tests/.

## Scope limits and publication decision

No real hardware testing; QEMU is not cycle-accurate. Since CYCCNT does not
advance, there are no cycle/throughput/WCET results. The sensor is synthetic,
the Secure service performs calibration; cryptographic identity, DMA/debugger
attacks, code injection/ROP, general Secure/FP task switching, and production
RTOS reliability were not verified. Interactive VS Code flow and physical-board
setup are not part of this gate.

At local delivery, the engineering gate had passed; the fixes were not yet
published. This historical result does not claim remote CI or merge had occurred.

The subsequent publication process is tracked in
[PR #12](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/12): no merge before
three-preset CI passes; main CI is also verified after merge. The exact merge SHA
and remote results belong to the PR/Actions records. Old `11fde900` does not
contain the H1 fix and is not the v1.0.0 target. Tag/Release and visibility changes
are outside this delivery and require separate approval.
[Release notes draft](RELEASE_NOTES_DRAFT.md).
