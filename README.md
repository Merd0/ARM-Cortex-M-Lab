# ARM Cortex-M Lab

Cortex-M33 için bare-metal C ve Assembly laboratuvarı. Firmware’i QEMU’da çalıştır, GDB ile işlemci davranışını izle ve kontrollü hatalarla teşhis pratiği yap. Hedef platform Cortex-M33 / QEMU `mps2-an505`.

**16 ders · 5 entegrasyon projesi · 1 TrustZone finali**

[Ders planı](BOOTCAMP.md) · [Secure Real-Time Sensor Node](final-project/secure-real-time-sensor-node/) · [v1.0.0 CI kanıtı](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40545977241)

> **English overview.** A hands-on Cortex-M33 lab with 16 lessons, five integration projects, and a TrustZone sensor-node capstone. Bare-metal C and Assembly experiments run on QEMU with GDB and automated checks across three build presets.

## Dersler, projeler ve final

| Katman | İçerik |
|---|---|
| [16 ders](lessons/) | Register ve stack deneylerinden exception, MPU, context switch ve TrustZone’a uzanan adım adım laboratuvarlar. |
| [5 entegrasyon projesi](projects/) | Birden fazla dersin mekanizmalarını tek bir firmware probleminde birleştiren uygulamalar. |
| [Secure Real-Time Sensor Node](final-project/secure-real-time-sensor-node/) | Secure ve Non-secure firmware imajlarıyla çalışan uçtan uca sensör sistemi. |

## Secure Real-Time Sensor Node

Üç ayrıcalıksız Non-secure görev sentetik sensör örneklerini toplar, işler ve UART üzerinden loglar. Ayrıcalıklı Non-secure çekirdek SVC ile servis sunar, PendSV ile görev değiştirir ve MPU ile görev stack’lerini ayırır. İşleme hattı, sınırlı DSP/FPU kalibrasyonu için TrustZone geçidini kullanır.

Normal Debug senaryosunda `PRODUCED=PROCESSED=LOGGED=32`. Test, işlenen sonucu bağımsız bir hesapla karşılaştırır. Negatif senaryolardan biri başka görevin stack’ine erişimin engellendiğini kontrol eder.

```bash
cmake --preset debug
cmake --build --preset debug
python3 tests/system-smoke.py --group final
```

[Final mimarisi, çıktı ve negatif senaryolar](final-project/secure-real-time-sensor-node/)

## v1.0.0 doğrulaması

[`2b2e1dc` CI çalıştırmasında](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40545977241) **Debug, Release ve MinSize** başarılı. Aşağıdaki sonuçlar her preset için ayrı raporlanmıştır.

| Kontrol | Sonuç |
|---|---:|
| QEMU smoke ve regresyon kontrolleri | 566 |
| Build probe | 47 |
| GDB entegrasyon oturumu | 13 |
| Beş entegrasyon projesinin senaryoları | 15 |
| Final sistem senaryoları | 7 |
| Göreli Markdown bağlantısı | 402 |

GDB kontrolleri, geçerli SG girişine dönüşü ve SG+4 adresinin reddini kapsar. Kapsam QEMU simülasyonudur. Sensör girdileri sentetiktir. Fiziksel kart, deadline ve WCET doğrulaması kapsam dışıdır.

