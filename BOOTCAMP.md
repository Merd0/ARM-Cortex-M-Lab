# ARM Cortex-M — 7 Day Practical Bootcamp

Bu dosya günlük ders planını ve çalışma kontrol listesini içerir.

**Etiketler:** `[QUICK]` 20–35 dk · `[STANDARD]` 45–75 dk · `[DEEP]` 90 dk+
Zamanın daralırsa `QUICK` ve `STANDARD` üzerinden ilerle, `DEEP` olanları
DEEP TRACK'e bırak.

**Durum:** `[ ]` yapılmadı · `[~]` devam ediyor · `[x]` bitti

Bu kutular öğrencinin kişisel çalışma kaydıdır; repository geliştirilince
otomatik işaretlenmez. Repository teknik teslimi: 16 ders, beş proje ve final
sensor node uygulandı; yerel üç-preset/QEMU ve kritik GDB kapıları geçti.
Ders 13 PR #9, Lesson 14–16 PR #10 ile üç preset CI sonrası merge edildi.
Proje/final PR'ı için de merge koşulu üç preset CI başarısıdır. Güncel
yayımlama ve uzak doğrulama bağlantıları [Progress](docs/PROGRESS.md) içindedir.
Öğrenci çalışma kutuları teknik teslim adına otomatik doldurulmaz. Güncel teknik durum: [Progress](docs/PROGRESS.md).

---

## Day 0 — Kurulum (bir kez)

- [ ] `[QUICK]` Ortam kontrolü — `./scripts/check-environment.sh` veya `.\scripts\check-environment.ps1`
- [ ] `[QUICK]` İlk build — `cmake --preset debug && cmake --build --preset debug`
- [ ] `[QUICK]` İlk çalıştırma — `./scripts/run-qemu.sh build/debug/tools/capability-probe/capability-probe.elf`
- [ ] `[QUICK]` İlk GDB bağlantısı — [`docs/DEBUGGING.md`](docs/DEBUGGING.md)
- [ ] `[QUICK]` Smoke test — `./tests/smoke-test.sh` (normal/bozuk/eksik hedeflerin beklenen davranışları doğrulanmalı)
- [ ] `[QUICK]` Kendi tool sürümlerini kaydet — [`docs/TOOL_VERSIONS.md`](docs/TOOL_VERSIONS.md)

---

> **Başlangıç:** Dersleri `lessons/01` ile başlayarak sırayla takip et.
> `README.md` de aynı sırayı gösterir.

## Day 1 — Cortex-M Fundamentals

- [ ] `[QUICK]` [01 — Cortex-M Family & Processor Selection](lessons/01-cortex-m-family/)
- [ ] `[STANDARD]` [02 — Programmer Model & Registers](lessons/02-programmer-model/)
- [ ] `[QUICK]` Ders 02'deki `decode_apsr_flags()` TODO'sunu tamamla
- [ ] `[QUICK]` Ders 02 "Break It" + "Debug It" — HardFault'u kendin teşhis et
- [ ] `[QUICK]` Ders 01 ve 02'nin "What I Learned" bölümlerini doldur

**Gün sonu hedefi:** R0–R12/SP/LR/PC/xPSR'yi GDB'de görebiliyorum;
CONTROL'ün `nPRIV` ve `SPSEL` alanlarını açıklayabiliyorum; bir HardFault'un kaynağını
CFSR/HFSR'den bulabiliyorum.

---

## Day 2 — Functions, Assembly & Stack

- [ ] `[STANDARD]` [03 — C and Assembly](lessons/03-c-and-assembly/)
- [ ] `[STANDARD]` [04 — Stack, MSP & PSP](lessons/04-stack-msp-psp/)
- [ ] `[QUICK]` Kendi assembly fonksiyonunu yaz ve C'den çağır
- [ ] `[QUICK]` Thread Mode'u PSP'ye geçir, GDB'de doğrula

**Gün sonu hedefi:** Bir C fonksiyonunun assembly karşılığını satır satır
takip edebiliyorum; stack'in nasıl büyüdüğünü bellekte görebiliyorum;
MSP ile PSP arasında bilerek geçiş yapabiliyorum.

---

## Day 3 — Privilege, SVC & Protection

- [ ] `[STANDARD]` [05 — Privilege & SVC](lessons/05-privilege-svc/)
- [ ] `[STANDARD]` [06 — MPU](lessons/06-mpu/)
- [ ] `[QUICK]` Unprivileged Thread Mode'a geç ve sınırı test et
- [ ] `[QUICK]` SVC ile privileged bir servis çağır
- [ ] `[STANDARD]` MPU ile korumalı bölge kur, ihlali MemManage ile doğrula

**Gün sonu hedefi:** Privileged/unprivileged ayrımını kodda kurabiliyorum;
MPU ihlalini sadece "crash oldu" diye değil, fault status register'larıyla
açıklayabiliyorum.

---

## Day 4 — Exceptions, Faults & Build Pipeline

