**Dil:** [English](README.md) | Türkçe

# Exercise 01 — Decode an Exception Entry

**Hedef:** Exception girişinden geriye dönüp "ne oldu" sorusunu cevaplayabilmek.

## Görev

`main.c` içindeki `decode_exception_entry()` fonksiyonunu doldur. Üç girdi
alıyor ve altı alanı dolduruyor:

```c
static void decode_exception_entry(const exception_frame_t *frame,
                                   uint32_t exc_return,
                                   uint32_t ipsr,
                                   entry_info_t *out);
```

| Alan | Ne yazmalı | Nereden |
|---|---|---|
| `interrupted_mode` | `"Thread"` / `"Handler"` | `EXC_RETURN` |
| `frame_stack` | `"PSP"` / `"MSP"` | `EXC_RETURN` |
| `exception_number` | Çalışan exception'ın numarası | `ipsr` |
| `stacked_r0` | Frame'in `R0` yuvası | `frame` |
| `stacked_pc` | Frame'in `PC` yuvası | `frame` |
| `stacked_xpsr` | Frame'in `xPSR` yuvası | `frame` |

## Kapsam

Yalnızca bu repo'nun desteklediği alt küme: **standart 8 word frame, tek
güvenlik durumu**. Her `EXC_RETURN` kuralını yeniden uygulaman istenmiyor —
iki bit yeterli.

## ⚠️ Tuzak

`frame_stack` ile *"handler hangi stack'i kullanıyor"* **aynı soru değildir**.

- Birincisi `EXC_RETURN` bit 2'den gelir ve frame'in **nerede yığıldığını**
  söyler.
- İkincisinin cevabı **her zaman MSP**'dir.

## Build & Run

```bash
cmake --build --preset debug --target 07-exercise-decode
./scripts/run-qemu.sh build/debug/lessons/07-exceptions/07-exercise-decode.elf
```

Başlangıç çıktısı (gerçek):

```text
[4] Exception girisi cozumu
[  INTERRUPTED_MODE] ?
[  FRAME_STACK] ?
[  EXCEPTION_NUMBER] 0
[  STACKED_R0] 0x00000000
[  CHECK_DECODE_MODE] FAIL
[  CHECK_DECODE_STACK] FAIL
[  CHECK_DECODE_EXCNUM] FAIL
[  CHECK_DECODE_R0] FAIL
[  CHECK_DECODE_PC] FAIL
[  CHECK_DECODE_XPSR] FAIL
```

Firmware sorunsuz çalışıyor; yalnızca çözümleme boş. Bu **beklenen başlangıç
durumudur**, bir bug değildir.

## Doğrulama

Altı `CHECK_DECODE_*` da `PASS` olmalı **ve** diğer bütün `CHECK_*` satırları
`PASS` kalmalı.

Beklenen değerler (`svc #7`, PSP kaynaklı):

```text
INTERRUPTED_MODE  Thread
FRAME_STACK       PSP
EXCEPTION_NUMBER  11
STACKED_R0        0x11111111
```

## Kendi ölçümünle doğrula

```gdb
break svc_dispatch
continue
next 3
cm-exc-return g_entry_lr
cm-frame g_frame_addr g_entry_lr
```

Fonksiyonunun çıkardığı sonuçlar `cm-frame`'in bastıklarıyla aynı mı?

## Opsiyonel (DEEP) — Exercise 02

Şu `EXC_RETURN` değerlerini sınıflandır: Thread mi Handler mı, MSP mi PSP mi,
basic mi extended mi, ve bu laboratuvarın `cm-frame` yardımcısı tarafından
desteklenip desteklenmediği.

```text
0xFFFFFFF9    0xFFFFFFFD    0xFFFFFFED    0xFFFFFFF1    0xFFFFFFF5
```

Sonuncusunda dikkatli ol. Cevaplar `answers/quick-check.md` sonunda.

## Takıldıysan

Referans çözüm: [`../../solutions/01-decode-exception-entry/`](../../solutions/01-decode-exception-entry/README.tr.md)

---

[← Ders 07](../../README.tr.md)
