**Dil:** [English](README.md) | Türkçe

# Exercise 01 — Build the Thread MPU Policy

**Hedef:** Ders 05'teki yazılım sözleşmesini donanım kuralına çevirmek.

## Görev

`main.c` içindeki `mpu_configure()` fonksiyonunu tamamla. `MAIR` ayarı ve
`ARM_MPU_Disable()` hazır; geri kalan dört adım senin.

| Adım | Ne |
|---|---|
| 1 | **Region 0 — FLASH:** ayrıcalıksız okuma + çalıştırma, yazma yok |
| 2 | **Region 1 — Thread bloğu:** ayrıcalıksız okuma + yazma, çalıştırma yok |
| 3 | MemManage exception'ını etkinleştir |
| 4 | MPU'yu enable et; ayrıcalıklı erişimler varsayılan haritayı kullansın |

`g_service_counter` için bölge **tanımlama**. Korumanın kaynağı bir bölge
eklememek: ayrıcalıksız erişim hiçbir bölgeyle eşleşmezse reddedilir.

Hazır adres sabitleri: `FLASH_BASE_ADDR`, `FLASH_LIMIT_ADDR`, `UNPRIV_BASE`,
`UNPRIV_LIMIT`, `ATTR_IDX_NORMAL`.

## Kullanabileceğin CMSIS API'si

Repo'da vendored duran resmi başlık:
`external/CMSIS/Core/Include/m-profile/armv8m_mpu.h`

```c
ARM_MPU_SetRegion(rnr, rbar, rlar)
ARM_MPU_RBAR(BASE, SH, RO, NP, XN)
ARM_MPU_RLAR(LIMIT, AttrIndx)        /* LIMIT = bölgenin SON byte'ı */
ARM_MPU_SH_NON
ARM_MPU_Enable(control)
MPU_CTRL_PRIVDEFENA_Msk
SCB_SHCSR_MEMFAULTENA_Msk
```

`ARM_MPU_RBAR` parametreleri: `RO = 1` salt-okunur, `NP = 1` ayrıcalıksız
erişime açık, `XN = 1` çalıştırılamaz.

## ⚠️ Armv7-M örneklerine dikkat

Armv8-M MPU **base + limit** modelidir. Armv7-M'in "region size" alanı
**burada yoktur** ve bölgenin kendi boyutuna hizalı olma zorunluluğu da
yoktur. İnternette bulacağın MPU örneklerinin çoğu Armv7-M içindir ve
doğrudan uygulanamaz.

`LIMIT` **dahildir**: bölgenin son byte'ının adresi. Granularite 32 byte.

## Build & Run

```bash
cmake --build --preset debug --target 06-exercise-mpu
./scripts/run-qemu.sh build/debug/lessons/06-mpu/06-exercise-mpu.elf
```

Başlangıç çıktısı (gerçek):

```text
[  CHECK_MPU_ENABLED] FAIL
[  CHECK_PRIVDEFENA] FAIL
[  CHECK_MEMFAULT_ENABLED] FAIL
[  CHECK_FLASH_REGION_RO_EXEC] FAIL
[  CHECK_STACK_REGION_RW_XN] FAIL
[  CHECK_THREAD_UNPRIV] PASS
[  CHECK_UNPRIV_REGION_WRITABLE] PASS
[  CHECK_SVC_PROTECTED_ACCESS] PASS
===== END =====
```

MPU kapalı olduğu için firmware sorunsuz çalışıyor ve SVC yolu işliyor — ama
**koruma yok**. Bu beklenen başlangıç durumudur, bir bug değildir.

## Doğrulama

Sekiz `CHECK_*` satırının hepsi `PASS` olmalı.

`CHECK_FLASH_REGION_RO_EXEC` ve `CHECK_STACK_REGION_RW_XN` kontrolleri
register'ları geri okuyup `AP`, `XN` ve `EN` bitlerini denetler. Böylece
politikanın donanıma yazıldığı kontrol edilir.

### İhlali de doğrula

Politikayı uyguladıktan sonra korumayı şu deneyle kontrol et:

```bash
cmake --build --preset debug --target 06-mpu-broken
./scripts/run-qemu.sh build/debug/lessons/06-mpu/06-mpu-broken.elf
```

Bu hedef (referans politikayla) doğrudan yazma deniyor ve `MemManage`
alıyor. GDB'de `cm-fault` ile `MMFAR`'ın `COUNTER_ADDR` ile aynı olduğunu
doğrula.

## Takıldıysan

Referans çözüm: [`../../solutions/01-thread-mpu-policy/`](../../solutions/01-thread-mpu-policy/README.tr.md)

---

[← Ders 06](../../README.tr.md)
