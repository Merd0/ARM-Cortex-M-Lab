# Lesson 12 — Debugging & DWT Profiling

`[STANDARD]` · Day 5 · **STATUS: scaffold**

> Bu ders henüz yazılmadı. Aşağıdaki iskelet neyin kapsanacağını ve hangi
> deneylerin yapılacağını tanımlar. Ders 02 tam kalite referanstır:
> [`lessons/02-programmer-model/`](../02-programmer-model/)

---

## Why This Matters

Debugger kullanmak komut ezberlemek değil, bir teşhis yöntemi izlemektir. Ayrıca bu ders simülatörün gerçek sınırını gösteriyor.

---

## Core Idea

Kapsanacak konular:

- Breakpoint, watchpoint, single step, disassembly
- Stack'ten çağrı zincirini elle çıkarma
- DWT birimi ve CYCCNT
- **Bu target'ta CYCCNT saymıyor** — ölçüldü, `docs/TARGET_CAPABILITIES.md`
- Ölçüm yapılabilen alternatifler: kod boyutu, komut sayısı, SysTick
- Ölçüm hatası kaynakları: console I/O, optimizasyon, tekrarlanabilirlik

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
cmake --build --preset debug --target 12-debug-dwt
./scripts/run-qemu.sh build/debug/lessons/12-debug-dwt/12-debug-dwt.elf
```

---

## Expected Output

<!-- Firmware gerçekten çalıştırıldıktan SONRA buraya gerçek çıktı yapıştırılacak.
     Çalıştırılmadan örnek çıktı uydurulmayacak. -->

---

## Observe in GDB

Planlanan gözlem komutları:

```text
watch <var>
rwatch
x/1wx 0xE0001004
x/1wx 0xE000EDFC
disassemble /s
```

<!-- Her komut için "neye bakmalısın" açıklaması ders yazılırken eklenecek -->

---

## Experiment

1. Bir değişkene watchpoint koyup değiştiren kodu bul
2. DWT'yi etkinleştirip CYCCNT'in gerçekten saymadığını kendin doğrula
3. Aynı fonksiyonu `-O0` ve `-O2` ile derleyip boyut/komut sayısı karşılaştır
4. SysTick tabanlı göreceli ölçüm yap ve neden mutlak olmadığını yaz

---

## Break It

Ölçüm bölgesinin içine bir `console_puts()` koy; sonucun nasıl anlamsızlaştığını gör.

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

Armv8-M ARM — DWT; `docs/PERFORMANCE_MEASUREMENT.md`, `docs/LIMITATIONS.md`

---

**Previous:** [11 — Startup — Reset to main](../11-startup/)
**Next:** [13 — Synchronization, FPU & DSP](../13-sync-fpu-dsp/)
