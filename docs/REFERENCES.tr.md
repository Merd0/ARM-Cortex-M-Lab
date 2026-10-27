**Dil:** [English](REFERENCES.md) | Türkçe

# References

Bu repodaki teknik iddialar oncelikle asagidaki **resmi** kaynaklara
dayandirilir. Tutorial/blog kaynaklari kullanilmaz.

## Arm mimari dokumantasyonu

| Belge | Ne icin |
|---|---|
| **Armv8-M Architecture Reference Manual** (Arm DDI 0553) | Mimarinin tek dogru kaynagi: register'lar, exception modeli, MPU, SAU, komut seti |
| **Cortex-M33 Devices Generic User Guide** (Arm 100235) | Cortex-M33'e ozel programlama bilgisi, CPUID, NVIC, SysTick |
| **Cortex-M33 Technical Reference Manual** (Arm 100230) | Implementasyon detaylari, DWT, hata ayiklama birimleri |
| **Procedure Call Standard for the Arm Architecture (AAPCS)** | Calling convention: argumanlar, donus degeri, korunan register'lar |
| **Arm Cortex-M for Beginners / Armv8-M whitepapers** | Genel bakis ve islemci karsilastirmasi |

Erisim: <https://developer.arm.com/documentation>

## Board / platform

| Belge | Ne icin |
|---|---|
| **Application Note AN505** (Arm DAI0505) | MPS2-AN505 FPGA imaji: bellek haritasi, cevre birimleri |
| **Arm IoT Kit FVP User Guide** (Arm ECM0601256) | AN505'in dayandigi IoT Kit alt sistemi |
| **Cortex-M System Design Kit TRM** (Arm DDI 0479) | CMSDK APB UART register yerlesimi |
| **MPS2+ Board TRM** (Arm 100112) | Board seviyesinde donanim |

## QEMU

| Kaynak | Ne icin |
|---|---|
| QEMU Arm MPS2/MPS3 board dokumantasyonu | Hangi makinelerin modellendigi, QEMU ile gercek donanim farklari |
| `qemu-system-arm -machine help` | Mevcut makineler (yerel dogrulama) |
| `qemu-system-arm -cpu help` | Mevcut islemciler (yerel dogrulama) |

<https://www.qemu.org/docs/master/system/arm/mps2.html>

## Toolchain

| Kaynak | Ne icin |
|---|---|
| Arm GNU Toolchain indirme ve release notlari | Derleyici surumu ve ozellikleri |
| GNU ld dokumantasyonu | Linker script sozdizimi |
| GNU binutils dokumantasyonu | objdump/objcopy/nm/size |
| GDB dokumantasyonu | Debug komutlari, remote protokol |

<https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>

## CMSIS

| Kaynak | Ne icin |
|---|---|
| CMSIS-Core (M) dokumantasyonu | Standart register isimlendirmesi ve API'si |

Not: Bu repo'nun baseline core API'si **resmi CMSIS-Core**'dur
(`external/CMSIS/`, Apache-2.0; giris noktasi `platform/mps2_an505.h`).
Bunun yaninda, register'in kendisinin konu oldugu derslerde kullanilmak uzere
elle yazilmis minimal bir egitim header'i da vardir
(`core/include/core/cm33_teaching.h`). Isimlendirme ikisinde de ayni tutulur.
Gerekce ve kullanim kurali: `docs/ARCHITECTURE.md`

## Kaynak kullanim kurali

- Teknik bir iddia yazilacaksa once resmi belgeden dogrulanir
- Olculebilen bir sey varsa **olculur**, kaynak gosterilmez ve "olculdu" denir
- Dogrulanamayan bilgi repoya `OLCULMEDI` / `DOGRULANMADI` olarak girer

---

## Kaynak politikasi (onarim sonrasi netlestirildi)

Onceki surum "olculmus veri icin referans gerekmez" anlamina gelebilecek bir
ifade tasiyordu. Duzeltme: **olcumun kendisi** icin referans gerekmez, ama
**olcumun yorumu** icin gerekir.

| Ne | Kaynak |
|---|---|
| "QEMU'da `SAU_TYPE` 0x8 okundu" | Olcum. Kaynak: `tools/capability-probe`. |
| "Yani bu hedefte 8 SAU bolgesi var" | Yorum. Kaynak: Armv8-M ARM, `SAU_TYPE` tanimi. |
| "Demek ki her Cortex-M33'te 8 bolge var" | **Gecersiz.** Ne olcum ne belge bunu soyler. |

Basarili bir olcum, evrensel bir mimari iddiayi kanitlamaz. Tek bir hedefte
gozlenen davranis o hedefin sozlesmesidir.

Birincil kaynaklar (dogrudan baglantilar):

- Armv8-M Architecture Reference Manual (Arm DDI 0553):
  <https://developer.arm.com/documentation/ddi0553/latest/>
- Cortex-M33 Devices Generic User Guide (Arm **100235**):
  <https://developer.arm.com/documentation/100235/latest/>
- Cortex-M33 Technical Reference Manual (Arm 100230):
  <https://developer.arm.com/documentation/100230/latest/>
- AAPCS32: <https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst>
- CMSIS-Core (M): <https://arm-software.github.io/CMSIS_6/latest/Core/index.html>
- QEMU v8.2.2 AN505 modeli:
  <https://github.com/qemu/qemu/blob/v8.2.2/hw/arm/mps2-tz.c>
- QEMU v8.2.2 NVIC:
  <https://github.com/qemu/qemu/blob/v8.2.2/hw/intc/armv7m_nvic.c>
- GNU as (ARM directives):
  <https://sourceware.org/binutils/docs/as/ARM-Directives.html>

Not: `tests/check-links.py` yalnizca **goreli** Markdown linklerini dogrular.
Yukaridaki dis URL'leri ve belge icindeki bolum capalarini dogrulamaz.
