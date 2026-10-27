**Language:** English | [Türkçe](COURSE_MAP.tr.md)

# Course Map - Coursera Arm Cortex-M Specialization ↔ Lessons

This repository is **not** a copy of a course. All explanations, code, and examples
are original and verified against official Arm/CMSIS/QEMU documentation
(`docs/REFERENCES.md`).

This table only tracks **which lesson corresponds to which course topic**.
You do not need access to the course to use this repository.

## Course 1 - Cortex-M Processor Family

| Course topic | Lesson |
|---|---|
| Cortex-M family, processor selection | [01 - Cortex-M Family](../lessons/01-cortex-m-family/) |
| Performance / power / security trade-offs | 01 |

## Course 2 - Armv8-M Architecture Fundamentals

| Course topic | Lesson |
|---|---|
| Programmer model, registers, xPSR | [02 - Programmer Model](../lessons/02-programmer-model/) |
| T32/Thumb, assembly fundamentals | [03 - C and Assembly](../lessons/03-c-and-assembly/) |
| Stack, full descending, MSP/PSP | [04 - Stack, MSP & PSP](../lessons/04-stack-msp-psp/) |
| Thread/Handler Mode, CONTROL, privilege, SVC | [05 - Privilege & SVC](../lessons/05-privilege-svc/) |
| Memory model, MPU | [06 - MPU](../lessons/06-mpu/) |
| Exceptions, NVIC, priorities | [07 - Exceptions](../lessons/07-exceptions/) |
| Faults, escalation, fault status registers | [08 - Faults](../lessons/08-faults/) |

## Course 3 - Cortex-M Software Development Fundamentals

| Course topic | Lesson |
|---|---|
| Compiler, optimization, AAPCS | [09 - Compiler & AAPCS](../lessons/09-compiler-aapcs/) |
| Object files, ELF, linker, memory map, scatter-loading concepts | [10 - Linker, ELF & Memory](../lessons/10-linker-elf-memory/) |
| Startup, initialization, CMSIS, reset→main | [11 - Startup](../lessons/11-startup/) |
| Debugging, trace, DWT | [12 - Debug & DWT](../lessons/12-debug-dwt/) |

## Course 4 - Advanced Armv8-M Features

| Course topic | Lesson |
|---|---|
| Synchronization, exclusive access, memory barriers, FPU, DSP | [13 - Sync, FPU & DSP](../lessons/13-sync-fpu-dsp/) |
| SysTick, PendSV | [14 - SysTick & PendSV](../lessons/14-systick-pendsv/) |
| Context switching, scheduler | [15 - Mini Kernel](../lessons/15-mini-kernel/) |
| TrustZone, cache, PACBTI, MVE | [16 - TrustZone & Advanced](../lessons/16-trustzone-advanced/) |

## Where does the repository differ from the course?

- **Every topic is runnable.** A concept explained in the course becomes an
  experiment that runs in QEMU and can be observed in GDB here.
- **Simulator limits are explicit.** The course describes real hardware;
  this repository explains what cannot be observed in QEMU (`docs/LIMITATIONS.md`).
- **The order follows technical dependencies.** For example, the linker/ELF topic
  follows faults because interpreting fault addresses requires a map file.
