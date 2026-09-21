# Course Map - Coursera Arm Cortex-M Specialization ↔ Lessons

Bu repo bir kurs kopyasi **degildir**. Butun anlatim, kod ve ornekler
ozgundur ve resmi Arm/CMSIS/QEMU dokumantasyonuyla dogrulanir
(`docs/REFERENCES.md`).

Bu tablo yalnizca **hangi dersin hangi kurs konusuna karsilik geldigini**
izlemek icindir. Repoyu kullanmak icin kursa sahip olmak gerekmez.

## Course 1 - Cortex-M Processor Family

| Kurs konusu | Lesson |
|---|---|
| Cortex-M ailesi, islemci secimi | [01 - Cortex-M Family](../lessons/01-cortex-m-family/) |
| Performans / guc / guvenlik trade-off'lari | 01 |

## Course 2 - Armv8-M Architecture Fundamentals

| Kurs konusu | Lesson |
|---|---|
| Programmer model, register'lar, xPSR | [02 - Programmer Model](../lessons/02-programmer-model/) |
| T32/Thumb, assembly temelleri | [03 - C and Assembly](../lessons/03-c-and-assembly/) |
| Stack, full descending, MSP/PSP | [04 - Stack, MSP & PSP](../lessons/04-stack-msp-psp/) |
| Thread/Handler Mode, CONTROL, privilege, SVC | [05 - Privilege & SVC](../lessons/05-privilege-svc/) |
| Memory model, MPU | [06 - MPU](../lessons/06-mpu/) |
| Exceptions, NVIC, oncelik | [07 - Exceptions](../lessons/07-exceptions/) |
| Faults, escalation, fault status register'lari | [08 - Faults](../lessons/08-faults/) |

## Course 3 - Cortex-M Software Development Fundamentals

| Kurs konusu | Lesson |
|---|---|
| Compiler, optimizasyon, AAPCS | [09 - Compiler & AAPCS](../lessons/09-compiler-aapcs/) |
| Object files, ELF, linker, memory map, scatter-loading mantigi | [10 - Linker, ELF & Memory](../lessons/10-linker-elf-memory/) |
| Startup, initialization, CMSIS, reset→main | [11 - Startup](../lessons/11-startup/) |
| Debugging, trace, DWT | [12 - Debug & DWT](../lessons/12-debug-dwt/) |

## Course 4 - Advanced Armv8-M Features

| Kurs konusu | Lesson |
|---|---|
| Synchronization, exclusive access, memory barriers, FPU, DSP | [13 - Sync, FPU & DSP](../lessons/13-sync-fpu-dsp/) |
| SysTick, PendSV | [14 - SysTick & PendSV](../lessons/14-systick-pendsv/) |
| Context switching, scheduler | [15 - Mini Kernel](../lessons/15-mini-kernel/) |
| TrustZone, cache, PACBTI, MVE | [16 - TrustZone & Advanced](../lessons/16-trustzone-advanced/) |

## Repo, kurstan nerelerde ayriliyor?

- **Her konu calistirilabilir.** Kursta anlatilan bir kavram burada QEMU'da
  calisan ve GDB'de gozlemlenebilen bir deneye donusur.
- **Simulator limitleri acikca yazilir.** Kurs gercek donanimi anlatir;
  bu repo neyin QEMU'da gozlemlenemeyecegini soyler (`docs/LIMITATIONS.md`).
- **Sira teknik bagimliliga gore.** Ornegin linker/ELF konusu, faults'tan
  sonra gelir cunku fault adreslerini cozmek icin map file gerekir.
