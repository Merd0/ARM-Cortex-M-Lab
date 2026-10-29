**Dil:** [English](README.md) | Türkçe

# Exercise 01 — Locate Your Data

**Hedef:** Map dosyasına bakmadan, yalnızca linker sembollerinden ve nesne
adreslerinden bellek yerleşimini hesaplamak.

## Görev

[`locate.c`](locate.c) içindeki dört fonksiyonu doldur:

| Fonksiyon | Ne döndürmeli |
|---|---|
| `flash_used_bytes()` | FLASH'taki yük görüntüsünün **tamamı** (byte) |
| `ram_static_bytes()` | RAM'de **statik** olarak kullanılan byte — stack hariç |
| `data_lma_of(p)` | `p` `.data` içinde bir nesne; onun **ilk değerinin** FLASH adresi |
| `in_region(p, origin, length)` | `p`, `[origin, origin + length)` içinde mi |

Kullanabileceğin semboller `example/l10.h`'te bildirili; tanımları
`platform/mps2-an505/linker/mps2-an505-secure.ld`'de.

**Kural:** map dosyasından sayı kopyalama. Kodun başka bir build'de de doğru
kalmalı — referans çözümün sayıları ana örnekten zaten farklı.

## Build & Run

```bash
cmake --build --preset debug --target 10-exercise-linker
./scripts/run-qemu.sh build/debug/lessons/10-linker-elf-memory/10-exercise-linker.elf
```

Başlangıç çıktısı (gerçek):

```text
[  FLASH_USED_BYTES] 0
[  RAM_STATIC_BYTES] 0
[  G_INIT_LMA] 0x00000000
[  CHECK_FLASH_USED] FAIL
[  CHECK_RAM_STATIC] FAIL
[  CHECK_LMA_HOLDS_INIT] FAIL
[  CHECK_REGIONS] FAIL
===== END =====
```

Bu **beklenen başlangıç durumudur**.

## Doğrulama

Dört kontrol de `PASS`.

Kontroller sabit adreslere dayanmıyor:

- `CHECK_FLASH_USED` senin sonucunu linker'ın kendi hesapladığı bir sembolle
  karşılaştırır.
- `CHECK_LMA_HOLDS_INIT` döndürdüğün adresi **okur** ve orada `g_init`'in ilk
  değeri (`0xC0DEFACE`) var mı diye bakar — ama yalnızca adres FLASH yük
  görüntüsünün içindeyse, bitişe en az 4 byte uzaklıktaysa ve 4 byte'a
  hizalıysa. Aksi halde **okumaz**. Kontrol 32 bit taşmaya karşı güvenli
  yazıldı (`0xFFFFFFFC` gibi değerler dahil): yanlış bir cevap fault değil
  FAIL üretir.

## Cevaplaman gereken sorular

1. FLASH'ta hangi byte'lar var? Yalnızca kod mu?
2. `.data` FLASH'ta da yer kaplar mı? Neden?
3. `.bss` FLASH'ta yer kaplar mı?
4. `__etext` adı neyi ima ediyor, tanımı gerçekte ne? (Linker script'e bak.)
5. Bir nesnenin RAM'deki `.data` içindeki konumu ile FLASH'taki kopyasındaki
   konumu arasında nasıl bir ilişki var?
6. Stack neden `ram_static_bytes()`'a dahil değil?

## Takıldıysan

Referans çözüm: [`../../solutions/01-locate-your-data/`](../../solutions/01-locate-your-data/README.tr.md)

---

[← Ders 10](../../README.tr.md)
