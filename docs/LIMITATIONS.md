# Limitations - Simulator ile Gercek Donanim Farklari

QEMU **fonksiyonel** bir emülatördür: komut sonuçlarını modeller, donanım
zamanlamasını modellemez. Deney sonuçları aşağıdaki sınırlar içinde
yorumlanmalıdır.

Olculen degerler icin: `docs/TARGET_CAPABILITIES.md`

## 1. Timing gercek degil

QEMU komutlari ceviri blogu (TB) halinde calistirir. Bir "cycle" kavrami
gercek anlamda yoktur.

Bu yuzden:

- "Bu fonksiyon 120 cycle suruyor" gibi iddialari QEMU'da **yapamayiz**
- `-O2` ile `-O0`'i **kod boyutu ve uretilen assembly** uzerinden karsilastiririz,
  duvar saati uzerinden degil

## 2. DWT cycle counter SAYMIYOR

Olculdu: `CYCCNTENA` acilsa bile `DWT->CYCCNT` sifirda kaliyor.

```text
DWT CYCCNT PROFILING: NOT SUPPORTED ON CURRENT TARGET (QEMU mps2-an505)
```

Sonuc: klasik "DWT ile cycle say" profiling yontemi bu target'ta
**calistirilamaz**. Ders 12 bu sınırı gösterir ve
alternatif ölçüm yöntemlerini kullanır (`docs/PERFORMANCE_MEASUREMENT.md`).

## 3. Cache yok

Cortex-M33'te mimari olarak cache yoktur; QEMU'nun eksigi degildir.
Cache davranisi (temizleme, invalidate, coherency) bu target'ta
**gozlemlenemez**. Ders 16'da teorik olarak islenir, deney icin
M7/M55 sinifi bir target veya gercek donanim gerekir.

## 4. Bellek erisim hatalari farkli olabilir

Gercek bir MCU'da tanimsiz adrese erisim bus hatasi uretir.
QEMU'da bazi adresler sessizce okunabilir/yazilabilir. Ornegin
`0x00000000` bu board'da gercek bir RAM alias'idir; "null pointer
yazdim, hemen fault almaliydim" beklentisi burada yanilticidir.

Bu yuzden fault derslerinde hatayi **adres uydurup** degil,
MPU kurup **tanimli bir ihlal** yaparak uretiyoruz.

## 5. Peripheral seti sinirli

QEMU AN505 modelinde CMSDK UART, timer, watchdog gibi temel cevre birimleri
vardir. Gercek bir MCU'nun ADC/SPI/I2C surucu deneyimi burada yoktur.
Bu repo zaten **cekirdek mimariye** odaklanir, surucu yazmaya degil.

## 6. Reset ve power davranisi

Brown-out, power-on reset gecikmesi, clock tree konfigurasyonu, PLL kilitlenmesi
gibi konular modellenmiyor. `SystemInit()` icinde saat kurulumu yapmamizin
gerekmemesi bunun sonucudur; gercek donanimda bu adim zorunludur.

## 7. Bunlar sorun mu?

Ogrenmek istedigimiz konularin buyuk cogunlugu icin **hayir**:

| Konu | QEMU'da ogrenilebilir mi |
|---|---|
| Programmer model, register'lar | Evet |
| Stack, MSP/PSP, calling convention | Evet |
| Privilege, CONTROL, SVC | Evet |
| MPU, memory protection | Evet |
| Exceptions, faults, EXC_RETURN | Evet |
| Linker, ELF, startup | Evet |
| Context switching, PendSV | Evet |
| TrustZone temel ayrimi | Evet |
| **Cycle-accurate performans** | Hayir |
| **Cache davranisi** | Hayir |
| **Gercek cevre birimi suruculeri** | Hayir |
| **Elektriksel/analog davranis** | Hayir |

Board alindiginda nelerin degisecegi: `docs/REAL_HARDWARE_MIGRATION.md`
