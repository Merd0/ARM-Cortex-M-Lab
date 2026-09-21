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
| [protected-application](protected-application/) | Unprivileged thread + PSP + MPU + SVC | 04, 05, 06, 07, 08 | Planlandı |
| [fault-analyzer](fault-analyzer/) | Crash'i okunur rapora çevirir | 07, 08 | Planlandı |
| [from-reset-to-main](from-reset-to-main/) | Sıfırdan startup + linker script | 10, 11 | Planlandı |
| [performance-profiler](performance-profiler/) | Ölçüm metodolojisi + optimizasyon karşılaştırması | 09, 12 | Planlandı |
| [mini-kernel](mini-kernel/) | SysTick + PendSV + SVC round-robin scheduler | 14, 15 | Planlandı |

Hepsinin birleşimi: [`final-project/`](../final-project/)

---

## Ortak kural: student vs infrastructure

Bu projelerde **iskeleti hazır bulacaksın, çekirdek mantığı sen yazacaksın.**
Hazır gelen kısımlar: CMake hedefi, platform bağlantısı, console çıktısı,
test senaryosu. `TODO(student)` ile işaretli kısımlar: öğrenme hedefinin ta
kendisi olan kod.

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
    src/             kaynak (TODO(student) blokları ile)
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
