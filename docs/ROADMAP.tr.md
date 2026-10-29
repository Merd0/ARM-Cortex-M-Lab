**Dil:** [English](ROADMAP.md) | Türkçe

# Roadmap

## Güncel durum — 2026-10-27

16 ders, beş proje ve final sistem main üzerinde. PR #9–11 merge edildi;
11fde900 için üç preset CI PASS. [İç mühendislik denetimi](ENGINEERING_AUDIT.tr.md)
düzeltmeleri ve release hazırlığını kaydeder. Denetim yayın/CI/merge kaydı
[PR #12](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/12) üzerindedir. Yeni tag/Release oluşturulmadı.

## Tarihsel tamamlama sırası — 2026-10-23

Başlangıç: `main` = `266642efb55e881a275a1034a01923ecbd17a3fa`.
Ders 01–11 ve önceki audit düzeltmeleri korunur. Yeni bir müfredat veya
platform tasarlanmıyor; aşağıdaki mevcut kapsam tamamlanacak.

1. **Lesson 12 — Debugging & DWT:** PR #8 ile main üzerinde tamamlandı;
   `01bc73c51046d09169f68c8cae009df4deb8e433` için merge sonrası üç preset
   CI PASS (2026-10-24). Cycle sonucu uydurulmadı.
2. **Lesson 13 — Sync/FPU/DSP:** bu dalda dokuz firmware hedefi; kontrollü
   lost-update/exclusive retry, PRIMASK alıştırması, NOCP, eager/lazy frame,
   DSP ve softfp/hard ABI pozitif/negatif link kontrolleri uygulandı.
   Yerel üç preset QEMU/GDB ve PR #9 üç preset CI PASS; PR #9 merge edildi.
3. **Lesson 14 — SysTick/PendSV:** uygulandı; Debug build ve dört QEMU
   senaryosu PASS (beklenen starter/bozuk sonuçları dahil). Yerel commit.
4. **Lesson 15 — Mini Kernel:** PSP/MSP, sentetik ilk frame, assembly
   R4–R11/EXC_RETURN koruması, stack sınırları ve scheduler regresyonları.
   Uygulandı; Debug ve dört QEMU senaryosu PASS. Yerel commit.
5. **Lesson 16 — TrustZone:** AN505 üzerinde gerçek Secure/Non-secure imaj,
   SAU/NSC/gateway ve yetkisiz erişim deneyleri; ileri özellikler capability
   kanıtı olmadan çalışıyor sayılmayacak. İki imaj, NSC scalar servis,
   AUVIOL ve INVEP uygulandı; Debug/dört çift QEMU PASS. Yerel commit.
6. **Birleştirici projeler:** From Reset to Main (10–11), Fault Analyzer
   (07–08), Protected Application (04–08), Performance Profiler (09/12),
   Educational Mini Kernel (14–15). Her biri ayrı kabul kapısıdır.
7. **Final sensor node:** beş projenin ve TrustZone sınırının entegrasyonu;
   sentetik sensör, veri işleme ve kontrollü servisler. Güvenlik iddiaları
   negatif testlerle, gerçek zaman iddiaları hedef/model sınırlarıyla raporlanır.

2026-10-24 kullanıcı kararı: 14 → 15 → 16 aynı yerel geliştirme dalında,
her biri ayrı commit ve temel build/QEMU/kritik doğruluk kapısıyla ilerledi.
Ardından **toplu yerel kalite kapısı** tamamlandı: ayrıntılı GDB, üç preset
karşılaştırması ve geniş smoke/build probes/negatif testler PASS.
PendSV stack sınırları, context bütünlüğü ve cross-security exception/NSC
sınırı incelendi. Bu, bağımsız üçüncü taraf audit veya kart doğrulaması değildir.

Uzak push/PR/merge için ayrıca kullanıcı onayı alınır; CI geçmeden merge yapılmaz.
Main korunur. Beş proje ve final entegrasyonu da yerel kapıları geçti.
Checkpoint etiketi ve Release ayrıca kararlaştırılır.

Önceki auditlerin korunan kabul kriterleri: fresh PSP frame/stack guard,
EXC_RETURN ve fault-frame reddi, M33 HFSR/BFSR kuralları, tam cevap eşitliği,
AAPCS/SP recovery, effective -O2, libgcc sırası, FLASH LMA sınırı, reserved
vektörler 8/9/10/13. Mevcut testler azaltılmayacak.

## Faz 0 - Bootstrap (TAMAMLANDI)

- Repository iskeleti, CMake + Ninja + Arm GNU Toolchain build sistemi
- MPS2-AN505 (Cortex-M33) platform portu: startup, linker script, UART
- QEMU'da calisan firmware + GDB baglantisi dogrulandi
- Target capability probe ve olculmus capability matrisi
- Ders 02 (Programmer Model) tam kalite referans ders olarak hazir

## Faz 1 - FAST TRACK (~1 hafta, ana ogrenme yolu)

Gun gun plan: [`BOOTCAMP.md`](../BOOTCAMP.tr.md)

| Gun | Konu | Lessons |
|---|---|---|
| 1 | Temeller ve programmer model | 01, 02 |
| 2 | Assembly, fonksiyon cagrisi, stack | 03, 04 |
| 3 | Privilege, SVC, MPU | 05, 06 |
| 4 | Exceptions, faults, build pipeline | 07, 08, 09, 10 |
| 5 | Startup ve debugging | 11, 12 |
| 6 | Senkronizasyon, FPU/DSP, SysTick | 13, 14 |
| 7 | Context switching, mini kernel, TrustZone | 15, 16 |

FAST TRACK sonunda elde olmasi gereken: calisan bir mini kernel,
kullanilabilir bir fault analyzer ve her konuda kendi gozlemlerin.

## Faz 2 - DEEP TRACK (opsiyonel, sonradan)

Her ders kendi DEEP TRACK deneylerini icerir. Ayrica:

- **M55 platform portu** - `mps3-an547` icin ikinci platform; MVE/Helium
  capability'sini gercekten olcmek (su an `OLCULMEDI`)
- **Arm FVP entegrasyonu** - QEMU'da gozlemlenemeyen davranislar icin
- **Fault analyzer'i tam surume cikarma** - tum CFSR bitlerinin decode'u
- **MPU ile task isolation** - mini kernel uzerine
- **PACBTI ve cache** - target destegi dogrulandiktan sonra

## Faz 3 - Specialization sonrasi

Bu repoya bagimli olmayan ayri bir faz:

```text
FreeRTOS veya Zephyr  ->  micro-ROS  ->  ROS 2
```

Mini kernel egitim amaclidir; uretim RTOS'u degildir. Gercek projede
olgunlasmis bir RTOS kullanilir. Bu repodaki bilgi o noktada
"RTOS'un altinda ne oluyor" sorusunun cevabi olur.

## Gercek donanim

Board alindiginda: `docs/REAL_HARDWARE_MIGRATION.md`

## Yerel tamamlama ve yayınlama sırası — 2026-10-24

Beş proje ve final sensor pipeline uygulandı. Son yerel üç-preset kapıları
[Progress](PROGRESS.tr.md) içinde: clean build, smoke/probes, 34 yeni runtime
senaryosu ve kritik GDB gözlemleri PASS. Öğrenci TODO'ları derslerde ayrı kaldı.

Kullanıcı sıralı yayınlamayı onayladı. Yeni müfredat eklenmez:
1. PR #9, üç preset CI sonrası merge edildi; commit geçmişi korundu.
2. Lesson 14–16 [PR #10](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/10) ile
   üç preset CI sonrası merge edildi.
3. Proje/final entegrasyonu PR #11 ile merge edildi; main `11fde900`,
   [merge sonrası üç preset CI PASS](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40105930630).

Güncel uzak sonuçlar [GitHub Actions](https://github.com/Merd0/ARM-Cortex-M-Lab/actions)
üzerindedir. Yerel inceleme ve CI bağımsız üçüncü taraf audit diye sunulmaz.
Tag/Release ayrıca kararlaştırılacak.
