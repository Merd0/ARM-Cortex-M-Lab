# Exercise 01 — Add an SVC Service

**Hedef:** Ayrıcalıksız bir isteğin ayrıcalıklı bir servise dönüşme yolunu
kendin kurmak.

## Görev

`main.c` içindeki `svc_dispatch()` fonksiyonuna **`SVC #2`** için bir `case`
ekle.

```c
#define SVC_GET_VERSION      2u
```

Servis: `get_service_version()` — sabit bir sürüm/kabiliyet değeri döndürür.

| Gereksinim | Not |
|---|---|
| İstek ayrıcalıksız Thread Mode'dan gelsin | Zaten öyle çağrılıyor |
| Dispatcher `SVC #2`'yi tanısın | Senin eklediğin `case` |
| Servis ayrıcalıklı Handler Mode'da çalışsın | Dispatcher zaten orada |
| Dönüş değeri `0x00010002` olsun | |
| Thread normal devam etsin | |

## Build & Run

```bash
cmake --build --preset debug --target 05-exercise-svc2
./scripts/run-qemu.sh build/debug/lessons/05-privilege-svc/05-exercise-svc2.elf
```

Başlangıç çıktısı (gerçek):

```text
[  UNKNOWN_SVC] 2
...
[6] SVC #2 (get_service_version)
[  SVC2_VERSION] 0xFFFFFFFF
[  CHECK_SVC2_VERSION] FAIL
```

`SVC #2` şu an hiçbir `case`'e uymuyor ve `default` dalına düşüyor. Bu
**beklenen başlangıç durumudur**, bir bug değil.

## Doğrulama

`CHECK_SVC2_VERSION` `PASS` olmalı **ve** diğer bütün `CHECK_*` satırları
`PASS` kalmalı:

```text
CHECK_PRIV_DROPPED
CHECK_SELF_PROMOTION_BLOCKED
CHECK_PERIPHERAL_BLOCKED_UNPRIV
CHECK_SVC_SERVICE
CHECK_SVC_DID_NOT_PROMOTE
CHECK_SVC2_VERSION
CHECK_PRIV_RESTORED_BY_HANDLER
```

## Asıl soru

Sonucu Thread'e **nasıl** ulaştıracaksın?

`return` yazamazsın: `svc_dispatch`'in dönüşü Thread'e değil, **exception
return'e** gidiyor (`svc_handler.S`'in neden `bl` yerine `b` kullandığına
bak). Thread'in gördüğü `R0`, exception frame'inden geri yüklenen değerdir.

`SVC_INCREMENT` servisine bak: sonucu nereye yazıyor ve donanım o word'ü ne
zaman `R0`'a geri yüklüyor?

## GDB ile doğrula

```gdb
break svc_dispatch
continue
# SVC #2 cagrisina kadar ilerle
p svc_number
p/x frame->r0
```

Servisi yazdıktan sonra dönüşte:

```gdb
info registers r0
```

## Takıldıysan

Referans çözüm: [`../../solutions/01-add-svc-service/`](../../solutions/01-add-svc-service/)

---

[← Ders 05](../../README.md)
