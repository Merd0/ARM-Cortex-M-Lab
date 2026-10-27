**Dil:** [English](README.md) | Türkçe

# Reference Solution — Critical Update

[`critical.c`](critical.c) önceki PRIMASK'ı kaydeder, maskeler, read–pend–write
adımlarını yapar ve **kaydedilen maskeyi** geri yükler. Starter'a linklenmez.

Girişte maske açıksa pending SysTick, yeniden açılınca çalışır. Maske zaten
kapalıysa fonksiyon onu kapalı bırakır; sürücü açtıktan sonra handler çalışır.
PRIMASK kritik bölümünün kapsamı bu Secure single-core lab'ındaki
maskelenebilir exception'lardır; NMI/HardFault, DMA veya çok çekirdekli
bir kilit sağlamaz. `volatile` burada erişimleri görünür tutar; C11 thread
atomikleri yerine geçtiği iddia edilmez.

```bash
cmake --preset debug
cmake --build --preset debug --target 13-solution-critical
./scripts/run-qemu.sh build/debug/lessons/13-sync-fpu-dsp/13-solution-critical.elf
```

Yerel debug/release/minsize: **6/6 PASS**, END. Giriş maskesi ve toplam ayrı
sınanır. Sadece `counter++` veya koşulsuz IRQ açma bu sözleşmeyi karşılamaz.

[← Alıştırma](../../exercises/01-critical-update/README.tr.md)
