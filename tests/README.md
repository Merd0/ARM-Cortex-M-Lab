# tests/

Bu dizin **unit test** için değil. Gömülü tarafta unit test ayrı bir konudur
(host üzerinde derleyip çalıştırmak gerekir) ve bu repoda kapsam dışıdır.

Burada firmware davranışı, link sözleşmeleri, debugger gözlemleri ve belge bağlantıları sınanır:

```bash
./tests/smoke-test.sh          # davranışsal regresyon (preset: debug)
./tests/build-probes.sh        # link/linker sözleşme probe'ları
python3 tests/check-links.py   # Markdown göreli link kontrolü
python3 tests/lesson12-check.py debug  # GDB / ölçüm sınırları
python3 tests/lesson13-check.py debug  # synchronization / FP frame / ABI
```

Hepsi üç preset için CI'da çalışır (`.github/workflows/ci.yml`).

## `smoke-test.sh` — hedef sınıflandırması

Bağımsız bir denetim, eski testin ciddi bir hatayı kaçırdığını gösterdi:
`-O2`/`-Os` altında Lesson 04 PSP çözümü **CHECK_REGION FAIL** veriyordu ama
test geçiyordu, çünkü yalnızca metin varlığına bakıyordu. Artık her hedef
türüne göre beklenen **davranış** doğrulanıyor:

| Hedef türü | Beklenen |
|---|---|
| Normal örnek | Doğru sonuç + tamamlanma işareti, hiç `FAIL` yok |
| Referans çözüm | Tüm `CHECK_*` satırları `PASS` |
| Bilerek bozuk | **Beyan edilen** yanlış sonuç/FAIL ya da fault; END sözleşmeye bağlı |
| Bitmemiş ödev | Eksik davranış; çözülmüş gibi raporlanmamalı |

`===== END =====` görünmesi **tek başına** hiçbir yerde başarı sayılmaz.

