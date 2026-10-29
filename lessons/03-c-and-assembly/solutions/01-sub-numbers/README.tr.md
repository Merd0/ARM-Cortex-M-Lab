**Dil:** [English](README.md) | Türkçe

# Solution 01 — `sub_numbers`

Exercise: [`../../exercises/01-sub-numbers/`](../../exercises/01-sub-numbers/README.tr.md)

## Build & Run

```bash
cmake --build --preset debug --target 03-solution-sub
./scripts/run-qemu.sh build/debug/lessons/03-c-and-assembly/03-solution-sub.elf
```

## Gerçek çıktı

```text
===== LESSON 03 SOLUTION 01 =====
[SUB_NUMBERS(12,5)] 7
[SUB_NUMBERS(5,12)] 4294967289
[CHECK_12_MINUS_5] PASS
[CHECK_5_MINUS_12] PASS
===== END =====
```

## Üretilen kod (ölçüldü)

```text
10000040 <sub_numbers>:
10000040:  1a40    subs  r0, r0, r1
10000042:  4770    bx    lr
```

İki komut, dört byte. `add_numbers` ile birebir aynı yapı — yalnızca
aritmetik komut değişti. AAPCS tarafında hiçbir şey farklı değil: argümanlar
`r0`/`r1`, sonuç `r0`, dönüş `lr`.

## `4294967289` neden?

`4294967289` = `0xFFFFFFF9`. Bu, `-7` sayısının 32 bitlik ikiye tümleyen
(two's complement) gösterimidir. Register yanlış bir değer tutmuyor; biz onu
`console_kv_u32()` ile **işaretsiz** olarak bastığımız için öyle görünüyor.

Buradaki asıl ders: bir register 32 bitlik bir desen tutar. "İşaretli" veya
"işaretsiz" olması register'ın değil, **onu okuyan kodun** özelliğidir. İşlemci
`subs` komutunu çalıştırırken ikisini ayırt etmez; ayrımı `BGE` (signed) ile
`BHS` (unsigned) gibi dallanma komutları yapar.

## `subs` mi `sub` mu?

İkisi de doğru sonucu üretir; fark APSR bayraklarını (N/Z/C/V) güncelleyip
güncellememeleridir. Burada bayrakları kimse okumuyor, dolayısıyla ikisi de
çalışır.

`subs` seçildi çünkü düşük register'larla 16-bit kodlamaya sığıyor (`0x1a40`,
2 byte). Ama bu bir kural değil, bu operand'lar için ölçülmüş bir sonuçtur —
farklı operand'larda assembler farklı kodlama seçebilir. Doğru alışkanlık:
ezberlemek yerine `objdump` ile bakmak.

---

[← Ders 03](../../README.tr.md)
