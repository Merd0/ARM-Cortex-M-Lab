# Solution 01 — Decode an Exception Entry

Exercise: [`../../exercises/01-decode-exception-entry/`](../../exercises/01-decode-exception-entry/)

## Çözüm

```c
static void decode_exception_entry(const exception_frame_t *frame,
                                   uint32_t exc_return,
                                   uint32_t ipsr,
                                   entry_info_t *out)
{
    out->interrupted_mode = (exc_return & 0x8u) ? "Thread" : "Handler";
    out->frame_stack      = (exc_return & 0x4u) ? "PSP"    : "MSP";
    out->exception_number = ipsr & 0x1FFu;

    out->stacked_r0   = frame->r0;
    out->stacked_pc   = frame->pc;
    out->stacked_xpsr = frame->xpsr;
}
```

## Build & Run

```bash
cmake --build --preset debug --target 07-solution-decode
./scripts/run-qemu.sh build/debug/lessons/07-exceptions/07-solution-decode.elf
```

Gerçek çıktı:

```text
[4] Exception girisi cozumu
[  INTERRUPTED_MODE] Thread
[  FRAME_STACK] PSP
[  EXCEPTION_NUMBER] 11
[  STACKED_R0] 0x11111111
[  STACKED_PC] 0x100000BA
[  STACKED_XPSR] 0x61000000
[  CHECK_DECODE_MODE] PASS
[  CHECK_DECODE_STACK] PASS
[  CHECK_DECODE_EXCNUM] PASS
[  CHECK_DECODE_R0] PASS
[  CHECK_DECODE_PC] PASS
[  CHECK_DECODE_XPSR] PASS
```

---

## Her satır neden böyle

### `interrupted_mode` — `EXC_RETURN` bit 3

```c
(exc_return & 0x8u) ? "Thread" : "Handler"
```

Bit 3 (`Mode`) dönüşün **nereye** yapılacağını söyler. Dönülecek yer, kesilen
bağlamın bulunduğu yerdir — yani aynı zamanda "hangi modu kestik" sorusunun
cevabı.

`1` = Thread Mode'a dönüş, `0` = Handler Mode'a dönüş (iç içe exception).

Bu ölçümde `0xFFFFFFFD` → bit 3 = 1 → Thread. Doğru, çünkü `svc` Thread
Mode'dan çalıştırıldı.

### `frame_stack` — `EXC_RETURN` bit 2

```c
(exc_return & 0x4u) ? "PSP" : "MSP"
```

Bit 2 (`SPSEL`) frame'in **hangi stack'e yığıldığını** söyler.

**Bu, handler'ın hangi stack'i kullandığı sorusu değildir.** Handler her zaman
MSP kullanır — donanım giriş sırasında `SPSEL`'i temizler. Ölçüm:

```text
frame (PSP'de):     0x300002d0
handler $sp (MSP):  0x30007ff0
```

İki farklı bölge, ikisi de doğru.

Deney 1'de MSP kaynaklı durumda `EXC_RETURN = 0xFFFFFFF9` → bit 2 = 0 → frame
MSP'de. Orada frame ile handler aynı stack'tedir; stack full-descending
olduğu için handler'ın prologu ve yerel verisi MSP'yi daha da aşağı iter ve
kaydedilmiş frame'in **altında, daha düşük adreslerde** büyür.

### `exception_number` — `IPSR`

```c
ipsr & 0x1FFu
```

`IPSR`, `xPSR`'nin alt 9 bitidir ve **o anda çalışan** exception'ın numarasını
taşır. `0` = Thread Mode.

Bu ölçümde `11` = SVCall. Aynı sayı vector table indeksidir: `vector[11]`.

Dikkat: `IPSR` parametresi handler içinde `__get_IPSR()` ile okundu. **Yığılmış
`xPSR`'nin `IPSR` alanı farklı bir şeydir** — o, *kesilen* bağlamın exception
numarasıdır ve bu ölçümde `0` (Thread Mode'daydık). İkisini karıştırma:

```text
__get_IPSR()  (handler icinde)  = 11   <- calisan exception
frame->xpsr & 0x1FF             = 0    <- kesilen baglamin modu
```

### `stacked_*` — frame yuvaları

Ofsetler mimari olarak sabittir; `exception_frame_t` struct'ı sırayı birebir
yansıtıyor:

```text
+0x00 R0   +0x04 R1   +0x08 R2   +0x0C R3
+0x10 R12  +0x14 LR   +0x18 PC   +0x1C xPSR
```

`stacked_pc = 0x100000BA` → `svc` komutundan **sonraki** komut. `PC - 2`'de
`svc 7` var (README'nin GDB bölümünde `x/2i` ile doğrulandı).

`stacked_xpsr = 0x61000000` → T biti (bit 24) `1`, `IPSR` alanı `0`, hizalama
biti (bit 9) `0`.

---

## Neden bu kadar basit?

Çünkü işin zor kısmını **sarmalayıcı** yaptı: giriş anında `EXC_RETURN`'ü ve
frame adresini kaydetti. O bilgi olmadan bu fonksiyon yazılamaz —
handler içinde C çalıştıktan sonra `lr` ve `msp` artık giriş değerleri
değildir.

Çözümün gerçek dersi bu: **doğru anı yakalamak, doğru bitleri okumaktan daha
zor.**

## Kapsam

Bu çözüm `FType = 1` (standart frame) ve tek güvenlik durumu varsayar.
Genişletilmiş FP frame'inde `stacked_*` ofsetleri aynıdır ama frame daha
uzundur; güvenlik durumu geçişlerinde ek varyantlar vardır. `cm-frame` bu
durumları zaten reddediyor — bkz. `docs/DEBUGGING.md`.

---

[← Ders 07](../../README.md)
