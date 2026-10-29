**Language:** English | [Türkçe](README.tr.md)

# Project — Educational Mini Kernel

**STATUS: on main; Debug/Release/MinSize CI PASS (11fde900)**

Lesson 15 kernel/frame/context sources are compiled directly instead of maintaining
modified copies. Three small weak application hooks add only bounded work and
result reporting; the lesson's default behavior does not change. The ready hook
prevents completion before the queue drains; the 16 samples expected by the test
are not reduced. [application.c](src/application.c) uses an eight-slot ring buffer
between two tasks: the producer generates 0..15, and the consumer calculates the
sum. Counters and the queue are updated in the existing short observation critical
section; UART is used only in the completion Handler.

```bash
cmake --preset debug
cmake --build --preset debug --target project-kernel
./scripts/run-qemu.sh build/debug/projects/mini-kernel/project-kernel.elf
python tests/system-smoke.py
```

Debug/QEMU: PRODUCED=16, CONSUMED=16, QUEUE_SUM=120, CHECK_QUEUE=PASS.
The kernel's nine context/stack/progress checks also PASS. After the initial
cooperative yields, tasks progress through timer preemption. R4–R11 and EXC_RETURN
are saved; stack bounds are validated before STM. PSP belongs to tasks and MSP to
Handlers; the initial MSP frame is not reused.

This reference version uses privileged tasks/basic Secure frames; it provides no
FP task context, MPU task isolation, priority inheritance, or production RTOS
guarantee. Unprivileged and TrustZone integration will be verified separately in
the final project. The [Lesson 15](../../lessons/15-mini-kernel/) starter and
solution remain separate.

[Projects](../README.md)
