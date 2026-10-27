**Dil:** [English](README.md) | Türkçe

# Exercise 01 — Repair an ABI-Broken Assembly Function

**Hedef:** Doğru sonuç üreten ama AAPCS sözleşmesini bozan bir assembly
fonksiyonunu onarmak.

## Görev

[`scale_and_sum.S`](scale_and_sum.S):

```c
uint32_t scale_and_sum(uint32_t a, uint32_t b);   /* helper_scale(a) + b */
```

`helper_scale` bir C fonksiyonu (`example/helpers.c`). `scale_and_sum` onu
çağırıyor, yani **yaprak değil**.

Fonksiyon **doğru sonucu** döndürüyor. Ama harness iki ihlal buluyor:

| Kontrol | Başlangıç |
|---|---|
| `CHECK_RESULT` | PASS |
| `CHECK_CALLEE_SAVED` | **FAIL** — harness bir register'ın bozulduğunu buldu |
| `CHECK_SP_ALIGNED_AT_CALL` | **FAIL** — `BL` anında SP 8'in katı değil |

**Yalnızca `scale_and_sum.S`'i düzenle.** `main.c` ve harness'e dokunma.
`BL`'den hemen önceki SP kaydını (`g_ex_sp_at_call`) silme; kontrol onu
kullanıyor.

## Build & Run

```bash
cmake --build --preset debug --target 09-exercise-callee-save
./scripts/run-qemu.sh build/debug/lessons/09-compiler-aapcs/09-exercise-callee-save.elf
```

Başlangıç çıktısı (gerçek, `debug`):

```text
[  CHECK_HARNESS_POSITIVE_CONTROL] PASS
[  RESULT] 36
[  MASK] 0x00000002
[  CORRUPTED] r5
[  SP_AT_BL] 0x30007FAC
[  CHECK_RESULT] PASS
[  CHECK_CALLEE_SAVED] FAIL
[  CHECK_SP_ALIGNED_AT_CALL] FAIL
===== END =====
```

Bu **beklenen başlangıç durumudur**. `release` ve `minsize`'da adresler farklı
ama kararlar aynı.

## Doğrulama

Üç kontrolün üçü de `PASS`, `RESULT` hâlâ `36`.

## Cevaplaman gereken sorular

1. `b` çağrı boyunca hangi register'da tutuluyor? AAPCS'e göre o register
   kimin? Kim onu korumakla yükümlü?
2. `SP_AT_BL` neden 8'in katı değil? Fonksiyonun girişinde SP 8'e hizalıydı
   (harness bunu garanti ediyor). Girişten `BL`'ye kadar kaç byte yığıldı?
3. Bir `push` listesine eklediğin her register 4 byte'tır. Toplam 8'in katı
   olmalıysa ve korumak istediğin register sayısı tekse ne yaparsın?
   (İpucu: harness'in kendi `push` listesine bak.)
4. Fonksiyon neden LR'yi saklıyor? Saklamasaydı `helper_scale`'den sonra ne
   olurdu?
5. Dönüş neden `bx lr` değil de `pop {..., pc}` ile yapılıyor? İkisi de
   doğru mu?

Sorular 4 ve 5 çalışma zamanında test edilmiyor — yanlış bir dönüş bir çökme
veya sonsuz döngü üretirdi, kontrollü bir FAIL değil. Onları disassembly'ye
bakarak cevapla.

## Takıldıysan

Referans çözüm: [`../../solutions/01-callee-save/`](../../solutions/01-callee-save/README.tr.md)

---

[← Ders 09](../../README.tr.md)
