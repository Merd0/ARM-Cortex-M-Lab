**Dil:** [English](README.md) | Türkçe

# Reference Solution — Measurement Validity

[`measurement.c`](measurement.c), [aynı sözleşmeyi](../../measurement.h)
uygular. Starter'dan ayrı linklenir; onu tamamlanmış hale getirmez.

Önce I/O kontrol edilir. Ardından cycle counter yokluğu, Secure durumda
saymanın engellenmesi, enable bitleri ve uç değer eşitliği incelenir.
Sadece uygun girdiye `L12_COUNTER_DELTA` verilir. `end - start` unsigned
aritmetik nedeniyle modüler fark verir. Birden fazla tur sayısını iki uçtan
geri kazanamazsın; örneğin aynı uçlar tam bir turu da gizleyebilir.

```bash
cmake --preset debug
cmake --build --preset debug --target 12-solution-measurement
./scripts/run-qemu.sh build/debug/lessons/12-debug-dwt/12-solution-measurement.elf
```

Debug/release/minsize üzerinde çalıştırıldı: **12/12 PASS**, END.
Forward/wrap farkları 32; sıfırda ve sıfır dışında durmuş sayaç, kapalı
TRCENA, kapalı CYCCNTENA, NOCYCCNT, CYCDISS ve I/O vakaları doğru ayrılıyor.

Bu sonuçlar sentetik girdiler içindir. QEMU'da DWT çalıştığı anlamına gelmez.
Gerçek firmware snapshot'ları [dersin ölçüm bölümünde](../../README.tr.md).
