**Dil:** [English](README.md) | Türkçe

# Exercise 01 — Is This Measurement Usable?

Önce [dersi](../../README.tr.md) çalıştır. Bu alıştırma gerçek QEMU cycle ölçümü
yapmaz; sentetik register snapshot'larını yorumlar. Kod yalnızca
[`measurement.c`](measurement.c) içindeki iki `TODO(student)` ile tamamlanır.

## Sözleşme

[`measurement.h`](../../measurement.h) girişleri ve enum değerlerini tanımlar.
Tüm snapshot'lar Secure durumda alınmış kabul edilir; pointer geçerlidir.

1. Ölçüm aralığında I/O varsa diğer alanlardan bağımsız `L12_CONTAMINATED` dön.
2. Aşağıdakilerden biri varsa `L12_UNAVAILABLE` dön: `NOCYCCNT=1`,
   `CYCDISS=1`, `CYCCNTENA=0`, `DEMCR.TRCENA=0` veya eşit uç değerler.
3. Diğer durumlarda `L12_COUNTER_DELTA` dön. Bu yalnızca counter farkının
   kullanılabileceğini belirtir; hardware benchmark veya WCET onayı değildir.
4. `l12_delta()` iki `uint32_t` counter değerinin modüler farkını hesaplasın.
   Çağıran, aralığın `2^32` artıştan kısa olduğunu ayrıca garanti etmelidir.
   Başlangıç `0xFFFFFFF0`, bitiş `0x10` ise sonuç 32 olmalıdır.

Bu API'nin geri dönüşleri enum'dur; metnin ilk harfine dayalı kabul yoktur.
Driver'daki beklenen sonuçları veya platform kodunu değiştirme.

## Çalıştır

```bash
cmake --preset debug
cmake --build --preset debug --target 12-exercise-measurement
./scripts/run-qemu.sh build/debug/lessons/12-debug-dwt/12-exercise-measurement.elf
```

Başlangıç sürümü **0/12** bildirir; on iki kontrol de FAIL'dir ve END'e ulaşır.
Tamamlandığında aynı driver **12/12** bildirmeli. `release` ve `minsize` ile
tekrar et. Ana lesson testinin starter için bilerek 0/12 beklediğini unutma:
öğrenci çözümünü ana repository starter'ının üzerine teslim etmiyoruz.

## Teşhis

- Yalnızca sıfır counter'a mı baktın? Eşit ama sıfır olmayan uçları dene.
- Sayaç değerleri farklı olsa da enable bitleri kapalıysa ne olur?
- Wrap, mutlak değer veya signed çıkarma ile doğru hesaplanır mı?
- Eşit uçlar sıfır iş demek mi, durma veya tam tur da olabilir mi?

Bit adları için vendored CMSIS `core_cm33.h`; çözüm için ancak kendi
denemenden sonra [ayrı referans çözüm](../../solutions/01-valid-measurement/README.tr.md).
