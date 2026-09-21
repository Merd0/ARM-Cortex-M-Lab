# Lesson 11 — Startup — Reset to main

`[STANDARD]` · Day 5 · **STATUS: scaffold**

> Bu ders henüz yazılmadı. Aşağıdaki iskelet neyin kapsanacağını ve hangi
> deneylerin yapılacağını tanımlar. Ders 02 tam kalite referanstır:
> [`lessons/02-programmer-model/`](../02-programmer-model/)

---

## Why This Matters

`main`'e gelmeden önce çalışan kodu anlamadan gerçek bir bring-up yapamazsın. Board açılmıyorsa hata büyük ihtimalle buradadır.

---

## Core Idea

Kapsanacak konular:

- Reset'te donanımın yaptığı: vector[0] → MSP, vector[1] → PC
- Vector table'ın yerleşimi ve VTOR
- `.data` kopyalama ve `.bss` sıfırlama döngüleri
- `SystemInit` ve C runtime'ın başlatılması
- `main`'e geçiş
- Thumb bitinin reset'teki kritik rolü

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
cmake --build --preset debug --target 11-startup
./scripts/run-qemu.sh build/debug/lessons/11-startup/11-startup.elf
```

---

## Expected Output

<!-- Firmware gerçekten çalıştırıldıktan SONRA buraya gerçek çıktı yapıştırılacak.
     Çalıştırılmadan örnek çıktı uydurulmayacak. -->

---

## Observe in GDB

Planlanan gözlem komutları:

```text
cm-vectors
stepi
p/x $msp
x/16wx &__bss_start
break main
```

<!-- Her komut için "neye bakmalısın" açıklaması ders yazılırken eklenecek -->

---

## Experiment

1. `TODO(student)`: kendi `Reset_Handler`'ını ve vector table'ını sıfırdan yaz
2. Kendi startup'ınla platform startup'ını karşılaştır
3. Reset'ten `main`'e kadar sadece `stepi` ile git
4. `.bss` sıfırlanmadan önce ve sonra belleği oku

---

## Break It

`.bss` sıfırlamayı atla veya `.data` kopyalamasında yanlış uzunluk kullan.

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

Armv8-M ARM — Reset behaviour; `platform/mps2-an505/src/startup_mps2_an505.c`

---

**Previous:** [10 — Linker, ELF & Memory Layout](../10-linker-elf-memory/)
**Next:** [12 — Debugging & DWT Profiling](../12-debug-dwt/)
