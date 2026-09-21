# Roadmap

## Faz 0 - Bootstrap (TAMAMLANDI)

- Repository iskeleti, CMake + Ninja + Arm GNU Toolchain build sistemi
- MPS2-AN505 (Cortex-M33) platform portu: startup, linker script, UART
- QEMU'da calisan firmware + GDB baglantisi dogrulandi
- Target capability probe ve olculmus capability matrisi
- Ders 02 (Programmer Model) tam kalite referans ders olarak hazir

## Faz 1 - FAST TRACK (~1 hafta, ana ogrenme yolu)

Gun gun plan: [`BOOTCAMP.md`](../BOOTCAMP.md)

| Gun | Konu | Lessons |
|---|---|---|
| 1 | Temeller ve programmer model | 01, 02 |
| 2 | Assembly, fonksiyon cagrisi, stack | 03, 04 |
| 3 | Privilege, SVC, MPU | 05, 06 |
| 4 | Exceptions, faults, build pipeline | 07, 08, 09, 10 |
| 5 | Startup ve debugging | 11, 12 |
| 6 | Senkronizasyon, FPU/DSP, SysTick | 13, 14 |
| 7 | Context switching, mini kernel, TrustZone | 15, 16 |

FAST TRACK sonunda elde olmasi gereken: calisan bir mini kernel,
kullanilabilir bir fault analyzer ve her konuda kendi gozlemlerin.

## Faz 2 - DEEP TRACK (opsiyonel, sonradan)

Her ders kendi DEEP TRACK deneylerini icerir. Ayrica:

- **M55 platform portu** - `mps3-an547` icin ikinci platform; MVE/Helium
  capability'sini gercekten olcmek (su an `OLCULMEDI`)
- **Arm FVP entegrasyonu** - QEMU'da gozlemlenemeyen davranislar icin
- **Fault analyzer'i tam surume cikarma** - tum CFSR bitlerinin decode'u
- **MPU ile task isolation** - mini kernel uzerine
- **PACBTI ve cache** - target destegi dogrulandiktan sonra

## Faz 3 - Specialization sonrasi

Bu repoya bagimli olmayan ayri bir faz:

```text
FreeRTOS veya Zephyr  ->  micro-ROS  ->  ROS 2
```

Mini kernel egitim amaclidir; uretim RTOS'u degildir. Gercek projede
olgunlasmis bir RTOS kullanilir. Bu repodaki bilgi o noktada
"RTOS'un altinda ne oluyor" sorusunun cevabi olur.

## Gercek donanim

Board alindiginda: `docs/REAL_HARDWARE_MIGRATION.md`
