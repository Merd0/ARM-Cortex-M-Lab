# Project — From Reset to Main

**STATUS: Debug temel kapısı PASS; son toplu kalite kapısı bekliyor.**

Lesson 10–11'in linker/startup ilişkisini gerçek bir initialized calibration
dizisi ve sıfırlanan uygulama buffer'ı üzerinde birleştirir. Çalışan reset,
vector ve RAM poisoning fixture'ı Lesson 11 referansından **aynı kaynak dosyalarla**
derlenir. Proje [main](src/main.c) ve [linker girişini](layout.ld) ekler;
audited platform yerleşimi INCLUDE edilir. Startup kodunun ikinci kopyası tutulmaz.

```bash
cmake --preset debug
cmake --build --preset debug --target project-reset
./scripts/run-qemu.sh build/debug/projects/from-reset-to-main/project-reset.elf
python tests/system-smoke.py
```

Reset önce fixture ile .data/.bss VMA'sını zehirler, sonra FLASH'tan .data'yı
kopyalar, .bss'yi sıfırlar, SystemInit ve main'i çağırır. Böylece QEMU'nun
başlangıçta sıfır RAM sağlaması eksik startup'ı gizlemez. Bu poisoning bir test
aracıdır, üretim boot politikası değildir.

Debug/QEMU: CHECK_DATA, BSS, ORDER, VECTORS, VTOR ve LMA PASS.
Calibration 11/22/33; buffer'ın 12 word'ü sıfır; phase=0x123.
Vector[0] stack top, vector[1] Thumb Reset_Handler; reserved 8/9/10/13 sıfır.
.data LMA'sı FLASH, VMA'sı RAM'dir; AT>FLASH ve overflow ASSERT'leri korunur.

GDB toplu kapısında Reset_Handler → main ve LMA/VMA kopyası izlenecek.
Öğrenci değişiklik deneyi: Lesson 11'in ayrı starter/solution hedeflerini kullan;
projenin referans reset'ini öğrenci TODO'su ile değiştirme.

[Lesson 11](../../lessons/11-startup/) · [Projects](../README.md)
