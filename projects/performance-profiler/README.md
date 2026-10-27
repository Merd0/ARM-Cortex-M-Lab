**Language:** English | [Türkçe](README.tr.md)

# Project — Performance Profiler

**STATUS: on main; Debug/Release/MinSize CI PASS (11fde900)**

Lesson 12's runtime-input workload and reference measurement classifier are reused
from the same sources. The project produces five different workload objects:
-O0, -O1, -O2, -O3, -Os. Only the workload flag changes; the preset's general
Release=-O2 contract remains unchanged. The driver verifies checksum=424 in every variant.

```bash
cmake --preset debug
cmake --build --preset debug
python tests/system-smoke.py
python tests/profile-report.py
```

Debug/GCC 15.2.1 measurement: workload sizes are 70,32,32,32,28 bytes respectively;
static instruction counts are 35,13,13,13,11. The script produces
`build/debug/system-evidence/profile.json` from actual nm/objdump output.
These are not executed instruction counts, cycles, speedups, or WCET;
they may change with the toolchain.

DWT did not advance on QEMU 11.1.0 AN505; DWT_RESULT=UNAVAILABLE was measured.
The work progressing over four model-local SysTicks is counted; there is no
console I/O in the measurement window. Iterations do not establish physical
throughput or a reliable optimization ranking. Smaller code does not mean faster
code. FVP timing models also depend on configuration; not every FVP is assumed
cycle-accurate. Hardware performance requires separate measurement.

Negative measurement classification and the student starter/solution are preserved
in [Lesson 12](../../lessons/12-debug-dwt/). Detailed GDB checks and the
three-preset comparison belong to the combined quality gate.

[Projects](../README.md)
