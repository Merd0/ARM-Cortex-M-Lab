# Lesson 16 — TrustZone & Advanced Features

`[DEEP]` · Day 7 · **STATUS: scaffold**

> Bu ders henüz yazılmadı. Aşağıdaki iskelet neyin kapsanacağını ve hangi
> deneylerin yapılacağını tanımlar. Ders 02 tam kalite referanstır:
> [`lessons/02-programmer-model/`](../02-programmer-model/)

---

## Why This Matters

TrustZone, güvenliği yazılım disiplininden donanım sınırına taşır. Cache/PACBTI/MVE ise hangi donanımda çalıştığının neden önemli olduğunu gösterir.

---

## Core Idea

Kapsanacak konular:

- Secure / Non-secure state ayrımı
- SAU ile adres uzayının bölünmesi
- NSC (non-secure callable) bölgeler ve secure gateway mantığı
- Non-secure koddan secure belleğe erişim denemesinin sonucu
- Secure API tasarımı: ne açılır, ne açılmaz
- Cache / PACBTI / MVE: genel bakış + target desteği doğrulaması

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
cmake --build --preset debug --target 16-trustzone-advanced
./scripts/run-qemu.sh build/debug/lessons/16-trustzone-advanced/16-trustzone-advanced.elf
```

---

## Expected Output

<!-- Firmware gerçekten çalıştırıldıktan SONRA buraya gerçek çıktı yapıştırılacak.
     Çalıştırılmadan örnek çıktı uydurulmayacak. -->

---

## Observe in GDB

Planlanan gözlem komutları:

```text
x/1wx 0xE000EDD0
x/1wx 0xE000EDD4
cm-fault
cm-mode
```

<!-- Her komut için "neye bakmalısın" açıklaması ders yazılırken eklenecek -->

---

## Experiment

1. SAU ile secure ve non-secure bölgeler tanımla
2. Non-secure'dan secure belleğe erişmeyi dene, SecureFault'u incele
3. Kontrollü bir secure servis çağrısı yaz
4. Cache/PACBTI/MVE için önce `capability-probe` ile desteği ölç; desteklenmiyorsa `NOT SUPPORTED ON CURRENT TARGET` olarak kaydet

---

## Break It

Secure API'den non-secure belleğe işaretçi döndür; neden tehlikeli olduğunu açıkla.

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

Armv8-M ARM — Security Extension, SAU; `docs/TARGET_CAPABILITIES.md`

---

**Previous:** [15 — Educational Mini Kernel](../15-mini-kernel/)
**Next:** [Final Project](../../final-project/)
