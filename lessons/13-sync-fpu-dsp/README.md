# Lesson 13 — Synchronization, FPU & DSP

`[DEEP]` · Day 6 · **STATUS: scaffold**

> Bu ders henüz yazılmadı. Aşağıdaki iskelet neyin kapsanacağını ve hangi
> deneylerin yapılacağını tanımlar. Ders 02 tam kalite referanstır:
> [`lessons/02-programmer-model/`](../02-programmer-model/)

---

## Why This Matters

Kesme ile paylaşılan veriye dokunan her firmware'de race condition riski vardır. FPU ve DSP ise performansın en somut olduğu yer.

---

## Core Idea

Kapsanacak konular:

- Problem önce: kesme ile paylaşılan sayaçta bozulan güncelleme
- Exclusive access (`LDREX`/`STREX`) ve exclusive monitor
- Memory barrier'lar: `DMB`, `DSB`, `ISB` — hangisi ne zaman
- FPU'yu `CPACR` ile açmak, FP register'lar
- Lazy stacking ve genişletilmiş exception frame
- DSP komutlarıyla sentetik sensör verisi işleme

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
cmake --build --preset debug --target 13-sync-fpu-dsp
./scripts/run-qemu.sh build/debug/lessons/13-sync-fpu-dsp/13-sync-fpu-dsp.elf
```

---

## Expected Output

<!-- Firmware gerçekten çalıştırıldıktan SONRA buraya gerçek çıktı yapıştırılacak.
     Çalıştırılmadan örnek çıktı uydurulmayacak. -->

---

## Observe in GDB

Planlanan gözlem komutları:

```text
x/1wx 0xE000ED88
info all-registers
cm-frame <adres> <EXC_RETURN>
x/32wx $sp
```

<!-- Her komut için "neye bakmalısın" açıklaması ders yazılırken eklenecek -->

---

## Experiment

1. Atomik olmayan read-modify-write ile race condition üret
2. Aynı güncellemeyi `LDREX`/`STREX` ile yaz, farkı gözle
3. FPU'yu aç; exception frame'in büyüdüğünü stack'te doğrula
4. Moving average / dot product / basit FIR'i düz C ile yaz, assembly'sine bak

---

## Break It

FPU'yu açmadan FP komutu çalıştır; hangi fault geldiğini ve nedenini çöz.

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

Armv8-M ARM — Exclusive access, Memory ordering, FP extension

---

**Previous:** [12 — Debugging & DWT Profiling](../12-debug-dwt/)
**Next:** [14 — SysTick & PendSV](../14-systick-pendsv/)
