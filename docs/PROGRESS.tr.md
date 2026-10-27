**Dil:** [English](PROGRESS.md) | Türkçe

# Progress

Bu bölüm mevcut ilerlemeyi gösterir. Yalnızca tamamlanan ve doğrulanan
çalışmalar işaretlenir. Bir dersin öğrenci tarafından tamamlanması için:

1. Firmware build edildi
2. QEMU'da çalıştırıldı ve çıktısı README'ye yazıldı
3. GDB gözlemi yapıldı
4. Experiment ve Break It denendi
5. "What I Learned" dolduruldu

"What I Learned" öğrencinin kişisel kaydıdır; teknik teslim adına doldurulmaz.
2026-10-24 teknik teslim: 16 ders, beş proje ve final sistem uygulandı.
Ders 13 [PR #9](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/9) ile merge edildi;
[run 40079336979](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40079336979)
debug/release/minsize PASS. Merge commit'i
`161d93d94b2aad62f5a31b3454c9d61f76b561d1`.
Lesson 14–16, [PR #10](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/10) ile
üç preset CI sonrası merge edildi
([run 40101724183](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40101724183));
merge commit'i `669257b11a672bbcd5abf718e0360b891c5e9547`.
Proje/final çalışması [PR #11](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/11)
ile merge edildi: `11fde900835d4443a55d0e7da64438ea4da4c690`.
[Main CI 40105930630](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40105930630)
debug/release/minsize PASS. 2026-10-27 iç mühendislik denetiminin teknik kaydı
[ayrı raporda](ENGINEERING_AUDIT.tr.md), yayın/CI/merge durumu
[PR #12](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/12) üzerinde izlenir. Yerel test/teknik inceleme ve CI, bağımsız
üçüncü taraf audit veya donanım sertifikası değildir. Öğrencinin kişisel
tamamlama kriterleri bundan ayrı kalır. Aşama planı [Roadmap](ROADMAP.tr.md) içinde.

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
| Environment check scriptleri | ✅ Bash ve PowerShell environment check doğrulandı (2026-10-27) |
| Target capability probe | ✅ (ölçüldü) |
| Dokümantasyon temeli | ✅ |
| Smoke test (`tests/smoke-test.sh`) | ✅ davranışsal regresyon; güncel sonuç aşağıda |
| CI (`.github/workflows/ci.yml`) | ✅ üç preset; build/regresyon/debugger/proje/final kapıları; uzak sonuçlar yukarıdaki Actions bağlantısında |
| VS Code tasks + launch | ✅ yazıldı / ⬜ Windows'ta denenmedi |
| Proje ve final-project tanımları | ✅ main üzerinde; üç-preset CI PASS (11fde900) |

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
| 12 | Debug & DWT | ✅ | ✅ | ✅ main/CI | ✅ üç preset | ⬜ |
| 13 | Sync, FPU & DSP | ✅ | ✅ | ✅ PR #9 CI | ✅ üç preset | ⬜ |
| 14 | SysTick & PendSV | ✅ | ✅ | ✅ üç preset | ✅ üç preset | ⬜ |
| 15 | Mini Kernel | ✅ | ✅ | ✅ üç preset | ✅ üç preset | ⬜ |
| 16 | TrustZone & Advanced | ✅ | ✅ | ✅ üç preset | ✅ ortak boot/final; ayrı standalone GDB iddiası yok | ⬜ |

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

## Tarihsel teslim kayıtları

Aşağıdaki tarihli kayıtlar teslim anını anlatır; güncel yayın durumu yukarıdadır.

## Lesson 13 yerel teslim — 2026-10-24

Windows / GCC 15.2.1 / QEMU 11.1.0, debug/release/minsize:

- Temiz build: **64 ELF/preset**, 0 uyarı / 0 hata.
- Tam smoke/regresyon: **566 PASS / 0 FAIL**; önceki 454 kontrol korundu.
- Mevcut build probe'ları: **47 PASS / 0 FAIL / 0 SKIP**.
- `lesson12-check.py`: üç preset tekrar PASS.
- `lesson13-check.py`: dokuz hedef, altı GDB senaryosu/preset;
  R4–R11/SP, NOCP PC, eager/lazy raw frame, S0–S2 ve FPSCR kontrol durumu,
  DSP/ABI metadata kontrolleri PASS. İki mixed ABI link probe'u ve iki
  yanlış kritik-bölüm çözümü beklenen şekilde reddedildi.
- Göreli link kontrolü: **358 link, 0 kırık**; `git diff --check` PASS.

FP worker'ın gereksiz FPSCR sıfırlaması son AAPCS incelemesinde kaldırıldı.
Çağıranın açık ortam destek fonksiyonuyla seçtiği FZ=1 korunuyor; kaydedilen
FPSCR ve çağrı sonrası değer GDB'de karşılaştırılıyor. Bu kontrol koruması,
denormal aritmetiğin veya donanım performansının doğrulaması değildir.

Lesson 13 daha sonra PR #9'a push edildi; üç uzak CI işi başarıyla tamamlandı.
Ardından kullanıcı onayıyla PR #9 merge edildi. Bağımsız audit veya donanım
doğrulaması tamamlandı denmiyor.

### Lessons 14–16 — hızlı geliştirme kapısı, 2026-10-24

Windows / GCC 15.2.1 / QEMU 11.1.0, yerel dal:
- Debug bütün repository build'i PASS; toplam 80 ELF (16 yeni ELF).
- `python tests/development-smoke.py all`: 12 QEMU senaryosunun tamamı
  beklenen sonuçta; CHECK alanlarının tam kümesi ve beklenen FAIL'ler doğrulanır.
- 14: SysTick/PendSV sırası 1,2,3; yüksek öncelikli PendSV 1,3,2;
  maskeli çift pending yazması tek dispatch; timer dört tick.
- 15: iki task PSP üzerinde, gerçek timer preemption, R4–R11 canary'leri,
  stack guard'ları, SVC yield; eksik frame güvenli ret, bozuk save beklenen FAIL.
- 16: dört ayrı Secure/NS imaj çifti; geçerli scalar NSC çağrıları,
  sınır dışı girdilerin reddi, Secure okuma AUVIOL=0x08 ve SG atlama INVEP=0x01.
  SFARVALID temiz olduğunda SFAR adres diye yorumlanmaz.
- Seçili eski Debug senaryoları yeniden çalıştırıldı: Lesson 04 PSP çözümü,
  Lesson 08 BusFault snapshot/clear, Lesson 11 startup ve Lesson 13 lazy FP PASS.
  Bu, eski geniş smoke setinin tamamının yeniden çalıştırılması değildir.
- Yeni hızlı kapı CI workflow'una eklendi; bu ilk yerel kapı sırasında
  dal henüz push edilmemişti. Güncel yayın kaydı belgenin başındadır.
- Geniş smoke/probe paketi, kapsamlı GDB, Release/MinSize karşılaştırmaları,
  geniş negatif testler ve bağımsız mimari audit bu aşamada çalıştırılmadı.
  Toplu kalite kapısı bunları kapsayacak; projeler/final entegrasyonu başlamadı.

## Exercise / Solution / Answers

| Ders | Exercise | Solution | Cevap anahtarı |
|---|---|---|---|
| 01 | senaryo yazımı (kod yok) | — | `lessons/01-cortex-m-family/answers/reference-answers.md` |
| 02 | `exercises/01-decode-apsr-flags/` | `solutions/01-decode-apsr-flags/` (çalıştırıldı ✅) | `lessons/02-programmer-model/answers/quick-check.md` |
| 14 | `exercises/01-defer-work/` (öncelik eksik) | `solutions/01-defer-work/` (Debug PASS) | `lessons/14-systick-pendsv/answers/quick-check.md` |
| 15 | `exercises/01-initial-frame/` (frame eksik, launch yok) | `solutions/01-initial-frame/` (Debug PASS) | `lessons/15-mini-kernel/answers/quick-check.md` |
| 16 | `exercises/01-secure-service/` (fail-closed) | `solutions/01-secure-service/` (Debug PASS) | `lessons/16-trustzone-advanced/answers/quick-check.md` |
| 13 | `exercises/01-critical-update/` (2/6, eksik) | `solutions/01-critical-update/` (6/6 PASS) | `lessons/13-sync-fpu-dsp/answers/quick-check.md` |
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
| Protected Application | ✅ main üç preset CI PASS (11fde900) |
| Fault Analyzer | ✅ main üç preset CI PASS (11fde900) |
| From Reset to Main | ✅ main üç preset CI PASS (11fde900) |
| Performance Profiler | ✅ main üç preset CI PASS (11fde900) |
| Mini Kernel | ✅ main üç preset CI PASS (11fde900) |
| Final Project | ✅ main üç preset CI PASS (11fde900) |

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

## Son toplu yerel teslim — 2026-10-24

- Debug/Release/MinSize: clean build, her birinde 0 warning / 0 error.
- Mevcut geniş smoke: **566 PASS / 0 FAIL**, her preset.
- Build probes: **47 PASS / 0 FAIL**, her preset. Eski libgcc/FLASH LMA,
  linker/startup negatif kontrolleri azaltılmadı.
- Lessons 14–16: **12 QEMU senaryosu**, beklenen starter/bozuk FAIL'leriyle PASS.
- Beş proje: **15 bağımsız QEMU senaryosu**, her preset PASS.
- Final: **7 Secure/NS imaj çifti**, her preset PASS. 32 örnek, toplam 298;
  dört telemetry kaydı ayrı integer oracle ile karşılaştırıldı.
- Lesson 12 ve 13 debugger/ABI kapıları: üç preset PASS.
- integration-check.py: her preset **9 GDB oturumu**. Deferred PendSV;
  Secure/NS R4–R11 save/restore, MSP public hizası ve PSP/PSPLIM;
  Secure DSP/FP sonucu, FPSCR ve FPCA/SFPA restore, S0/S1 temizliği;
  MPU/cross-task/STKOF/SecureFault ve Secure-call defer/continuation doğrulandı.
- Statik profiler: beş gerçek optimizasyon varyantı; ölçüm cycle/throughput/WCET
  olarak sunulmaz. Son rapor build/<preset>/system-evidence/profile.json.
- Ortak platform, CMSIS, debugger helper'ları ve Lessons 01–13 kaynakları,
  Lesson 13 PR başından itibaren değiştirilmedi.

Kapıda düzeltilenler: Lesson 14'ün optimizasyona bağlı kısa polling bütçesi
yerine SEV/WFE; queue tamamlanmadan kernel projesinin kapanmaması;
task'tan bootstrap isteğinin reddi; diagnostic privilege politikası;
Secure Thread kesilirken context değiştirmeden dönüş; sınırlı FP servisi için
Handler-only politika ve açık FP payload/control temizliği.

Sonuçlar yerel GCC 15.2.1 / QEMU 11.1.0 / Arm GDB 16.3.90 ortamındadır.
Bu bölüm yerel kapının kaydıdır; GitHub Ubuntu/QEMU 8.2.2 proje/final CI
sonuçları ilgili PR/commit'in Actions kaydından ayrı izlenir.
PR #9'un eski üç-preset CI başarısı yeni dalın CI sonucu diye sunulmaz.
Yeni tag/Release oluşturulmadı. Bağımsız üçüncü taraf audit,
gerçek kart ve genel Secure/FP task switching doğrulaması yapılmadı.
