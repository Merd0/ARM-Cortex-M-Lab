# Exercise 01 — Complete the Reset Sequence

**Hedef:** Reset'ten `main`'e giden yolda C'nin varsaydığı dünyayı kurmak.

## Görev

[`l11_reset.c`](l11_reset.c) bir `Reset_Handler` iskeleti. Hazır olanlar:

- vektör tablosu (`example/l11_vectors.c`),
- zehirleme (`l11_poison_ram()` — **eğitim amaçlı test aracı**: RAM'i
  `0xA5A5A5A5` ile doldurarak fiziksel donanımdaki "tanımsız RAM"i taklit
  eder; silme),
- faz kaydı, `SystemInit()` ve `main()` çağrıları.

Eksik iki adım var — dosyadaki `TODO(student)` satırları:

1. `.data`'nın ilk değerlerini yerine taşımak.
2. `.bss`'i sıfırlamak.

Yalnızca `fixtures.h`'te bildirilen linker sembollerini kullan:
`__etext`, `__data_start`, `__data_end`, `__bss_start`, `__bss_end`.
**Adres sabiti yazma.** Faz kayıtlarının yerini ve sırasını değiştirme.

## Build & Run

```bash
cmake --build --preset debug --target 11-exercise-startup
./scripts/run-qemu.sh build/debug/lessons/11-startup/11-exercise-startup.elf
```

Başlangıç çıktısı (gerçek):

```text
===== LESSON 11 EXERCISE 01 =====
[  g_l11_init] 0xA5A5A5A5
[  g_l11_zero] 0xA5A5A5A5
[  g_l11_phase] 0x00000123
[  vector[0]] 0x30008000
[  CHECK_DATA_INIT] FAIL
[  CHECK_BSS_ZERO] FAIL
[  CHECK_PHASE_ORDER] PASS
[  CHECK_VECTOR0_EQ_STACK_TOP] PASS
===== END =====
```

Bu **beklenen başlangıç durumudur**. İki adım birden eksikken bile konsol
çalışıyor: UART ve konsol `.data`/`.bss`'e dayanmıyor (ölçüldü).

## Doğrulama

Dört kontrolün dördü de `PASS`: `g_l11_init = 0x11C0FFEE`, `g_l11_zero = 0`,
faz sırası `0x123`.

## Cevaplaman gereken sorular

1. `.data`'nın ilk değerleri hangi bellekte, hangi adreste duruyor? Değişken
   kendisi nerede?
2. Hangi sembol "ilk değerlerin başladığı yer"? Adı neden yanıltıcı?
   (Ders 10'a bak.)
3. Kaç byte kopyalanmalı? Bunu hangi iki sembolden hesaplarsın?
4. Neden word (4 byte) birimiyle çalışmak güvenli? Linker script bunu nasıl
   garanti ediyor?
5. `.bss` sıfırlanmadan önce `.data` kopyalansa ya da tersi, fark eder mi?
6. Zehirleme olmasaydı, `.bss` adımını unutan bir çözüm QEMU'da ne
   raporlardı? Neden?

## Takıldıysan

Referans çözüm: [`../../solutions/01-reset-sequence/`](../../solutions/01-reset-sequence/)

---

[← Ders 11](../../README.md)
