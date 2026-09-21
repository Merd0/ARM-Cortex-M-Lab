# Solution 01 — `decode_apsr_flags()`

Referans çözüm. Exercise: [`../../exercises/01-decode-apsr-flags/`](../../exercises/01-decode-apsr-flags/)

## Build & Run

```bash
cmake --build --preset debug --target 02-solution-apsr
./scripts/run-qemu.sh build/debug/lessons/02-programmer-model/02-solution-apsr.elf
```

## Gerçek çıktı

```text
--- xpsr flags ---
[APSR_AFTER_5_MINUS_5] 0x60000000
[FLAGS] N=0 Z=1 C=1 V=0
[APSR_AFTER_5_MINUS_7] 0x80000000
[FLAGS] N=1 Z=0 C=0 V=0
```

## Çıktının okunması

`5 - 5 = 0` → `Z = 1` beklenen. Ama `C = 1` de çıkıyor ve bu çoğu kişiyi
şaşırtır: Arm'da çıkarma işleminde `C`, "carry" değil **"borrow olmadı"**
anlamına gelir. `5 >= 5` olduğu için borrow yok, `C = 1`.

`5 - 7 = -2` → `N = 1` beklenen. `C = 0`, çünkü bu kez borrow gerekti
(`5 < 7`). `V = 0`, çünkü işaretli taşma yok — `-2`, 32 bit işaretli aralığa
rahatça sığıyor.

`V` ile `C`'yi karıştırmak yaygın bir hatadır: `C` işaretsiz (unsigned)
karşılaştırma için, `V` işaretli (signed) karşılaştırma için anlamlıdır.
`BHI`/`BLS` gibi dallanmalar `C`'ye, `BGT`/`BLT` gibi olanlar `N` ve `V`'ye
bakar.

## Tasarım notu

Bayraklar bit kaydırma + maskeleme ile çıkarıldı, C bitfield'ı ile değil.
Sebep: C standardı bitfield'ların bellekteki yerleşimini garanti etmez
(implementation-defined). Donanım register'ı okurken taşınabilir olan tek
yöntem maske + kaydırmadır.

---

[← Ders 02](../../README.md)
