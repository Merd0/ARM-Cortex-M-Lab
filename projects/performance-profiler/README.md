# Project — Performance Profiler

**STATUS: main üzerinde; Debug/Release/MinSize CI PASS (11fde900)**

Lesson 12'nin runtime-input workload'u ve referans ölçüm sınıflandırıcısı aynı
kaynaklardan kullanılır. Proje beş farklı workload object'i üretir:
-O0, -O1, -O2, -O3, -Os. Sadece workload bayrağı değişir; preset'in genel
Release=-O2 sözleşmesi değişmez. Driver her varyantta checksum=424 doğrular.

```bash
cmake --preset debug
cmake --build --preset debug
python tests/system-smoke.py
python tests/profile-report.py
```

Debug/GCC 15.2.1 ölçümü: workload boyutları sırasıyla 70,32,32,32,28 byte;
statik komut sayıları 35,13,13,13,11. Script gerçek nm/objdump çıktısından
`build/debug/system-evidence/profile.json` üretir. Bunlar yürütülen komut
sayısı, cycle, hızlanma veya WCET değildir; toolchain ile değişebilir.

QEMU 11.1.0 AN505'te DWT ilerlemedi; DWT_RESULT=UNAVAILABLE ölçüldü.
Model-local dört SysTick boyunca ilerleyen iş sayılır; ölçüm penceresinde
console I/O yoktur. İterasyonlardan fiziksel throughput veya güvenilir
optimizasyon sıralaması çıkarılmaz. Daha küçük kod daha hızlı demek değildir.
FVP de yapılandırmasına göre timing modeline sahiptir; her FVP cycle-accurate
kabul edilmez. Donanım performansı ayrı ölçüm gerektirir.

Negatif ölçüm sınıflandırması ve öğrenci starter/solution
[Lesson 12](../../lessons/12-debug-dwt/) içinde korunur. Ayrıntılı GDB ve
üç-preset karşılaştırması toplu kalite kapısındadır.

[Projects](../README.md)
