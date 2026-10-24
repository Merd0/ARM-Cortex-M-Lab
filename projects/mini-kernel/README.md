# Project — Educational Mini Kernel

**STATUS: yerel Debug/Release/MinSize PASS; uzak CI/merge onayı bekliyor.**

Lesson 15 kernel/frame/context kaynakları değiştirilmiş kopyalar yerine doğrudan
derlenir. Üç küçük weak uygulama hook'u yalnız bounded iş ve sonuç raporlaması
ekler; dersin varsayılan davranışı değişmez. Ready hook'u queue boşalmadan
bitişe izin vermez; testin beklediği 16 örnek azaltılmaz. [application.c](src/application.c)
iki task arasında sekiz slot'lu ring buffer kullanır: producer 0..15 üretir,
consumer toplamı hesaplar. Counter ve queue güncellemesi mevcut kısa observation
kritik bölümünde yapılır; UART yalnız bitiş Handler'ındadır.

```bash
cmake --preset debug
cmake --build --preset debug --target project-kernel
./scripts/run-qemu.sh build/debug/projects/mini-kernel/project-kernel.elf
python tests/system-smoke.py
```

Debug/QEMU: PRODUCED=16, CONSUMED=16, QUEUE_SUM=120, CHECK_QUEUE=PASS.
Kernel'in dokuz context/stack/progress kontrolü de PASS. İlk cooperative
yield'lerden sonra task'lar timer preemption ile ilerler. R4–R11 ve EXC_RETURN
saklanır; stack sınırı STM'den önce doğrulanır. PSP task'lara, MSP Handler'lara
aittir; ilk MSP frame'i tekrar kullanılmaz.

Bu referans sürüm privileged task/basic Secure frame kullanır; FP task context,
MPU task izolasyonu, priority inheritance veya üretim RTOS garantisi yoktur.
Unprivileged ve TrustZone entegrasyonu final projede ayrı doğrulanacak.
[Lesson 15](../../lessons/15-mini-kernel/) starter ve çözümü ayrı kalır.

[Projects](../README.md)
