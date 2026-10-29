**Dil:** [English](quick-check.md) | Türkçe

# Lesson 12 — Quick Check Cevapları

## 1. Watchpoint neden source breakpoint'ten farklıdır?

Breakpoint belli bir yürütme adresinde durdurur. `watch -l g_l12_watch`,
seçilen bellekte değer değişince durdurur; yazan fonksiyonu önceden bilmek
gerekmez. `rwatch` okuma erişimini izler. GDB'deki "hardware" etiketi QEMU
TCG gdbstub desteğini anlatır; fiziksel DWT comparator sayısını kanıtlamaz.

## 2. CTRL.NOCYCCNT=0 ve CYCCNT=0 neyi kanıtlar?

Tek başına çalışan cycle counter'ı kanıtlamaz. QEMU 11.1.0 testinde CTRL ve
DEMCR yazılarının enable bitleri de geri okunmadı; iş yükünden önce/sonra
CYCCNT sıfır kaldı. Register snapshot'ı ile gözlenen davranış ayrı yazılır.
"İş sıfır cycle sürdü" sonucu çıkarılmaz.

## 3. `0xFFFFFFF0` → `0x10` farkı kaçtır?

`uint32_t` modüler çıkarma ile 32. Bunun gerçek elapsed count olması için
aralık `2^32` artıştan kısa olmalı. Aynı uçlar sıfır ilerleme ile tam turu
ayıramaz. Eşit uçları bu deney kullanılabilir ölçüm olarak kabul etmez.

## 4. O2 iş yükü daha küçükse donanımda daha hızlı mı?

Bu deney bunu kanıtlamaz. 70/32 byte ve 35/13 statik disassembly komutu
bu GCC build'inin kod ölçüleridir; döngülerin dinamik kaç kez yürütüldüğü
ve komutların donanım latency'si farklı konulardır. QEMU SysTick iterasyon
oranını fiziksel Cortex-M33 hız oranına çevirmek de geçersizdir.

## 5. Ölçüm aralığındaki console yazısı neden sorun?

İş yüküne UART sürücüsü ve çıktı beklemeleri eklenir. `12-profile-io`
sonuç checksum'ını bozmaz ama `CHECK_CLEAN_WINDOW` FAIL olur. Her koşulda
daha az iterasyon bekleyen test yazılmaz: host yükü ve emülatör zamanlaması
sonuçları etkiler. Ölçüm kapsamının değişmesi tek başına teşhistir.

## 6. `bt` ve stack dump neden aynı şey değildir?

`bt` debug/unwind bilgisi ve prolog çözümlemesiyle frame'leri çıkarır.
Stack'te her Flash benzeri word dönüş adresi değildir; veri de olabilir.
Leaf fonksiyon LR'yi stack'e hiç yazmayabilir. Bu derste `l12_middle` ve
`l12_outer` prolog/epilogları disassembly ile okunup kaydedilmiş LR'ler
çağrı yerleriyle karşılaştırılır. Bir build'in offset'leri diğerine taşınmaz.

## 7. Öğretim header'ı CMSIS'in tam karşılığı mı?

Hayır. İki seçilmiş read fonksiyonunun aynı makine kodu üretmesi yalnızca
o fonksiyonları, bu flag'leri ve bu toolchain'i kapsar. SAU korumaları,
eksik register'lar ve makrolar gibi farklar devam eder.

[← Ders](../README.tr.md)
