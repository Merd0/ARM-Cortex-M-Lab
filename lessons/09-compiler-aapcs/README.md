# Lesson 09 — Compiler, Optimization & AAPCS

`[STANDARD]` · Day 4 · **STATUS: scaffold**

> Bu ders henüz yazılmadı. Aşağıdaki iskelet neyin kapsanacağını ve hangi
> deneylerin yapılacağını tanımlar. Ders 02 tam kalite referanstır:
> [`lessons/02-programmer-model/`](../02-programmer-model/)

---

## Why This Matters

Optimizasyon açıkken kaybolan değişkenler, silinen döngüler ve bozulan `volatile` varsayımları embedded'da en çok zaman yakan hata sınıfıdır.

---

## Core Idea

Kapsanacak konular:

- Aynı fonksiyonun `-O0`/`-O1`/`-O2`/`-O3`/`-Os` altındaki assembly'si
- Dead code elimination, register allocation, inlining
- `volatile`'ın gerçekte ne garanti ettiği (ve etmediği)
- AAPCS: R0–R3 argüman, R0 dönüş, R4–R11 callee-saved
- 64-bit argümanlar ve register çiftleri
- Stack alignment kuralı

---

## Mental Model

<!-- Ders yazılırken doldurulacak: konuyu oturtan basit benzetme -->

---

## Visual

<!-- Mermaid diyagram: ders yazılırken eklenecek -->

---

## Tiny Example

<!-- İzole, minimum çalışan örnek: example/main.c  (exercise -> exercises/, çözüm -> solutions/) -->

---

## Build & Run

```bash
cmake --preset debug
cmake --build --preset debug --target 09-compiler-aapcs
./scripts/run-qemu.sh build/debug/lessons/09-compiler-aapcs/09-compiler-aapcs.elf
```

---

## Expected Output

<!-- Firmware gerçekten çalıştırıldıktan SONRA buraya gerçek çıktı yapıştırılacak.
     Çalıştırılmadan örnek çıktı uydurulmayacak. -->

---

## Observe in GDB

Planlanan gözlem komutları:

```text
disassemble /s
info registers r4 r5 r6 r7
stepi
```

<!-- Her komut için "neye bakmalısın" açıklaması ders yazılırken eklenecek -->

---

## Experiment

1. Aynı kaynağı farklı optimizasyon seviyeleriyle build edip boyut ve komut sayısını karşılaştır
2. `volatile` olmayan bir donanım register okumasının nasıl silindiğini gör
3. Assembly fonksiyonunda R4–R11 kullanıp kaydetmeyerek AAPCS'i ihlal et

---

## Break It

Callee-saved bir register'ı kaydetmeden kullan; hata hemen değil, çağıran fonksiyonda ortaya çıkacak.

---

## Debug It

<!-- Doğrudan cevap verilmeyecek; teşhis noktaları sırayla gösterilecek -->

---

## What I Learned

> Bu bölüm senin. Deneyi yaptıktan sonra doldur.

**Beni şaşırtan şey:**

**Yaptığım hata:**

**Debugger'da gördüğüm davranış:**

---

## Quick Check

<!-- 3–5 soru, en az biri reasoning gerektiren -->

---

## Takeaway

<!-- 3–5 madde -->

---

## References

AAPCS; GCC optimizasyon dokümantasyonu; `docs/PERFORMANCE_MEASUREMENT.md`

---

**Previous:** [08 — Faults & Fault Analyzer](../08-faults/)
**Next:** [10 — Linker, ELF & Memory Layout](../10-linker-elf-memory/)
