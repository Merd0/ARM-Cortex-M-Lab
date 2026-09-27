# Solution 01 — Complete the Reset Sequence

Exercise: [`../../exercises/01-reset-sequence/`](../../exercises/01-reset-sequence/)

Kod: [`l11_reset.c`](l11_reset.c)

## Gerçek çıktı

```text
===== LESSON 11 SOLUTION 01 =====
[  g_l11_init] 0x11C0FFEE
[  g_l11_zero] 0x00000000
[  g_l11_phase] 0x00000123
[  vector[0]] 0x30008000
[  CHECK_DATA_INIT] PASS
[  CHECK_BSS_ZERO] PASS
[  CHECK_PHASE_ORDER] PASS
[  CHECK_VECTOR0_EQ_STACK_TOP] PASS
===== END =====
```

`debug`, `release`, `minsize` — üçünde de aynı.

## 1) `.data`

```c
const uint32_t *src = &__etext;          /* LMA: FLASH'taki ilk degerler */
uint32_t *dst = &__data_start;           /* VMA: RAM'deki degiskenler   */
while (dst < &__data_end) {
    *dst++ = *src++;
}
```

- `__etext` adı "`.text`'in sonu" gibi okunur, ama linker script'teki tanımı
  `LOADADDR(.data)`: `.data`'nın **yükleme adresi**. Ders 10'da ölçüldü.
- Linker `.data`'yı FLASH'ta ve RAM'de **aynı iç düzenle** yerleştirir; bu
  yüzden tek, düz bir kopyalama yetiyor.
- Burada işaretçi aritmetiği **bilinçli olarak word** birimiyle: `dst++` 4 byte
  ilerler, `dst < &__data_end` işaretçi karşılaştırması. Bu yalnızca sınırlar
  4'e hizalı olduğu için doğru; linker script `ALIGN(4)` ile bunu garanti
  ediyor. (Ders 10'daki byte/eleman tuzağıyla karıştırma: orada **boyut**
  hesaplanıyordu, burada word word **yürünüyor**.)

## 2) `.bss`

```c
uint32_t *dst = &__bss_start;
while (dst < &__bss_end) {
    *dst++ = 0u;
}
```

`.bss`'in FLASH'ta bir kopyası yok: tek yapılacak şey sıfır yazmak.

## Sıra

`.data` ve `.bss` ayrık aralıklar; birbirlerine dokunmuyorlar. Bu yüzden
aralarındaki sıra teknik olarak önemli değil. Önemli olan, **ikisinin de**
C kodundan (`SystemInit`, `main`) **önce** bitmesi:

- `SystemInit` bir C fonksiyonu; `.data`/`.bss`'teki bir değişkene dayanabilir.
- `main` kesinlikle dayanır.

Bu çözüm paylaşılan startup'la aynı sırayı izliyor: `.data` → `.bss` →
`SystemInit` → `main`. Bu bir **repo tasarımı**; mimari bir kural değil.

## Faz kaydı neden `.bss`'te olduğu halde güvenli?

`g_l11_phase` `.bss`'te. Değeri Reset_Handler'ın **yerel** bir değişkeninde
biriktiriliyor ve `g_l11_phase`'e ancak iki döngü ve `SystemInit`'ten
**sonra** yazılıyor. Böylece ne zehirleme ne `.bss` sıfırlaması onu silebilir —
eksik bir `.bss` adımı bile (başlangıç ve bozuk hedef).

## Zehirleme neden var?

QEMU RAM'i sıfırla başlatıyor. Zehirleme olmasaydı `.bss` adımını unutan bir
çözüm de `CHECK_BSS_ZERO PASS` alırdı — `tests/build-probes.sh`'teki L11b
vaka C bunu her build'de ölçüyor. `l11_poison_ram()` bir **eğitim aracı**;
paylaşılan startup'ta yok ve olmamalı.

---

[← Ders 11](../../README.md)
