# Projects

`lessons/` tek bir kavramı izole eder. `projects/` ise **birden fazla dersi
birleştirip** çalışan bir şey üretir. Aradaki fark önemli:

| | `lessons/` | `projects/` |
|---|---|---|
| Kapsam | Tek kavram | Birleşik davranış |
| Kod boyutu | Minimum | Orta |
| Amaç | Anlamak | Kurabildiğini kanıtlamak |
| Ne zaman | Konuyu ilk görürken | İlgili dersler bitince |

Her proje, bağımlı olduğu dersler tamamlanmadan başlatılmamalı. Bağımlılık
tablosu aşağıda.

---

## Proje listesi

| Proje | Ne yapar | Gerekli dersler | Durum |
|---|---|---|---|
| [protected-application](protected-application/) | Unprivileged thread + PSP + MPU + SVC | 04, 05, 06, 07, 08 | Yerel üç-preset PASS; uzak CI bekliyor |
| [fault-analyzer](fault-analyzer/) | Crash'i okunur rapora çevirir | 07, 08 | Yerel üç-preset PASS; uzak CI bekliyor |
| [from-reset-to-main](from-reset-to-main/) | Sıfırdan startup + linker script | 10, 11 | Yerel üç-preset PASS; uzak CI bekliyor |
| [performance-profiler](performance-profiler/) | Ölçüm metodolojisi + optimizasyon karşılaştırması | 09, 12 | Yerel üç-preset PASS; uzak CI bekliyor |
| [mini-kernel](mini-kernel/) | SysTick + PendSV + SVC round-robin scheduler | 14, 15 | Yerel üç-preset PASS; uzak CI bekliyor |

Hepsinin birleşimi: [`final-project/`](../final-project/)

---

## Ortak kural: student vs infrastructure

Projelerin src/ dizinleri çalışan referans uygulamalardır. Aynı mekanizma
yalnız ayrı proje görünümü için kopyalanmaz: Lesson 06, 08, 11, 12 ve 15
kaynakları ilgili hedeflerde yeniden derlenir. `TODO(student)` alıştırmaları
ve ayrı çözümler lessons/ altında korunur; referans proje kodu starter'a sızdırılmaz.

Bitirdiğinde `review mode` diyebilirsin — kodu architectural correctness,
AAPCS, register preservation, stack alignment, volatile, privilege ve MPU
açısından gözden geçiririm; önce sorunu ve severity'sini söylerim, çözümü
doğrudan yazmam.

---

## Proje şablonu

Her proje dizini şunu içerir:

```text
<proje>/
    README.md        problem tanımı + mimari diyagram + kabul kriterleri
    src/             çalışan referans entegrasyon
    CMakeLists.txt   add_firmware() çağrısı
```

`README.md` içinde en az:

- **Problem** — hangi gerçek ihtiyaç?
- **Architecture** — mermaid diyagram
- **Acceptance criteria** — "bitti" ne demek, ölçülebilir şekilde
- **Bağımlı dersler**
- **Doğrulama** — QEMU çıktısı ve GDB'de ne görülmeli

---

[← Ana sayfa](../README.md) · [Bootcamp](../BOOTCAMP.md)

## Ortak doğrulama

```bash
cmake --preset debug
cmake --build --preset debug
python tests/system-smoke.py --group projects
python tests/profile-report.py
```

Her proje bağımsız ELF üretir. System smoke 15 senaryoda bütün beklenen
CHECK alanlarını doğrular; ayrıntılı kayıt [Progress](../docs/PROGRESS.md) içindedir.
