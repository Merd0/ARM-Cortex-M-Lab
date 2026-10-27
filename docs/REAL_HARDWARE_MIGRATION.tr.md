**Dil:** [English](REAL_HARDWARE_MIGRATION.md) | Türkçe

# Real Hardware Migration

Bir Cortex-M33 board'u (ornegin bir Armv8-M/TrustZone destekli MCU kartı)
alindiginda neler degisir?

## Degismeyenler (bu repodaki ogrenmenin buyuk kismi)

- Programmer model, register'lar, xPSR
- MSP/PSP, Thread/Handler Mode, CONTROL, privilege
- Exception modeli, EXC_RETURN, NVIC, fault status register'lari
- MPU programlama mantigi
- AAPCS, assembly, linker/ELF kavramlari
- Context switching mantigi (SysTick + PendSV)
- GDB komutlari ve teshis akisi

Bunlar **mimariye** aittir, board'a degil.

## Degisenler

### 1. Linker script ve bellek haritasi

| Simdi (QEMU AN505) | Gercek MCU |
|---|---|
| FLASH `0x10000000` (SRAM, yazilabilir) | Gercek flash, genelde `0x00000000`/`0x08000000` |
| RAM `0x30000000`, 32 KB | Cihaza ozel adres ve boyut |
| Flash yazma yok | Flash programlama adimi gerekir |

Degisecek dosya: `platform/<board>/linker/*.ld`

### 2. Clock kurulumu

QEMU'da `SystemInit()` neredeyse bos. Gercek MCU'da:
PLL konfigurasyonu, flash wait state, voltage scaling gerekir.
Yanlis wait state = acikliyamadigin fault'lar.

### 3. Console

Simdi: CMSDK APB UART, `0x50200000`.
Gercek: cihaza ozel UART + pin (GPIO alternate function) konfigurasyonu.
Degisecek dosya: `platform/<board>/src/uart_*.c`

### 4. Yukleme ve debug baglantisi

| Simdi | Gercek |
|---|---|
| `qemu-system-arm -kernel fw.elf` | `openocd` / `pyocd` / vendor araci + SWD probe |
| `-gdb tcp::3333` | OpenOCD/pyOCD GDB server (genelde port 3333) |

GDB komutlari aynen calisir - sadece baglanilan sunucu degisir.
Bu yuzden `scripts/gdb/cortex-m.gdb` tasinabilir.

### 5. Yeni platform eklemek

```text
platform/
    mps2-an505/        <- mevcut
    <new-board>/       <- yeni
        include/platform/board.h
        src/startup_<board>.c
        src/uart_<board>.c
        linker/<board>.ld
        CMakeLists.txt
```

`core/`, `common/`, `lessons/` degismez. Mimari katmanin board'dan
ayrilmasinin sebebi tam olarak budur (`docs/ARCHITECTURE.md`).

## Yeni acilan kapilar

| Konu | Simdi | Gercek donanimda |
|---|---|---|
| **DWT cycle counter** | Saymiyor | Calisir - gercek profiling mumkun |
| **Timing olcumu** | Anlamsiz | Gercek; osiloskop/GPIO ile dogrulanabilir |
| **Cache** | M33'te yok | M7/M55 sinifi cihazlarda gozlemlenebilir |
| **Guc tuketimi** | Yok | WFI/sleep modlarinin etkisi olculebilir |
| **Gercek cevre birimleri** | Cok sinirli | ADC/SPI/I2C/DMA |
| **Elektriksel sorunlar** | Yok | Gurultu, brown-out, sinyal butunlugu |

## Gecis kontrol listesi

- [ ] Yeni `platform/<board>/` dizini olustur
- [ ] Linker script'i cihazin flash/RAM adreslerine gore yaz
- [ ] Startup'ta clock ve flash wait state kurulumunu ekle
- [ ] UART surucusunu ve pin konfigurasyonunu yaz
- [ ] OpenOCD/pyOCD config dosyasi ekle
- [ ] `scripts/` icine flash yukleme scripti ekle
- [ ] `capability-probe`'u calistir ve `TARGET_CAPABILITIES.md`'ye **olculmus**
      bir sutun ekle
- [ ] DWT'nin gercekten saydigini dogrula, profiling derslerini guncelle
