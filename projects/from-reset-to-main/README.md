**Language:** English | [Türkçe](README.tr.md)

# Project — From Reset to Main

**STATUS: on main; Debug/Release/MinSize CI PASS (11fde900)**

Combines the Lesson 10–11 linker/startup relationship using a real initialized
calibration array and a zero-initialized application buffer. The working reset,
vector, and RAM-poisoning fixture are built from **the same source files** as the
Lesson 11 reference. The project adds [main](src/main.c) and a
[linker entry](layout.ld); the audited platform layout is INCLUDEd. No second
copy of startup code is maintained.

```bash
cmake --preset debug
cmake --build --preset debug --target project-reset
./scripts/run-qemu.sh build/debug/projects/from-reset-to-main/project-reset.elf
python tests/system-smoke.py
```

Reset first poisons the .data/.bss VMA using the fixture, then copies .data from
FLASH, zeros .bss, and calls SystemInit and main. This prevents QEMU's initially
zero RAM from hiding incomplete startup. This poisoning is a test tool, not a
production boot policy.

Debug/QEMU: CHECK_DATA, BSS, ORDER, VECTORS, VTOR, and LMA PASS.
Calibration 11/22/33; the buffer's 12 words are zero; phase=0x123.
Vector[0] is stack top, vector[1] Thumb Reset_Handler; reserved 8/9/10/13 are zero.
The .data LMA is in FLASH and its VMA in RAM; AT>FLASH and overflow ASSERTs are preserved.

The combined GDB gate will trace Reset_Handler → main and the LMA/VMA copy.
Student modification experiment: use Lesson 11's separate starter/solution targets;
do not replace the project's reference reset with a student TODO.

[Lesson 11](../../lessons/11-startup/) · [Projects](../README.md)
