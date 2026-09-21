# Repository Architecture

Tasarim kurali: **her ayrimin gercek bir sebebi olmali.**
Bu bir embedded ogrenme reposu, enterprise uygulama degil.

## Katmanlar

```text
lessons/  projects/  final-project/      <- ogrenme ve uygulama
        |
        v
common/        (console, tiny libc)      <- target'tan bagimsiz yardimcilar
        |
        v
platform/mps2-an505/                     <- board'a ozel: startup, UART, linker
        |                                   + mps2_an505.h (CMSIS device header)
        v
external/CMSIS/Core/Include/             <- resmi CMSIS-Core (BASELINE)
        |
        |   (yan yolda, yalnizca dersler icin)
        +-- core/include/core/cm33_teaching.h
        |
        v
cmake/  scripts/                         <- build ve calistirma altyapisi
```

## Neden bu ayrim?

**`core/` vs `platform/`**
`core/` Armv8-M **mimarisine** ait olan her seydir (SCB, NVIC, MPU, SAU, DWT).
Baska bir Cortex-M33 board'una gecince degismez.
`platform/` **board'a** aittir (UART adresi, bellek haritasi, linker script).
M55 portu eklenirse sadece `platform/` cogalir.

**`common/` ayri**
Console ve tiny libc board'dan bagimsizdir; UART'i `platform` saglar,
formatlamayi `common` yapar.

**CMSIS vs teaching header**

Repository'de Cortex-M33 register'larina iki ayri erisim yolu var. Bu bir
tutarsizlik degil, bilincli bir ayrim:

| | Baseline | Teaching |
|---|---|---|
| Dosya | `external/CMSIS/Core/Include/core_cm33.h` (Arm, Apache-2.0) | `core/include/core/cm33_teaching.h` (elle yazilmis, ~200 satir) |
| Giris noktasi | `platform/mps2_an505.h` (CMSIS device header) | dogrudan include |
| Kullanan | startup, system init, driver'lar, `tools/`, projeler, final project | yalnizca register'in kendisinin konu oldugu lesson firmware'leri |
| Amac | Dogruluk, surdurulebilirlik, gercek dunya ile uyum | Gorunurluk |

**Neden baseline CMSIS?**
CMSIS-Core, Arm'in resmi Cortex-M soyutlama katmanidir. Her silikon uretici,
her RTOS ve her ticari SDK onu varsayar. Elle yazilmis bir header uzun vadede
iki sekilde bozulur: ya bir register alani yanlis/eksik kalir, ya da mimari
guncellemelerini takip edemez. Bu nedenle altyapıda resmi CMSIS kullanılır.

**Neden hala elle yazilmis bir header var?**
`core_cm33.h` tek başına 3284 satırdır (ölçüldü). CONTROL register'ının
adresini ve `__get_CONTROL()` çağrısının nasıl çalıştığını incelemek için
daha küçük bir eğitim header'ı kullanıyoruz.

`cm33_teaching.h`, `SCB->CFSR` adresinin `0xE000ED28` olduğunu ve
`MRS`/`MSR` sarmalayıcılarının iki satır inline assembly ile nasıl yazıldığını
gösterir. İsimlendirme CMSIS ile aynı tutulmuştur; böylece aynı çağrılar
iki header ile de kullanılabilir.

**Pratik kural**

```text
lessons/          -> register'in kendisi konuysa teaching header kullanilabilir
diger her sey     -> CMSIS (platform/mps2_an505.h)
```

Ders 12'nin bir odevi: ayni firmware'i once teaching header ile, sonra CMSIS
ile derleyip uretilen makine kodunu karsilastirmak. Fark cikmiyorsa, soyutlama
bedava demektir — ve bu, soyutlamanin ne zaman guvenli oldugu hakkinda
somut bir ders olur.

**CMSIS surumu ve guncelleme**
Kullanilan surum ve tam dosya listesi: [`external/README.md`](../external/README.md).
Dosyalar oldugu gibi (vendored) tutulur, degistirilmez. Bunun nedeni build'in
ag erisimine bagimli olmamasi.