- [ ] `[STANDARD]` [07 — Exceptions & NVIC](lessons/07-exceptions/)
- [ ] `[STANDARD]` [08 — Faults & Fault Analyzer](lessons/08-faults/)
- [ ] `[STANDARD]` [09 — Compiler, Optimization & AAPCS](lessons/09-compiler-aapcs/)
- [ ] `[STANDARD]` [10 — Linker, ELF & Memory Layout](lessons/10-linker-elf-memory/)
- [ ] `[DEEP]` Fault analyzer'ın CFSR decode kısmını kendin yaz

**Gün sonu hedefi:** Exception stack frame'ini bellekte elle çözebiliyorum;
EXC_RETURN'ün ne anlattığını biliyorum; bir global değişkenin ELF'te nerede
olduğunu map file'dan bulabiliyorum.

---

## Day 5 — Startup & Debugging

- [ ] `[STANDARD]` [11 — Startup: Reset to main](lessons/11-startup/)
- [ ] `[STANDARD]` [12 — Debugging & DWT Profiling](lessons/12-debug-dwt/)
- [ ] `[DEEP]` Kendi `Reset_Handler`'ını sıfırdan yaz
- [ ] `[QUICK]` Reset'ten `main`'e kadar sadece `stepi` ile git
- [ ] `[QUICK]` DWT'nin bu target'ta saymadığını kendin doğrula

**Gün sonu hedefi:** `main` çalışmadan önce ne olduğunu adım adım
anlatabiliyorum; bir ölçümün neden geçersiz olduğunu savunabiliyorum.

---

## Day 6 — Advanced Execution

- [ ] `[DEEP]` [13 — Synchronization, FPU & DSP](lessons/13-sync-fpu-dsp/)
- [ ] `[STANDARD]` [14 — SysTick & PendSV](lessons/14-systick-pendsv/)
- [ ] `[QUICK]` Race condition üret, sonra exclusive access ile düzelt
- [ ] `[QUICK]` 1 ms tick kur ve çalıştığını doğrula

**Gün sonu hedefi:** Bir race condition'ı hem üretebiliyorum hem
düzeltebiliyorum; memory barrier'ın ne zaman gerektiğini açıklayabiliyorum.

---

## Day 7 — Scheduler & Security

- [ ] `[DEEP]` [15 — Educational Mini Kernel](lessons/15-mini-kernel/)
- [ ] `[DEEP]` [16 — TrustZone & Advanced Features](lessons/16-trustzone-advanced/)
- [ ] `[DEEP]` İki task arasında context switch çalıştır
- [ ] `[STANDARD]` SAU ile secure/non-secure ayrımını kur
- [ ] `[QUICK]` Cache / PACBTI / MVE için target desteğini **ölç**, sonucu kaydet
- [ ] `[STANDARD]` Final entegrasyon planını yaz — [`final-project/`](final-project/)

**Gün sonu hedefi:** Kendi yazdığım context switch ile iki task çalışıyor;
TrustZone sınırının neyi engellediğini deneyle gösterebiliyorum.

---

## DEEP TRACK — sonra dönülecekler

Bu çalışmalar FAST TRACK için ön koşul değildir; daha sonra tamamlanabilir:

- [ ] M55 (`mps3-an547`) platform portu — MVE capability'sini ölç
- [ ] Arm FVP kurulumu — QEMU'da gözlemlenemeyenler için
- [ ] Fault analyzer'ı tam sürüme çıkar (tüm CFSR bitleri)
- [ ] MPU ile task isolation — mini kernel üzerine
- [ ] Cache davranışı (uygun target bulunduğunda)
- [ ] PACBTI demo (target desteği doğrulandıktan sonra)
- [ ] Gerçek donanıma geçiş — [`docs/REAL_HARDWARE_MIGRATION.md`](docs/REAL_HARDWARE_MIGRATION.md)

---

## Günlük rutin

1. Bu dosyadan günün derslerini aç
2. Dersi oku → build et → çalıştır → GDB'de gözle
3. **Experiment** ve **Break It** deneylerini çalıştır
4. Gözlemlerini **What I Learned** bölümüne kaydet
5. Kutuyu işaretle, [`docs/PROGRESS.md`](docs/PROGRESS.md) ve README progress'i güncelle

## Takıldığında

| Sorun | Bak |
|---|---|
| Build olmuyor | [`docs/ENVIRONMENT.md`](docs/ENVIRONMENT.md) |
| Fault aldım, anlamıyorum | [`docs/DEBUGGING.md`](docs/DEBUGGING.md) |
| Bu özellik neden çalışmıyor | [`docs/TARGET_CAPABILITIES.md`](docs/TARGET_CAPABILITIES.md) |
| Adresler nereden geliyor | [`docs/MEMORY_MAP.md`](docs/MEMORY_MAP.md) |
| Terim anlamadım | [`docs/GLOSSARY.md`](docs/GLOSSARY.md) |
