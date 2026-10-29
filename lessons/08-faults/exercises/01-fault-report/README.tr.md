**Dil:** [English](README.md) | Türkçe

# Exercise 01 — Complete the Fault Report

**Hedef:** Ham fault register'larından **kararlar** üretmek — triage listesinin
kod hali.

## Görev

`decode.c` içindeki `decode_fault_snapshot()` fonksiyonunu doldur. Firmware
tek bir deterministik fault üretiyor: ayrıcalıksız Thread'in korunan veriye
doğrudan yazması (ders 06'daki MemManage `DACCVIOL`). Ham snapshot hazır.

| Alan | Karar | İpucu |
|---|---|---|
| `active_exception` | Hangi handler çalışıyor? | `s->ipsr`: 3/4/5/6 |
| `escalated` | Yükseltilmiş mi? | `HFSR.FORCED` |
| `fault_class` | `CFSR`'nin hangi alt alanı dolu? | `MMFSR`/`BFSR`/`UFSR` |
| `primary_bit` | Asıl sebep biti | `VALID` bitleri sebep **değildir** |
| `address_valid` | Adres register'ı kanıt mı? | `MMARVALID` / `BFARVALID` |
| `fault_address` | Yalnızca geçerliyse | |
| `frame_safe` | Frame çözülebilir mi? | `s->frame_valid` |
| `stacked_pc` | Yalnızca frame güvenliyse | |

**Kapsam:** her `CFSR` bitini çözmen istenmiyor. Bu dersin üç fault türü
(MemManage, BusFault, UsageFault) yeter.

## ⚠️ Tuzak

`fault_class` **aktif handler'dan türetilmez**, `CFSR`'den türetilir.
Escalation vakasında aktif handler HardFault'tur ama `fault_class` UsageFault
(veya MemManage) olmalıdır. Kodunu yazarken bunu düşün — bu hedef escalation
üretmiyor olsa bile.

## Build & Run

```bash
cmake --build --preset debug --target 08-exercise-fault-report
./scripts/run-qemu.sh build/debug/lessons/08-faults/08-exercise-fault-report.elf
```

Başlangıç çıktısı (gerçek):

```text
[  D_ACTIVE_EXCEPTION] ?
[  D_ESCALATED] YES
[  D_FAULT_CLASS] ?
[  D_PRIMARY_BIT] ?
[  CHECK_D_ACTIVE] FAIL
[  CHECK_D_NOT_ESCALATED] FAIL
[  CHECK_D_CLASS] FAIL
[  CHECK_D_BIT] FAIL
[  CHECK_D_ADDRESS] FAIL
[  CHECK_D_FRAME] FAIL
```

Bu **beklenen başlangıç durumudur**, bir bug değildir.

## Doğrulama

Altı `CHECK_D_*` satırı da `PASS` olmalı.

Metin alanları **tam olarak** karşılaştırılır — yalnızca ilk harf değil.
Beklenen değerlerin yazımı exercise yorumunda verildi (`"MemManage"`,
`"BusFault"`, `"UsageFault"`, `"HardFault"`; sebep bitleri CMSIS'teki
adlarıyla, örneğin `"DACCVIOL"`). `"DIVBYZERO"` gibi aynı harfle başlayan
yanlış bir cevap `CHECK_D_BIT`'ten **geçmez**.

## Kendi fonksiyonunu diğer vakalarda sına (opsiyonel)

`decode.c`'yi `08-faults-bus` veya `08-faults-escalation` hedefine de bağlayıp
kararlarının doğru çıkıp çıkmadığına bak. Escalation vakasında
`active_exception = HardFault`, `escalated = 1`, `fault_class = UsageFault`
olmalı.

## Takıldıysan

Referans çözüm: [`../../solutions/01-fault-report/`](../../solutions/01-fault-report/README.tr.md)

---

[← Ders 08](../../README.tr.md)
