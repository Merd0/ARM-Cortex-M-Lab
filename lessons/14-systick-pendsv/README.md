# Lesson 14 — SysTick & PendSV

`[STANDARD]` · Day 6 · **STATUS: scaffold**

> Bu ders henüz yazılmadı. Aşağıdaki iskelet neyin kapsanacağını ve hangi
> deneylerin yapılacağını tanımlar. Ders 02 tam kalite referanstır:
> [`lessons/02-programmer-model/`](../02-programmer-model/)

---

## Why This Matters

Her gerçek zamanlı sistemin iki yapı taşı: periyodik bir tick ve düşük öncelikli bir anahtarlama noktası.

---

## Core Idea

Kapsanacak konular:

- SysTick'i 1 ms periyotla kurmak
- Timer ile scheduler'ın aynı şey olmadığını görmek
- PendSV'nin neden en düşük önceliğe ayarlandığı
- `ICSR.PENDSVSET` ile PendSV tetiklemek
- Tick handler'ın kısa, PendSV'nin ağır işi yaptığı model

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
cmake --build --preset debug --target 14-systick-pendsv
./scripts/run-qemu.sh build/debug/lessons/14-systick-pendsv/14-systick-pendsv.elf
```

---

## Expected Output

<!-- Firmware gerçekten çalıştırıldıktan SONRA buraya gerçek çıktı yapıştırılacak.
     Çalıştırılmadan örnek çıktı uydurulmayacak. -->

---

## Observe in GDB

Planlanan gözlem komutları:

```text
x/1wx 0xE000E010
x/1wx 0xE000ED04
break SysTick_Handler
break PendSV_Handler
cm-mode
```

<!-- Her komut için "neye bakmalısın" açıklaması ders yazılırken eklenecek -->

---

## Experiment

1. 1 ms tick sayacı kur, saniyede bir mesaj bas
2. SysTick içinden PendSV'yi pending yap, çalışma sırasını gözle
3. PendSV önceliğini yükseltip sıranın nasıl bozulduğunu gör

---

## Break It

PendSV'ye SysTick'ten yüksek öncelik ver ve ortaya çıkan davranışı açıkla.

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

Armv8-M ARM — SysTick, PendSV, exception priorities

---

**Previous:** [13 — Synchronization, FPU & DSP](../13-sync-fpu-dsp/)
**Next:** [15 — Educational Mini Kernel](../15-mini-kernel/)