**Platform kapsami**
Bu repo su an tek guvenlik durumu (Secure) ve tek bir hedef varsayar:
QEMU `mps2-an505` / `cortex-m33`. Saat frekansi, NVIC oncelik biti sayisi ve
reset VTOR degeri gibi seyler MIMARI SABIT DEGIL, HEDEFE OZEL sozlesmelerdir;
kaynaklari `platform/mps2-an505/include/platform/board.h` ve
`platform/mps2_an505.h` icinde QEMU kaynagina referansla yazilidir.
Fiziksel kart degerleri bagimsiz olarak dogrulanmamistir.

**Bilinen sinir: SAU ve `-mcmse`**
CMSIS-Core `SAU` tanimlarini yalnizca derleyici `-mcmse` ile cagrildiginda
(`__ARM_FEATURE_CMSE == 3`) acar. Bu repo su an `-mcmse` kullanmiyor, bu
yuzden `tools/capability-probe` SAU_TYPE'i mimari sabit adresinden okur ve
bunu kaynak icinde acikca belirtir. Ders 16'da secure/non-secure ayrimi
kurulurken `-mcmse` acilacak.

## Build akisi

```text
CMakePresets.json
   |  (toolchainFile)
   v
cmake/arm-none-eabi.cmake        <- -mcpu/-mthumb/-mfloat-abi TEK yerde
   |
   v
CMakeLists.txt
   |
   +-- platform/mps2-an505/CMakeLists.txt   -> platform-mps2-an505 (INTERFACE)
   |
   +-- cmake/firmware.cmake                 -> add_firmware()
   |
   +-- lessons/*/CMakeLists.txt             -> her ders kendi firmware'ini tanimlar
```

**Platform neden INTERFACE library?**
Bazi dersler farkli derleyici flag'leri kullanir (ornegin ders 13 FPU'yu
hard-float ile acar). Onceden derlenmis tek bir static library'yi farkli ABI
ile link etmek sessiz hatalara yol acar. INTERFACE ile her firmware platform
kaynaklarini **kendi flag'leriyle** derler. Maliyeti birkac dosyanin
yeniden derlenmesi; kazanci ABI guvenligi.

**`add_firmware()` neden var?**
Her ders ayni 6 seyi istiyor: ELF, map, bin, disassembly, size, run/debug
hedefleri. Bunu 16 kez kopyalamak yerine tek fonksiyon.

## Ogrenci kodu vs altyapi kodu

Net ayrim:

| Altyapi (hazir gelir) | Ogrenci yazar |
|---|---|
| CMake, presets, toolchain dosyasi | Lesson icindeki `TODO(student)` bloklari |
| QEMU/GDB scriptleri | Fault analyzer'in decode mantigi |
| `platform/` startup (calisan surum) | Ders 11'deki sifirdan startup |
| Console/UART surucusu | MPU konfigurasyonu, SVC handler'i |
| `core/cm33_teaching.h` register tanimlari | Context switch kodu, scheduler |

`platform/` altindaki startup **bilerek** calisir durumdadir: ders 11'e
gelmeden once de firmware calistirabilmek gerekir. Ders 11'de ogrenci
kendi startup'ini sifirdan yazar ve ikisini karsilastirir.

## Dizin sorumluluklari

| Dizin | Icerik |
|---|---|
| `core/` | Armv8-M register tanimlari ve intrinsic'ler |
| `platform/mps2-an505/` | Startup, linker script, UART, board adresleri |
| `common/` | Console, minimal libc parcalari |
| `lessons/NN-*/` | Ders anlatimi (README) + calisan ornek |
| `projects/` | Birden fazla dersi birlestiren buyuk isler |
| `final-project/` | Specialization sonu entegrasyon projesi |
| `tools/` | Ogretim disi yardimci firmware (capability probe) |
| `cmake/` | Toolchain dosyasi ve build yardimcilari |
| `scripts/` | Environment check, QEMU, GDB |
| `docs/` | Kalici referans belgeler |
| `tests/` | Smoke test |
| `external/` | Vendored ucuncu taraf kaynak (CMSIS-Core) |

## Bilincli olarak YAPILMAYANLAR

- Docker zorunlulugu yok
- HAL/driver framework yok (bu repo cekirdek mimariye odaklanir)
- Kod uretimi (code generation) yok
- Dependency manager yok (tek dis bagimlilik: toolchain + QEMU)
- Soyutlama katmani ekleme: her `platform_*` fonksiyonunun gercek bir sebebi var

Gerekce: her ek katman, ogrenilmek istenen donanimi bir adim daha uzaga iter.
