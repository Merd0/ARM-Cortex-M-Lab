# Exercise 01 — `decode_apsr_flags()`

**Hedef:** ham `APSR` değerini okunabilir bayrak satırına çevirmek.

## Görev

`main.c` içindeki `decode_apsr_flags()` fonksiyonunu doldur. Üretmesi gereken
satır:

```text
[FLAGS] N=0 Z=1 C=1 V=0
```

Bit konumları (Armv8-M ARM, "APSR"):

| Bit | Bayrak | Anlamı |
|---|---|---|
| 31 | N | Negative — sonuç negatif |
| 30 | Z | Zero — sonuç sıfır |
| 29 | C | Carry — çıkarmada "borrow yok" |
| 28 | V | Overflow — işaretli taşma |

## Build & Run

```bash
cmake --build --preset debug --target 02-exercise-apsr
./scripts/run-qemu.sh build/debug/lessons/02-programmer-model/02-exercise-apsr.elf
```

## Doğrulama

| İşlem | Beklenen |
|---|---|
| `5 - 5` | `Z = 1` |
| `5 - 7` | `N = 1` |

`C` ve `V` için beklentini **çalıştırmadan önce** bir yere yaz. Tahminin
tutmazsa, asıl öğrenme orada.

## İpucu

`console_puts()` ve `console_putc()` yeterli. `'0' + bit` ifadesi bir biti
doğrudan karaktere çevirir. Yeni yardımcı fonksiyon gerekmez.

## Takıldıysan

Referans çözüm: [`../../solutions/01-decode-apsr-flags/`](../../solutions/01-decode-apsr-flags/)

---

[← Ders 02](../../README.md)
