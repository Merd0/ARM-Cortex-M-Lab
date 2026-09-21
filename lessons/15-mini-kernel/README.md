# Lesson 15 — Educational Mini Kernel

`[DEEP]` · Day 7 · **STATUS: scaffold**

> Bu ders henüz yazılmadı. Aşağıdaki iskelet neyin kapsanacağını ve hangi
> deneylerin yapılacağını tanımlar. Ders 02 tam kalite referanstır:
> [`lessons/02-programmer-model/`](../02-programmer-model/)

---

## Why This Matters

Context switch'i kendi elinle yazmak, RTOS'un altında ne olduğunu kalıcı olarak öğrenmenin tek yolu.

---

## Core Idea

Kapsanacak konular:

- Task kontrol bloğu ve ayrı task stack'leri
- Task stack'ini ilk çalıştırma için elle hazırlamak (sahte exception frame)
- PendSV içinde R4–R11'i kaydetme/geri yükleme
- PSP'nin task'lar, MSP'nin kernel için kullanılması
- Round-robin scheduling
- SVC ile sistem çağrıları
- Uzantı: privileged kernel + unprivileged task + MPU izolasyonu

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
cmake --build --preset debug --target 15-mini-kernel
./scripts/run-qemu.sh build/debug/lessons/15-mini-kernel/15-mini-kernel.elf
```

---

## Expected Output

<!-- Firmware gerçekten çalıştırıldıktan SONRA buraya gerçek çıktı yapıştırılacak.
     Çalıştırılmadan örnek çıktı uydurulmayacak. -->

---

## Observe in GDB

Planlanan gözlem komutları:

```text
p/x $psp
p/x $msp
x/32wx <task_stack>
break PendSV_Handler
cm-frame <adres> <EXC_RETURN>
```

<!-- Her komut için "neye bakmalısın" açıklaması ders yazılırken eklenecek -->

---

## Experiment

1. `TODO(student)`: task stack hazırlama fonksiyonunu yaz
2. `TODO(student)`: PendSV handler'ını assembly ile yaz
3. İki task çalıştır, sırayla mesaj bastıklarını gör
4. Debugger'da her task'ın kendi stack'ini incele

---

## Break It

Context switch'te R4–R11'i kaydetmeyi atla; bozulmanın hangi task'ta ortaya çıktığını bul.

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

Armv8-M ARM — Exception entry/return, EXC_RETURN; `projects/mini-kernel/`

---

**Previous:** [14 — SysTick & PendSV](../14-systick-pendsv/)
**Next:** [16 — TrustZone & Advanced Features](../16-trustzone-advanced/)