[Sürüm notları](https://github.com/Merd0/ARM-Cortex-M-Lab/releases/tag/v1.0.0) · [CI tanımı](.github/workflows/ci.yml) · [Yerel mühendislik denetimi](docs/ENGINEERING_AUDIT.md)

## Ne öğretiyor?

| Alan | Konular |
|---|---|
| **Mimari** | Programmer model, register'lar, MSP/PSP, Thread/Handler Mode, privilege, CONTROL |
| **Koruma** | MPU, SVC, privileged/unprivileged ayrımı, TrustZone |
| **Exception** | Vector table, NVIC, hardware stacking, EXC_RETURN, fault'lar ve escalation |
| **Build** | Compiler, optimizasyon, AAPCS, linker script, ELF, map file, startup |
| **Debug** | GDB ile teşhis akışı, disassembly, watchpoint, fault analizi |
| **Gerçek zamanlı** | SysTick, PendSV, context switching, eğitim amaçlı mini kernel |

---

## Başlarken

### 1. Ortamı kontrol et

```powershell
.\scripts\check-environment.ps1     # Windows
```

```bash
./scripts/check-environment.sh      # Linux / macOS / WSL
```

Eksik varsa: [`docs/ENVIRONMENT.md`](docs/ENVIRONMENT.md)

Gerekenler: Arm GNU Toolchain (`arm-none-eabi`), CMake, Ninja, QEMU, GDB.

### 2. Build et

```bash
cmake --preset debug
cmake --build --preset debug
```

### 3. Çalıştır

```bash
./scripts/run-qemu.sh build/debug/lessons/02-programmer-model/02-programmer-model.elf
```

Çıkış: `Ctrl-A` sonra `X`.

### 4. Debug et

```bash
# Terminal 1
./scripts/debug-qemu.sh build/debug/lessons/02-programmer-model/02-programmer-model.elf

# Terminal 2
arm-none-eabi-gdb -x scripts/gdb/cortex-m.gdb \
    build/debug/lessons/02-programmer-model/02-programmer-model.elf
(gdb) cm-connect
(gdb) cm-regs
```

### 5. Nereden başlamalı?

**[`BOOTCAMP.md`](BOOTCAMP.md)** — günlük çalışma planı.
İlk ders: [`lessons/01-cortex-m-family/`](lessons/01-cortex-m-family/) — kod
yazmadan, hedefin kendini nasıl tanıttığını okuyarak başlar.
Ardından [`lessons/02-programmer-model/`](lessons/02-programmer-model/).

---

## Target

| | |
|---|---|
| İşlemci | Cortex-M33 (Armv8-M Mainline, TrustZone + DSP + FPU) |
| Platform | QEMU `mps2-an505` (Arm Application Note AN505 / IoT Kit) |
| Kod bölgesi | `0x10000000` (Secure alias), 512 KB |
| RAM | `0x30000000` (Secure alias), 32 KB |
| Konsol | CMSDK APB UART0 @ `0x50200000` |
| Doğrulanan MPU | 8 bölge (ölçüldü) |
| Doğrulanan SAU | 8 bölge (ölçüldü) |

**Simülatör ile gerçek donanımın davranışları farklı olabilir.** Bu hedefte
ölçülen özellikler ve sınırlar aşağıdaki belgelerde yer alıyor:

- [`docs/TARGET_CAPABILITIES.md`](docs/TARGET_CAPABILITIES.md) — ölçülmüş capability matrisi
- [`docs/LIMITATIONS.md`](docs/LIMITATIONS.md) — simülatör ile donanım farkları

En önemli sınır: **DWT cycle counter bu target'ta saymıyor** (ölçüldü).
Profiling dersi bu nedenle alternatif ölçüm yöntemlerini kullanır.

---

## Beş entegrasyon projesi

| Proje | Birleştirdiği mekanizmalar |
|---|---|
| [Protected Application](projects/protected-application/) | PSP, ayrıcalıksız Thread Mode, MPU, SVC ve fault handling |
| [Fault Analyzer](projects/fault-analyzer/) | Exception frame çözümleme ve CFSR/HFSR analizi |
| [From Reset to Main](projects/from-reset-to-main/) | Vector table, startup ve linker script |
| [Performance Profiler](projects/performance-profiler/) | Ölçüm metodolojisi ve derleyici optimizasyonu |
| [Educational Mini Kernel](projects/mini-kernel/) | SysTick, PendSV, SVC ve context switch |

[Lesson 15](lessons/15-mini-kernel/) context switch mekanizmasını ders düzeyinde inceler. Educational Mini Kernel, bu mekanizmayı SysTick ve SVC ile birleştiren uygulamadır.

[Teslim geçmişi](docs/PROGRESS.md) · [Yol haritası](docs/ROADMAP.md)

---

## Repo yapısı

```text
lessons/        16 ders — her biri kendi başına anlaşılır
projects/       birden fazla dersi birleştiren projeler
final-project/  specialization sonu entegrasyon
external/       vendored CMSIS-Core (Arm, Apache-2.0) — baseline core API
core/           cm33_teaching.h — eğitim amaçlı minimal register tanımları
platform/       board'a özel kod: startup, UART, linker script
common/         console ve minimal libc parçaları
tools/          capability probe
cmake/          toolchain dosyası ve build yardımcıları
scripts/        environment check, QEMU, GDB
docs/           kalıcı referans belgeler
tests/          smoke test
.github/        CI workflow
.vscode/        build/run/debug task ve launch tanımları
```

---

## Dokümantasyon

| Belge | İçerik |
|---|---|
| [BOOTCAMP.md](BOOTCAMP.md) | 7 günlük çalışma planı ve kontrol listesi |
| [docs/ENVIRONMENT.md](docs/ENVIRONMENT.md) | Sıfırdan Windows kurulumu |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Repo mimarisi ve tasarım gerekçeleri |
| [docs/MEMORY_MAP.md](docs/MEMORY_MAP.md) | Doğrulanmış bellek haritası |
| [docs/TARGET_CAPABILITIES.md](docs/TARGET_CAPABILITIES.md) | Ölçülmüş capability matrisi |
| [docs/DEBUGGING.md](docs/DEBUGGING.md) | Cortex-M teşhis akışları |
| [docs/LIMITATIONS.md](docs/LIMITATIONS.md) | Simülatör sınırları |
| [docs/PERFORMANCE_MEASUREMENT.md](docs/PERFORMANCE_MEASUREMENT.md) | Ölçüm metodolojisi |
| [docs/GLOSSARY.md](docs/GLOSSARY.md) | İngilizce terim + kısa Türkçe karşılık |
| [docs/COURSE_MAP.md](docs/COURSE_MAP.md) | Kurs konuları ↔ ders eşlemesi |
| [docs/ROADMAP.md](docs/ROADMAP.md) | Uzun vadeli plan |
| [docs/REAL_HARDWARE_MIGRATION.md](docs/REAL_HARDWARE_MIGRATION.md) | Board alınınca ne değişir |
| [docs/REFERENCES.md](docs/REFERENCES.md) | Resmi kaynaklar |
| [docs/TOOL_VERSIONS.md](docs/TOOL_VERSIONS.md) | Doğrulanan araç sürümleri |
| [docs/PROGRESS.md](docs/PROGRESS.md) | İlerleme tablosu |
| [external/README.md](external/README.md) | Vendored CMSIS-Core: sürüm ve güncelleme |
| [projects/README.md](projects/README.md) | Birleştirici projeler |
| [tests/README.md](tests/README.md) | Smoke test |

---

## Dil politikası

Kod, tanımlayıcılar ve dosya isimleri **İngilizce**. Teknik anlatım ağırlıklı
**Türkçe**, teknik terimler İngilizce korunur ve ilk kullanımda kısa Türkçe
karşılığı verilir:

> `exception entry (istisnaya giriş)`, `callee-saved register (çağrılan
> fonksiyonun koruması gereken register)`

Amaç: hem konuyu hem de resmi dokümantasyonu okumak için gereken
İngilizce terminolojiyi öğrenmek.

---

## Özgünlük

Açıklamalar, diyagramlar, kod ve deneyler bu repo için hazırlanmıştır.
Teknik açıklamalar resmi Arm / CMSIS / QEMU dokümantasyonuyla doğrulanır;
ölçülebilen davranışlar deneylerle kontrol edilir
([`docs/REFERENCES.md`](docs/REFERENCES.md)).

Çalıştırılmamış bir kodun çıktısı "gerçek çıktı" olarak sunulmaz.
Doğrulanmamış bilgi `OLCULMEDI` olarak işaretlenir.

---

## Lisans

MIT — bkz. [LICENSE](LICENSE).

`external/CMSIS/` altındaki dosyalar Arm'a aittir ve Apache-2.0 ile
lisanslıdır; kendi lisans metni korunur ([external/CMSIS/LICENSE](external/CMSIS/LICENSE)).
