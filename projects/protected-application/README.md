**Language:** English | [Türkçe](README.tr.md)

# Project — Protected Application

**STATUS: on main; Debug/Release/MinSize CI PASS (11fde900)**

[main.c](src/main.c) establishes a limited SVC API for a protected counter.
Lesson 06's actual PSP transition, SVC entry, and single-store probe assembly
sources are recompiled. CONTROL.SPSEL is not changed inside the old MSP C frame:
assembly branches to a fresh Thread C entry on the new PSP.

The MPU opens only FLASH as unprivileged RO/execute and the app stack/result block
as RW/XN. The kernel counter is not included in the open regions. Handler uses MSP;
PRIVDEFENA permits the privileged default map. An nPRIV=1 task does not elevate
itself; the application has no privilege-restore service.

```bash
cmake --preset debug
cmake --build --preset debug --target project-protected-normal project-protected-denied
./scripts/run-qemu.sh build/debug/projects/protected-application/project-protected-normal.elf
python tests/system-smoke.py
```

SVC #1 adds only a scalar value in [0,31] to the counter. Calls with 1,2,3 return
1,3,6; 32 is rejected and the counter remains unchanged. The SVC frame is validated
through the Fault Analyzer capture boundary before decoding. SVC #2 reports the
result from the privileged Handler. UART need not be exposed to the unprivileged task.

Debug/QEMU normal: UNPRIV_PSP/SVC/REJECT/HANDLER/NO_FAULT PASS.
In the denied version, a direct store produces MemManage with IPSR=4, CFSR=0x82,
and the correct MMFAR; the frame is valid, kernel counter=6 and HFSR=0 remain.
This expected fault is a PASS.

ISB synchronizes the instruction stream after a CONTROL update;
it does not establish a memory-permission policy by itself. MSP/PSP selection and
privilege are separate. The student MPU/SVC exercise remains separate in
[Lesson 06](../../lessons/06-mpu/).

[Fault Analyzer](../fault-analyzer/) · [Projects](../README.md)
