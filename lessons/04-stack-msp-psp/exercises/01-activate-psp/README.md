# Exercise 01 — Activate PSP Correctly

**Hedef:** Thread Mode'u kendi stack'ine taşımak — RTOS'ların her gün yaptığı şey.

## Görev

Dolduracağın dosya **C değil, assembly**:

```text
exercises/01-activate-psp/psp_start.S
```

```asm
psp_start:                      @ r0 = psp_top, r1 = entry  — DONMEZ
    /* TODO(student): 1) msr psp  2) CONTROL.SPSEL = 1  3) isb */

    ldr     lr, =psp_entry_trap
    orr     lr, lr, #1
    bx      r1
```

`main.c` hazır: stack bloğunu ve guard word'lerini kurar, `psp_start()`
çağırır ve dönmez. `thread_entry()` kontrolleri yapar. Sen yalnızca geçişin
üç adımını yazacaksın.

### Neden C'de değil?

Çünkü `CONTROL.SPSEL`'i sıradan bir C fonksiyonunun ortasında değiştirmek
**geçersizdir**. Derleyici o fonksiyon için MSP üzerinde bir frame ayırmış
ve yerel değişkenlere `[sp, #offset]` ile erişen komutlar üretmiştir; `SPSEL`
değişince aynı offset'ler başka bir adrese denk gelir. Ayrıntı
`psp_start.S`'in başındaki açıklamada.

### Cevaplaman gereken sorular

- Full-descending stack'te PSP'ye hangi adres yazılmalı — `&block[0]`,
  `&block[127]`, `&block[128]`? Neden?
- `CONTROL`'ü nasıl güncellersin ki `nPRIV` (bit 0) ezilmesin?
- Değişiklikten sonra hangi komut gerekiyor ve **burada** neden gerekiyor?
  (İpucu: C'den `__set_CONTROL()` çağırsaydın durum farklı olurdu.)

## Build & Run

```bash
cmake --build --preset debug --target 04-exercise-psp
./scripts/run-qemu.sh build/debug/lessons/04-stack-msp-psp/04-exercise-psp.elf
```

## Doğrulama

| Kontrol | Ne bakıyor |
|---|---|
| `CHECK_SPSEL` | Thread Mode'dayız ve `CONTROL.SPSEL` gerçekten 1 mi? |
| `CHECK_REGION` | Yerel değişkenler artık `task_stack_block` içinde mi? |
| `CHECK_STACK_GUARD` | Tahsis edilen alanın üstündeki nöbetçi word'ler bozulmamış mı? |

Üçü de `PASS` olmalı. Başlangıçta ilk ikisi `FAIL` verir — bu **beklenen**
başlangıç durumudur, bir bug değil.

`CHECK_STACK_GUARD` hakkında bir sınır: yalnızca **izlenen o iki konumun**
üzerine yazılmadığını gösterir. Alt sınırdan taşmayı veya başka bir yere kaçak
yazmayı yakalamaz. Stack pointer ayrıca GDB ile kontrol edilir:

```gdb
p/x $sp     p/x $msp     p/x $psp     p/x $control
p/x task_stack_block[128]
```

`$sp` hangisiyle aynı çıkıyor? `$psp` hep `0x30000000 .. 0x30000200`
aralığında mı kalıyor?

## Sık yapılan hata

İki adımın sırasını ters çevirmek. Önce `SPSEL = 1` yapıp sonra PSP'yi
ayarlarsan ne olur? Denemeden önce tahmin et — ipucu ders 02'nin Break It
bölümünde.

## Opsiyonel (DEEP) — Exercise 02

PSP aktifken iç içe üç fonksiyon çağrısı yap ve her seviyede `PSP`'yi kaydet.
Her seviye kaç byte tüketti? `release` preset'inde ne değişiyor?
Bu ödevin referans çözümü yok — cevabın kendi ölçümün.

## Takıldıysan

Referans çözüm: [`../../solutions/01-activate-psp/`](../../solutions/01-activate-psp/)

---

[← Ders 04](../../README.md)
