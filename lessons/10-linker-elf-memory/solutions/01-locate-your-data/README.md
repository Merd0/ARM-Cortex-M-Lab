# Solution 01 — Locate Your Data

Exercise: [`../../exercises/01-locate-your-data/`](../../exercises/01-locate-your-data/)

Kod: [`locate.c`](locate.c)

## Gerçek çıktı

| Preset | `FLASH_USED_BYTES` | `RAM_STATIC_BYTES` | `G_INIT_LMA` | Kontroller |
|---|---|---|---|---|
| `debug` | 1424 | 8 | `0x1000058C` | 4/4 PASS |
| `release` | 1584 | 8 | `0x1000062C` | 4/4 PASS |
| `minsize` | 1184 | 8 | `0x1000049C` | 4/4 PASS |

Üç preset'te üç farklı sayı, aynı kod. Sabit bir sayı yazan bir çözüm bunlardan
en fazla birinde geçerdi.

Aşağıdaki kod bloklarının tamamı, sırayla birleştirildiğinde derlenebilir bir
`locate.c` oluşturur; üç preset'te bu tablodaki değerlerin aynısını üretir ve
0 uyarı verir (`-Wall -Wextra`).

## Önce: adres aritmetiği neden `A()` ile?

Linker sembolleri C'de `extern uint32_t __data_start;` olarak bildirili;
`&__data_start` bir `uint32_t *`. İki `uint32_t *`'ı doğrudan çıkarmak
(`&__data_end - &__data_start`) **byte değil eleman** (4 byte'lık word) sayısı
verir — sonuç 4 kat küçük çıkar. `&__etext + n` de `n` **word** ilerler.

Bu yüzden her hesap adresleri önce tamsayıya çeviriyor; aşağıdaki kod
[`locate.c`](locate.c) ile birebir aynı:

```c
#include "l10.h"
#include "locate.h"

static uint32_t A(const void *p) { return (uint32_t)(uintptr_t)p; }
```

## `flash_used_bytes()`

```c
uint32_t flash_used_bytes(void)
{
    uint32_t ro_part   = A(&__etext) - A(&__vectors_start);   /* byte */
    uint32_t data_copy = A(&__data_end) - A(&__data_start);   /* byte */
    return ro_part + data_copy;
}
```

FLASH yük görüntüsü:

```text
__vectors_start (0x10000000)
    .vectors  .text  .rodata  (+ hizalama)
__etext = LOADADDR(.data)
    .data'nin ilk degerleri
__flash_load_end
```

- `__etext` adı "`.text`'in sonu" gibi okunur ama tanımı `LOADADDR(.data)` —
  yani `.rodata` ve hizalama **dahil**, salt-okunur kısmın bittiği ve `.data`
  kopyasının başladığı yer.
- `.data` FLASH'ta **da** yer kaplar: ilk değerleri orada saklanır.
- `.bss` FLASH'ta **yer kaplamaz**: ilk değeri sıfır, saklanacak bir şey yok.

Sonuç linker'ın kendi hesabıyla aynı: `__flash_load_end − ORIGIN(FLASH)`.
Linker script bu sembolü zaten bir `ASSERT` için tanımlıyor.

## `ram_static_bytes()`

```c
uint32_t ram_static_bytes(void)
{
    return (A(&__data_end) - A(&__data_start)) +
           (A(&__bss_end)  - A(&__bss_start));
}
```

`.data` RAM'de de yer kaplar (VMA). Stack **dahil değil**: bir section değil,
linker script'te bir muhasebe (`__stack_size = 4K`, `__stack_top`,
`__stack_limit`). `--print-memory-usage` da onu saymıyor.

## `data_lma_of(p)`

```c
uint32_t data_lma_of(const void *p)
{
    return A(&__etext) + (A(p) - A(&__data_start));
}
```

`p` bir `const void *`, `&__data_start` bir `uint32_t *`: ikisini doğrudan
çıkarmak derlenmez bile. Byte ofseti ancak tamsayı adresleri üzerinden
alınabilir.

Linker `.data`'yı RAM'de (VMA) ve FLASH'ta (LMA) **aynı iç düzenle**
yerleştirir: bir nesnenin `.data` başına göre ofseti iki yerde de aynıdır.
Startup kodu tam da bu yüzden tek bir düz kopyalama döngüsüyle
(`__etext` → `__data_start`, `__data_end`'e kadar) işini bitirebiliyor.

Kontrol bu adresi **okuyup** orada `0xC0DEFACE` buluyor — hesabın bağımsız
doğrulaması.

## `in_region()`

```c
int in_region(const void *p, uint32_t origin, uint32_t length)
{
    uint32_t a = A(p);
    return (a >= origin) && ((a - origin) < length);
}
```

`a < origin + length` yerine `(a − origin) < length` yazıldı:
`origin + length` 32 bitte taşabilecek bir bölge için de doğru kalır
(örneğin `0xF0000000` + `0x20000000`). Bu repo'nun bölgelerinde fark etmez,
ama alışkanlık olarak doğrusu bu.

---

[← Ders 10](../../README.md)
