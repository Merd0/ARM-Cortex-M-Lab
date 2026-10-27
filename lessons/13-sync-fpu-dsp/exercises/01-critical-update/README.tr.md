**Dil:** [English](README.md) | Türkçe

# Exercise 01 — Preserve the Critical Section

Önce [dersin](../../README.tr.md) kayıp güncelleme deneyini çalıştır.
[`critical.c`](critical.c) içindeki tek TODO'yu tamamla. Starter bilerek iş
yapmaz; sürücü **2/6 PASS** bildirir. Yalnız maskeyi bozmamak yeterli değildir.

## Sözleşme

`l13_protected_increment(counter)` Secure privileged Thread/MSP üzerinde,
geçerli ve word-aligned bir Normal SRAM sayacıyla çağrılır:

1. Mevcut PRIMASK'ı kaydet, maskelenebilir exception'ları maskele.
2. Sayacı oku; **okuma ile yazma arasında `l13_pend_update()` çağır**.
3. Okunan değere 1 ekleyip sayaca yaz.
4. Girişteki PRIMASK'ı geri yükle. Koşulsuz `__enable_irq()` kullanma.

`l13_pend_update()` tek SysTick exception'ını pending yapar; zamanlayıcı kurmaz.
Çağıran, gerekiyorsa maskeyi açıp deferred exception'ın çalışmasını bekler.
Bu fonksiyonda maskeliyken IRQ beklemek deadlock olur. Sürücüyü, handler'ı ve
beklenen sonuçları değiştirme. NMI/HardFault bu korumanın kapsamında değildir.

```bash
cmake --preset debug
cmake --build --preset debug --target 13-exercise-critical
./scripts/run-qemu.sh build/debug/lessons/13-sync-fpu-dsp/13-exercise-critical.elf
```

Sürücü iki durumu sınar: girişte PRIMASK=0 ve girişte PRIMASK=1.
İkisinde de Thread ve handler birer artış yapmalı, toplam 2 olmalı.
İkinci çağrıda fonksiyon dönünce PRIMASK hâlâ 1 ve handler sayısı 0 olmalı;
maskeyi sürücü açar. Tam çözüm üç preset'te **6/6** vermelidir.
Başlangıç sürümünü çözülmüş gibi işaretleme; ana test starter için 2/6 bekler.

## Break / Debug

- Maskeyi güncellemeden sonra kapatırsan stale read nasıl kaybolur?
- Maskeliyken pending exception'ın ne zaman çalışmasını beklersin?
- İç içe korumada koşulsuz enable neden çağıranın sözleşmesini bozar?
- DMB eklemek okuma–değiştirme–yazmayı tek başına atomik yapar mı?

[Ayrı referans çözüm](../../solutions/01-critical-update/README.tr.md).
