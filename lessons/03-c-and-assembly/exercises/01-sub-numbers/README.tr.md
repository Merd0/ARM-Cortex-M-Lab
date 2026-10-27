**Dil:** [English](README.md) | Türkçe

# Exercise 01 — `sub_numbers`

**Hedef:** AAPCS sözleşmesini kendi elinle uygulamak.

## Görev

`sub_numbers.S` içindeki fonksiyonu doldur:

```c
int sub_numbers(int a, int b);    /* a - b */
```

Şu an sabit `0` dönüyor. Cevaplaman gerekenler:

- Argümanlar hangi register'lardan geliyor?
- Sonucu nereye koymalısın?
- Dönüşü nasıl yapıyorsun?

## Build & Run

```bash
cmake --build --preset debug --target 03-exercise-sub
./scripts/run-qemu.sh build/debug/lessons/03-c-and-assembly/03-exercise-sub.elf
```

## Doğrulama

| Test | Beklenen |
|---|---|
| `sub_numbers(12, 5)` | `7` |
| `sub_numbers(5, 12)` | `-7` |

İki `CHECK` satırı da `PASS` olmalı. İkinci testte konsol işaretsiz basıyor —
ne göreceğini **çalıştırmadan önce tahmin et**.

## İpucu

Tek aritmetik komut + tek dönüş komutu yeterli. Çıkarma komutunun adı için
Armv8-M ARM'da "SUB (register)" başlığına bak.

Kodunu yazdıktan sonra ne ürettiğine bak:

```bash
arm-none-eabi-objdump -d build/debug/lessons/03-c-and-assembly/03-exercise-sub.elf \
    | grep -A2 "<sub_numbers>:"
```

## Takıldıysan

Referans çözüm: [`../../solutions/01-sub-numbers/`](../../solutions/01-sub-numbers/README.tr.md)

---

[← Ders 03](../../README.tr.md)