Kapsam: capability-probe (hedef kimliği, MPU/SAU bölgeleri, NVIC öncelik
biti uyumu), Lesson 02 (aritmetik + bayraklar + kasıtlı fault), Lesson 03
(ABI ihlalinin beyan edilen yanlış sonucu), Lesson 04 (üç aşama, push/pop
delta'sı, stack guard, PSP çözümü).

Lesson 12 main kapanış kaydı: **454 PASS / 0 FAIL**, üç preset'te de
(GitHub Actions, Ubuntu 24.04 / QEMU 8.2.2). Lesson 13'ün yeni hedefleri de
eklenmiştir; güncel yerel sonuçlar [Progress](../docs/PROGRESS.md) içindedir.

## `build-probes.sh` — link sözleşmeleri

İki denetim bulgusunu kalıcı olarak yakalar:

- **I4 — libgcc sıralaması:** `-lgcc` nesne dosyalarından önce gelirse
  `__aeabi_uldivmod` çözümlenmez. Probe hem yanlış sıranın başarısız hem
  doğru sıranın başarılı olduğunu, hem de repo'nun gerçek link komutunda
  kütüphanenin nesnelerden **sonra** durduğunu kontrol eder.
- **I5 — FLASH yük görüntüsü sınırı:** `.text` + `.rodata` FLASH'a sığarken
  `.data`'nın FLASH kopyası eklenince taşan bir imaj **link hatası**
  vermeli. Ayrıca sınır içindeki bir imajın `.data`'sının çalışma zamanında
  gerçekten RAM'e ulaştığı ve `.bss`'in sıfırlandığı QEMU'da doğrulanır.

Main kapanış kaydı: **47 PASS / 0 FAIL**, üç preset. Lesson 10/11
linker/startup probe'ları da bu script'tedir; önceki kontroller azaltılmaz.

## Lesson-local debugger ve ABI kontrolleri

`lesson12-check.py`: gerçek write/read watchpoint, backtrace, single-step,
ölçüm penceresinde I/O reddi, durdurulan timer ve statik optimizasyon/header
karşılaştırması. `lesson13-check.py`: kontrollü SysTick, R4–R11/SP preservation,
basic/eager/lazy FP frame, NOCP fault PC, lazy slot'un materialize edilmesi,
S0–S2 restore, çağıranın FPSCR kontrol durumunun korunması, matching/mixed float ABI link probe'ları ve emitted DSP/FP.

İkisi de stdlib Python ve Arm GDB (`arm-none-eabi-gdb` veya `gdb-multiarch`)
kullanır; GDB Python eklentisi gerekmez. Eksik araç SKIP gibi sunulmaz.
Lesson 13'ün negatif hedefleri retry eksikliğini ve kapalı FPU iznini,
link probe'ları ise iki yöndeki softfp/hard karışımını yakalar. Geçici
olarak derlenen iki yanlış kritik-bölüm çözümü de korumasız güncelleme ve
koşulsuz IRQ açmanın doğru kontrol tarafından reddedildiğini doğrular.

Ham kanıtlar ignored `build/<preset>/lesson12-evidence/` ve
`build/<preset>/lesson13-evidence/` altında kalır; workflow bunları upload eder.
GDB'nin QEMU watchpoint'i fiziksel DWT comparator/cycle desteği sayılmaz.

## Neden bu kadar az

Bilerek. Derslerin asıl doğrulaması GDB ile **elle** yapılır — çünkü öğrenme
hedefi budur. Otomatik test, altyapının bozulmadığını garantilemek için var;
öğrenmenin yerine geçmek için değil.

Firmware'ler sonsuz döngüyle bittiği için QEMU `timeout` ile kesilir
(`QEMU_TIMEOUT`, varsayılan 8 s). Test yalnızca kendi başlattığı süreçleri
sonlandırır.

---

[← Ana sayfa](../README.md)

## Lessons 14–16 — hızlı geliştirme kapısı

```bash
cmake --preset debug
cmake --build --preset debug
python tests/development-smoke.py all
# Tek ders: python tests/development-smoke.py 15
```

Stdlib Python; her imajın END işareti, CHECK anahtarlarının tam kümesi ve
beklenen PASS/FAIL sonuçları kontrol edilir. 12 senaryo: her derste örnek,
kasıtlı hata, eksik starter ve ayrı çözüm. Lesson 16 her seferinde Secure
kernel ELF + Non-secure loader ELF birlikte yükler; tek imaj yeterli değildir.

PendSV sırası/pending birleşmesi, gerçek timer preemption, R4–R11 canary'leri,
stack guard'ları, ilk frame reddi, NSC dönüşleri, AUVIOL ve INVEP temel kapsamdır.
SFAR yalnız SFARVALID set ise adres olarak kullanılır. Ham kayıtlar ignored
`build/<preset>/development-evidence/` altında tutulur. Süreçler yalnız testin
başlattığı QEMU'ya ait PID üzerinden kapatılır.

Kapı üç CI preset'ine bağlıdır; main 11fde900 için üç preset CI geçti.
`--preset release` / `--preset minsize` aynı kapıyı diğer optimizasyonlarda çalıştırır. Mevcut smoke, build-probes ve GDB script'leri
azaltılmadı. Bu hızlı test tam mimari/security audit veya donanım testi değildir.

## Projeler ve final entegrasyon kapısı

```bash
python tests/system-smoke.py --preset debug
python tests/integration-check.py --preset debug
python tests/profile-report.py --preset debug
```

Aynı komutlar release/minsize için de çalıştırıldı. System gate: 15 proje +
7 final senaryosu. GDB gate: 13 oturum (frame sınırları ve üç SG giriş/ret testi dahil); R4–R11 save/restore ve EXC_RETURN,
PSP/PSPLIM/MSP alignment, FP payload/control, MPU/STKOF ve Secure geçişleri.
Kontrollü NS SysTick, Secure Thread devam noktasının sahibi değiştirilmeden
korunduğunu sınar; GDB tail-chaining'i anında Thread'e dönüş sanmaz.

Diagnostic API privilege testi ve Thread-origin FP servis reddi beklenen
politikayı ölçer. Queue tamamlanma kontrolü örnek sayısını azaltarak geçilmez.
CI adımları her üç preset'e bağlıdır. Denetim düzeltmelerinin
yerel sonuçları PR/main CI başarısından ayrı tutulur; [denetim kaydı](../docs/ENGINEERING_AUDIT.md).
Yerel kapılar üçüncü taraf bağımsız audit veya kart testi değildir.

Frame regresyonları: 8-byte M33 hizası, padding sınırı, Thread/Handler IPSR
uyumu, T/PC ve yedi stacking/unstack/lazy hata biti. Geçerli tamponla hata
bitleri sınanır; yalnız geçersiz pointer kullanmak bounds kontrolü yüzünden
hatalı status kontrolünü gizler. STKERR veto mutasyonu yeni testi FAIL yapar.
