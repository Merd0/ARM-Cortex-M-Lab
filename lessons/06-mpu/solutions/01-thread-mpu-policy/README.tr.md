**Dil:** [English](README.md) | Türkçe

# Solution 01 — Build the Thread MPU Policy

Exercise: [`../../exercises/01-thread-mpu-policy/`](../../exercises/01-thread-mpu-policy/README.tr.md)

## Çözüm

```c
static void mpu_configure(void)
{
    ARM_MPU_Disable();

    ARM_MPU_SetMemAttr(ATTR_IDX_NORMAL,
        ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1u,1u,1u,1u),
                     ARM_MPU_ATTR_MEMORY_(1u,1u,1u,1u)));

    /* Region 0: FLASH — ayrıcalıksız oku + çalıştır, yazma yok */
    ARM_MPU_SetRegion(0u,
        ARM_MPU_RBAR(FLASH_BASE_ADDR, ARM_MPU_SH_NON, 1u, 1u, 0u),
        ARM_MPU_RLAR(FLASH_LIMIT_ADDR, ATTR_IDX_NORMAL));

    /* Region 1: Thread bloğu — ayrıcalıksız oku + yaz, çalıştırma yok */
    ARM_MPU_SetRegion(1u,
        ARM_MPU_RBAR(UNPRIV_BASE, ARM_MPU_SH_NON, 0u, 1u, 1u),
        ARM_MPU_RLAR(UNPRIV_LIMIT, ATTR_IDX_NORMAL));

    /* g_service_counter için BİLEREK bölge yok */

    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
}
```

## Build & Run

```bash
cmake --build --preset debug --target 06-solution-mpu
./scripts/run-qemu.sh build/debug/lessons/06-mpu/06-solution-mpu.elf
```

Gerçek çıktı (kontrol satırları):

```text
[  CHECK_MPU_INITIALLY_DISABLED] PASS
[  CHECK_MPU_ENABLED] PASS
[  CHECK_PRIVDEFENA] PASS
[  CHECK_MEMFAULT_ENABLED] PASS
[  CHECK_FLASH_REGION_RO_EXEC] PASS
[  CHECK_STACK_REGION_RW_XN] PASS
[  CHECK_THREAD_UNPRIV] PASS
[  CHECK_UNPRIV_REGION_WRITABLE] PASS
[  CHECK_SVC_PROTECTED_ACCESS] PASS
===== END =====
```

---

## Neden her parça gerekli?

### Neden FLASH region'ı gerekli?

MPU etkinken ayrıcalıksız erişimler için "eşleşme yoksa reddet" kuralı
geçerlidir — ve bu **komut getirmeyi de** kapsar. FLASH için bir bölge
tanımlamazsan ayrıcalıksız Thread ilk komutunu bile getiremez ve anında
`IACCVIOL` alırsın.

`RO = 1`: kodun kendisini yazmaya karşı korur.
`XN = 0`: buradan komut getirilebilir — bu bölgenin varlık sebebi.

Region 0 ayrıca `.rodata`'yı da kapsıyor (aynı FLASH aralığı), yani
ayrıcalıksız kod sabit string ve tabloları okuyabiliyor.

### Neden task stack region'ı gerekli?

Aynı kural veri tarafında. Bölge olmazsa Thread kendi stack'ine ilk `push`'u
yapamaz — ve bu `psp_start()` hedefinin prolog'unda, yani daha ilk anda olur.

`RO = 0`, `NP = 1`: oku ve yaz, ayrıcalıksız erişime açık.

Region 1 hem stack'i hem Thread'in kendi veri alanını (`unpriv_obs_t`)
kapsıyor. İkisi tek bir 32 byte hizalı blokta duruyor — bölgeler
`RBAR.BASE`/`RLAR.LIMIT` ile 32 byte granularitede tanımlandığı için
dağınık nesneleri ayrı ayrı kapsamak bölge israfı olurdu.

### Neden stack `XN`?

Oradan kod çalıştırmak için meşru bir sebep yok. `XN = 1`, stack taşması veya
tampon aşımı ile enjekte edilen verinin **çalıştırılmasını** engeller — klasik
W^X sertleştirmesi. İhlali `MMFSR.IACCVIOL` olarak görünür (`DACCVIOL` değil).

Region 0 ile birlikte kural tamamlanır: yazılabilir olan çalıştırılamaz,
çalıştırılabilir olan yazılamaz.

### Neden `g_service_counter` için izin verilmedi?

**Koruma bir bölge eklememekten geliyor.** Armv8-M'de MPU etkinken,
ayrıcalıksız bir erişim eşleşen bölge bulamazsa reddedilir. Sayaç hiçbir
bölgenin içinde olmadığı için ayrıcalıksız Thread'e kapalı.

Ölçülen adresler bunu gösteriyor:

```text
COUNTER_ADDR = 0x30000000
UNPRIV_BASE  = 0x30000020    <- Region 1 buradan basliyor
```

Ayrı bir "yasak bölge" tanımlamadık — Armv8-M MPU'da "erişimi reddet" diye
bir bölge türü zaten yoktur. İzin **verilen** bölgeler listelenir, geri kalan
kapalıdır.

