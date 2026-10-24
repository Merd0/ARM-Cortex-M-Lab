# Progress

Bu bölüm mevcut ilerlemeyi gösterir. Yalnızca tamamlanan ve doğrulanan
çalışmalar işaretlenir. Bir dersin öğrenci tarafından tamamlanması için:

1. Firmware build edildi
2. QEMU'da çalıştırıldı ve çıktısı README'ye yazıldı
3. GDB gözlemi yapıldı
4. Experiment ve Break It denendi
5. "What I Learned" dolduruldu

"What I Learned" öğrencinin kişisel kaydıdır; teknik teslim adına doldurulmaz.
2026-10-23: ders 01–11 main üzerinde, ders 12 bu dalda yerel doğrulandı;
uzak CI/PR sonucu henüz yok. Aşama planı [Roadmap](ROADMAP.md) içinde.

## Altyapı

| Öğe | Durum |
|---|---|
| CMake + Ninja + arm-none-eabi build sistemi | ✅ |
| CMSIS-Core 6.3 entegrasyonu (baseline core API) | ✅ (clean build + QEMU + GDB ile doğrulandı) |
| Eğitim header'ı `cm33_teaching.h` (lesson'lar için) | ✅ |
| Markdown link kontrolü (`tests/check-links.py`) | ✅ son çalıştırma sayısı aşağıda |
| CMake presets (debug/release/minsize) | ✅ |
| MPS2-AN505 platform portu (startup, linker, UART) | ✅ |
| QEMU'da çalışan firmware | ✅ (doğrulandı) |
| GDB bağlantısı + breakpoint | ✅ (doğrulandı) |
| Cortex-M GDB yardımcı komutları | ✅ (doğrulandı) |
| Environment check scriptleri | ✅ bash doğrulandı / ⬜ PowerShell Windows'ta test edilmeli |
| Target capability probe | ✅ (ölçüldü) |
| Dokümantasyon temeli | ✅ |
| Smoke test (`tests/smoke-test.sh`) | ✅ davranışsal regresyon; güncel sonuç aşağıda |
| CI (`.github/workflows/ci.yml`) | ✅ önceki PR #7 üç preset PASS; bu dalın CI değişikliği henüz uzakta çalışmadı |
| VS Code tasks + launch | ✅ yazıldı / ⬜ Windows'ta denenmedi |
| Proje ve final-project tanımları | ✅ README'ler yazıldı / ⬜ kod yok (planlı) |

## Dersler

| # | Ders | Anlatım | Kod | Çalıştırıldı | GDB | What I Learned |
|---|---|---|---|---|---|---|
| 01 | Cortex-M Family | ✅ | — | ✅ (probe) | ✅ | ⬜ |
| 02 | Programmer Model | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 03 | C and Assembly | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 04 | Stack, MSP & PSP | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 05 | Privilege & SVC | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 06 | MPU | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 07 | Exceptions & NVIC | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 08 | Faults | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 09 | Compiler & AAPCS | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 10 | Linker, ELF & Memory | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 11 | Startup (Reset → main) | ✅ | ✅ | ✅ | ✅ | ⬜ |
| 12 | Debug & DWT | ✅ | ✅ | ✅ yerel | ✅ üç preset | ⬜ |
| 13 | Sync, FPU & DSP | ⬜ scaffold | ⬜ | ⬜ | ⬜ | ⬜ |
| 14 | SysTick & PendSV | ⬜ scaffold | ⬜ | ⬜ | ⬜ | ⬜ |
| 15 | Mini Kernel | ⬜ scaffold | ⬜ | ⬜ | ⬜ | ⬜ |
| 16 | TrustZone & Advanced | ⬜ scaffold | ⬜ | ⬜ | ⬜ | ⬜ |

## Doğrulama kaydı

Önceki bağımsız audit düzeltmeleri Git geçmişinde korunuyor. Başlangıç
regresyonu 2026-10-23 Windows üzerinde yeniden geçti: üç preset, 48 mevcut
ELF/preset, smoke 369/0 ve probes 47/0.

Lesson 12 teslimi, 2026-10-23 Windows / GCC 15.2.1 / QEMU 11.1.0:

| Kontrol | Sonuc |
|---|---|
| Temiz build, uc preset | ✅ 55 ELF / preset, 0 uyarı / 0 hata |
| `tests/smoke-test.sh` | ✅ 454 PASS / 0 FAIL, üç preset |
| `tests/build-probes.sh` | ✅ 47 PASS / 0 FAIL, uc preset |
| `tests/check-links.py` | ✅ 337 link, 0 kırık |
| `tests/lesson12-check.py` | ✅ üç preset; write/read watchpoint, backtrace, single-step, stopped-timer ve I/O negatif kontrolleri, header/optimizasyon karşılaştırması |
| Lesson 04 PSP cozumu (debug/release/minsize) | ✅ SPSEL + REGION + GUARD hepsi PASS |
| GDB frame cozumu (MSP/PSP kaynakli + reddedilen vakalar) | ✅ |

