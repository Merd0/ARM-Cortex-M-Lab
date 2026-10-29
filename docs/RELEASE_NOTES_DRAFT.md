**Language:** English | [Türkçe](RELEASE_NOTES_DRAFT.tr.md)

# Release notes draft — v1.0.0

> **Archived draft:** The [stable v1.0.0 release](https://github.com/Merd0/ARM-Cortex-M-Lab/releases/tag/v1.0.0)
> was published later. The statement below that a release was not yet created
> describes this draft's historical checkpoint, not the present status.

Proposal: **v1.0.0 — ARM Cortex-M Practical Lab**. This is the first complete
release proposal for this educational scope; it does not imply a production RTOS
or hardware certification. The tag and GitHub Release have not been created.
The exact target SHA is selected by verifying the merge commit of the audit fixes
in [PR #12](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/12) and then the main
three-preset CI result. The v1.0.0 tag/Release requires separate user approval.

- 16 practical lessons on Cortex-M33 / QEMU mps2-an505; separate incomplete
  exercises and reference solutions, controlled Break It / Debug It experiments.
- Five integration projects: Protected Application, Fault Analyzer,
  From Reset to Main, Performance Profiler, Educational Mini Kernel.
- Secure/Non-secure sensor node: three unprivileged tasks, MPU stack isolation,
  SVC services, PendSV scheduling, and limited Secure DSP/FP calibration.
- Debug/Release/MinSize builds, behavioral smoke/probes, negative scenarios,
  and GDB gates. Final results and actual findings are in the
  [internal audit record](ENGINEERING_AUDIT.md).
- Frame alignment, padding/mode validation, and stack-error negative tests were strengthened.
- The erroneous scheduler panic for a task interrupted at SG entry while still
  Non-secure was fixed; only known gateway entry addresses are accepted.

Limits: synthetic sensor; no hardware testing or third-party audit.
QEMU is not cycle-accurate; no performance or WCET claim is based on DWT CYCCNT.
A general TrustZone/FP-aware RTOS, cryptographic assurance, and protection against
physical attacks are outside this release's scope. For initial setup, see the
[README](../README.md).