### Ayrıcalıklı Handler neden erişebiliyor?

`MPU_CTRL.PRIVDEFENA = 1` sayesinde. Ayrıcalıklı bir erişim hiçbir bölgeyle
eşleşmezse mimarinin **varsayılan bellek haritası** devreye girer ve erişim
izinli olur.

Ayrıcalıksız erişimlerin böyle bir kurtarma yolu **yoktur**. Bu dersin
politikası tam olarak bu asimetriye dayanıyor.

### `PRIVDEFENA` tam olarak ne yapıyor?

```text
PRIVDEFENA = 1   ayricalikli + eslesme yok  -> varsayilan harita (izin)
PRIVDEFENA = 0   ayricalikli + eslesme yok  -> REDDET
her durumda      ayricaliksiz + eslesme yok -> REDDET
```

`0` seçersen ayrıcalıklı kodun kullandığı her adres için de bölge tanımlaman
gerekir: MSP stack'i, kernel verisi, UART, NVIC... Bu derste MPU'yu enable
eden `main()` kendi stack'ine erişemez hale gelirdi. Bazı yüksek güvenlikli
tasarımlar bunu bilerek yapar ama 8 bölge hızla yetmez.

### `MAIR` neden gerekli?

`RLAR.AttrIndx` zorunlu bir alandır: her bölge `MAIR0`/`MAIR1` içindeki 8
attribute girdisinden birini seçmek **zorundadır**. Attribute'suz bölge
tanımlanamaz.

Attribute, işlemciye o belleğin **ne tür bellek** olduğunu söyler — Normal mi
Device mi, cacheable mı, shareable mı. Yani MPU yalnızca "kim erişebilir"
sorusunu değil, "bu bellek nasıl davranır" sorusunu da cevaplar.

Bu derste tek attribute kullanıldı (Normal, WB, non-transient, RA/WA) ve
`MAIR0 = 0x000000FF` ölçüldü. Cache davranışı bu dersin konusu değil.

### `RBAR`/`RLAR` nasıl yorumlanıyor?

Ölçülen ham değerler ve çözümleri:

```text
Region 0
RBAR = 0x10000006   XN=0  AP=11 (RO=1,NP=1)  SH=00  BASE=0x10000000
RLAR = 0x1007FFE1   EN=1  AttrIndx=0  PXN=0  LIMIT=0x1007FFE0
                    -> DAHIL son byte: 0x1007FFFF

Region 1
RBAR = 0x30000023   XN=1  AP=01 (RO=0,NP=1)  SH=00  BASE=0x30000020
RLAR = 0x30000321   EN=1  AttrIndx=0  PXN=0  LIMIT=0x30000320
                    -> DAHIL son byte: 0x3000033F
```

Bit yerleşimi:

```text
RBAR: [31:5] BASE | [4:3] SH | [2:1] AP | [0] XN
RLAR: [31:5] LIMIT | [4] PXN | [3:1] AttrIndx | [0] EN
```

`LIMIT` **dahildir**: son byte'ın adresi. CMSIS başlığının ifadesiyle değer
"one extended" — alt 5 bit 1 kabul edilir. Bu yüzden `0x1007FFE0` yazıp
`0x1007FFFF`'e kadar kapsıyorsun.

### Neden Armv7-M "region size" örnekleri kullanılamaz?

| | Armv7-M (PMSAv7) | Armv8-M (PMSAv8) |
|---|---|---|
| Bölge | `base` + kodlanmış **size** (2'nin kuvveti) | `BASE` + `LIMIT` |
| Hizalama | Bölge kendi boyutuna hizalı olmalı | 32 byte yeter |
| Alt bölge | `SRD` alanı var | Yok |
| Bellek türü | `TEX`/`C`/`B`/`S` bölge içinde | `AttrIndx` → `MAIR` |

Armv7-M örneklerindeki `SIZE` ve `SRD` alanları Armv8-M'de **yoktur**.
Kopyalarsan ya derlenmez ya da bambaşka bir bölge programlarsın. Doğru
kaynak: Armv8-M ARM veya repodaki vendored
`external/CMSIS/Core/Include/m-profile/armv8m_mpu.h`.

---

## Korumanın işlediğini doğrula

Politika doğruysa ihlal `MemManage` üretir:

```text
IPSR  = 4               MemManage (HardFault degil)
CFSR  = 0x00000082      MMFSR.DACCVIOL = 1, MMARVALID = 1
HFSR  = 0x00000000      escalation yok
MMFAR = 0x30000000      = &g_service_counter
stacked PC = 0x10000078 = "str r1, [r0, #0]"
```

---

## Sınır

Bu politika iki bölgeden ibaret. Gerçek bir sistemde çevre birimleri,
paylaşılan tamponlar ve **her görev için ayrı stack bölgesi** gerekir — ve
bu hedefte toplam 8 bölge var. Politika tasarımı bir bütçe işidir.

---

[← Ders 06](../../README.tr.md)
