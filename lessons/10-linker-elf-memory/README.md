# Lesson 10 — Linker, ELF & Memory Layout

`[STANDARD]` · Day 4 · **STATUS: scaffold**

> Bu ders henüz yazılmadı. Aşağıdaki iskelet neyin kapsanacağını ve hangi
> deneylerin yapılacağını tanımlar. Ders 02 tam kalite referanstır:
> [`lessons/02-programmer-model/`](../02-programmer-model/)

---

## Why This Matters

"Firmware flash'a sığmıyor", "değişken neden burada", "`.data` nereden kopyalanıyor" sorularının tek cevap yeri: linker script ve map file.

---

## Core Idea

Kapsanacak konular:

- Derleme zinciri: `.c` → `.o` → linker → `.elf`
- Section'lar: `.text`, `.rodata`, `.data`, `.bss`
- VMA ve LMA farkı — `.data` neden iki adrese sahip
- Linker sembolleri (`__data_start`, `__bss_end`) ve nasıl kullanıldığı
- Map file okuma
- Arm Compiler'ın scatter-loading'i ile GNU linker script karşılaştırması

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
cmake --build --preset debug --target 10-linker-elf-memory
./scripts/run-qemu.sh build/debug/lessons/10-linker-elf-memory/10-linker-elf-memory.elf
```

---

## Expected Output

<!-- Firmware gerçekten çalıştırıldıktan SONRA buraya gerçek çıktı yapıştırılacak.
     Çalıştırılmadan örnek çıktı uydurulmayacak. -->

---

## Observe in GDB

Planlanan gözlem komutları:

```text
p/x &my_global
info symbol <adres>
x/4wx &my_global
```

<!-- Her komut için "neye bakmalısın" açıklaması ders yazılırken eklenecek -->

---

## Experiment

1. Global, static, const ve yerel değişkenler tanımla; her birinin hangi section'a gittiğini bul
2. `arm-none-eabi-nm -n` çıktısında kendi sembolünü bul
3. Map file'da bir sembolü arayıp adresini doğrula
4. Linker script'te FLASH boyutunu küçültüp overflow hatasını gör

---

## Break It

`.data` kopyalamasını startup'tan kaldır; initialized global'lerin ne olduğunu gözle.

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

GNU ld dokümantasyonu; `docs/MEMORY_MAP.md`

---

**Previous:** [09 — Compiler, Optimization & AAPCS](../09-compiler-aapcs/)
**Next:** [11 — Startup — Reset to main](../11-startup/)