## Exercise / Solution / Answers

| Ders | Exercise | Solution | Cevap anahtarı |
|---|---|---|---|
| 01 | senaryo yazımı (kod yok) | — | `lessons/01-cortex-m-family/answers/reference-answers.md` |
| 02 | `exercises/01-decode-apsr-flags/` | `solutions/01-decode-apsr-flags/` (çalıştırıldı ✅) | `lessons/02-programmer-model/answers/quick-check.md` |
| 12 | `exercises/01-valid-measurement/` (0/12, eksik) | `solutions/01-valid-measurement/` (12/12 PASS) | `lessons/12-debug-dwt/answers/quick-check.md` |
| 11 | `exercises/01-reset-sequence/` | `solutions/01-reset-sequence/` (çalıştırıldı ✅) | `lessons/11-startup/answers/quick-check.md` |
| 10 | `exercises/01-locate-your-data/` | `solutions/01-locate-your-data/` (çalıştırıldı ✅) | `lessons/10-linker-elf-memory/answers/quick-check.md` |
| 09 | `exercises/01-callee-save/` | `solutions/01-callee-save/` (çalıştırıldı ✅) | `lessons/09-compiler-aapcs/answers/quick-check.md` |
| 08 | `exercises/01-fault-report/` | `solutions/01-fault-report/` (çalıştırıldı ✅) | `lessons/08-faults/answers/quick-check.md` |
| 07 | `exercises/01-decode-exception-entry/` | `solutions/01-decode-exception-entry/` (çalıştırıldı ✅) | `lessons/07-exceptions/answers/quick-check.md` |
| 06 | `exercises/01-thread-mpu-policy/` | `solutions/01-thread-mpu-policy/` (çalıştırıldı ✅) | `lessons/06-mpu/answers/quick-check.md` |
| 05 | `exercises/01-add-svc-service/` | `solutions/01-add-svc-service/` (çalıştırıldı ✅) | `lessons/05-privilege-svc/answers/quick-check.md` |
| 04 | `exercises/01-activate-psp/` (+ opsiyonel DEEP) | `solutions/01-activate-psp/` (çalıştırıldı ✅) | `lessons/04-stack-msp-psp/answers/quick-check.md` |
| 03 | `exercises/01-sub-numbers/` | `solutions/01-sub-numbers/`, `solutions/02-multiply-add/` (ikisi de çalıştırıldı ✅) | `lessons/03-c-and-assembly/answers/quick-check.md` |

## Projeler

| Proje | Durum |
|---|---|
| Protected Application | ⬜ planlandı |
| Fault Analyzer | ⬜ planlandı |
| From Reset to Main | ⬜ planlandı |
| Performance Profiler | ⬜ planlandı |
| Mini Kernel | ⬜ planlandı |
| Final Project | ⬜ planlandı |

## Öğrenme hedefi kontrol listesi

Proje sonunda tamamlanması hedeflenen çalışmalar:

- [ ] Cortex-M register modelini debugger'da inceledim
- [ ] C kodunun assembly karşılığını takip ettim
- [ ] AAPCS calling convention kurallarını uyguladım
- [ ] MSP ve PSP arasında geçiş yaptım
- [ ] Thread Mode ve Handler Mode davranışını gözlemledim
- [ ] Privileged ve unprivileged execution kullandım
- [ ] SVC ile privileged service çağırdım
- [ ] MPU ile memory protection oluşturdum
- [ ] MemManage, UsageFault, BusFault ve HardFault davranışlarını inceledim
- [ ] Exception sırasında oluşan stack frame'i decode ettim
- [ ] Kendi linker/memory layout deneylerimi yaptım
- [ ] ELF ve map file analiz ettim
- [ ] Reset'ten main'e kadar startup akışını uyguladım
- [ ] GDB ile Cortex-M debug ettim
- [ ] DWT ile profiling denedim (ve bu target'taki sınırını belgeledim)
- [ ] SysTick kullandım
- [ ] PendSV kullanarak context switch yaptım
- [ ] Küçük educational scheduler/kernel geliştirdim
- [ ] TrustZone Secure/Non-secure ayrımını uyguladım
- [ ] DSP/FPU/MVE gibi ileri özellikleri deneysel olarak inceledim

## Güncelleme

Bir ders bittiğinde: bu tabloyu, `README.md` progress bölümünü ve
`BOOTCAMP.md` kutularını birlikte güncelle.
