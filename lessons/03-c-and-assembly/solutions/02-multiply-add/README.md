# Solution 02 — `multiply_add` (Deney 1'in referans çözümü)

```c
int multiply_add(int a, int b, int c);    /* a * b + c */
```

## Build & Run

```bash
cmake --build --preset debug --target 03-solution-mla
./scripts/run-qemu.sh build/debug/lessons/03-c-and-assembly/03-solution-mla.elf
```

## Gerçek çıktı

```text
===== LESSON 03 SOLUTION 02 =====
[MULTIPLY_ADD(3,4,5)] 17
[MULTIPLY_ADD(7,7,1)] 50
[CHECK_3x4p5] PASS
===== END =====
```

## Üretilen kod (ölçüldü)

```text
10000040 <multiply_add>:
10000040:  fb00 2001   mla  r0, r0, r1, r2
10000044:  4770        bx   lr
```

Cevaplanan soru: **üçüncü argüman `r2`'ye gelir.** AAPCS ilk dört argümanı
sırayla `r0`, `r1`, `r2`, `r3`'e yerleştirir. `mla r0, r0, r1, r2` bunu tek
komutta kullanıyor: `r0 = r0 * r1 + r2`.

## İki farklı yol

| Yol | Komutlar | Boyut | Mimari |
|---|---|---|---|
| `muls` + `adds` | 2 komut | 2 + 2 byte | Armv6-M dahil her yerde |
| `mla` | 1 komut | 4 byte | Armv7-M ve sonrası |

Burada bilerek `MLA` seçildi, çünkü ders 01'deki soruyu somutlaştırıyor:
`MLA` Cortex-M33'te vardır ama Cortex-M0/M0+ (Armv6-M) üzerinde **yoktur**.
Aynı C kodunu M0+ için derlersen derleyici iki komutlu yolu üretmek zorunda
kalır.

Dikkat çekici bir ayrıntı: `mla` **32-bit** bir kodlama (`fb00 2001`), `adds`
ise 16-bit (`1840`). Yani "tek komut" her zaman "daha az byte" demek değil.
Burada 1 komut = 4 byte, 2 komut = 4 byte — boyut aynı, fark yürütme
tarafında.

`MLA` bayrakları güncellemez ve `s` sonekli bir sürümü yoktur.

---

[← Ders 03](../../README.md)
