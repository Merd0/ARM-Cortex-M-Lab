# Memory Map - MPS2-AN505 (Cortex-M33)

Buradaki adresler iki kaynakla dogrulanmistir:

1. QEMU'nun kendi modeli: `qemu-system-arm -M mps2-an505 -monitor stdio` -> `info mtree -f`
2. Arm Application Note AN505 (DAI0505) ve Arm IoT Kit bellek haritasi

## Secure / Non-secure alias mantigi

Armv8-M'de bir bellek bolgesi iki farkli adresten gorunebilir.
IoT Kit'te ayrim **bit 28**'dir:

```text
0x0_______   bit28 = 0   ->  Non-secure alias
0x1_______   bit28 = 1   ->  Secure alias     (ayni fiziksel bellek)
```

Firmware **Secure state'te boot eder**, bu yuzden bu repoda Secure alias'lar
kullanilir (`0x10000000`, `0x30000000`, `0x50200000`).

## QEMU'da gercekten var olan bolgeler

| Adres (NS) | Adres (S) | Boyut | Ne |
|---|---|---|---|
| `0x00000000` | `0x10000000` | 4 MB | ZBT SSRAM1 - kod bolgesi |
| `0x20000000` | `0x30000000` | 32 KB | IoT Kit internal SRAM |
| `0x28000000` | `0x38000000` | 2 MB | ZBT SSRAM2 |
| `0x28200000` | `0x38200000` | 2 MB | ZBT SSRAM3 |
| `0x40200000` | `0x50200000` | 4 KB | CMSDK APB UART0 |
| `0x40080000` | `0x50080000` | 4 KB | IoT Kit Security Controller |
| - | `0xE0000000` | 1 MB | Private Peripheral Bus (SCS, NVIC, SCB, MPU, SAU, DWT) |

## Bu repodaki linker yerlesimi

`platform/mps2-an505/linker/mps2-an505-secure.ld`:

```text
FLASH : ORIGIN = 0x10000000, LENGTH = 512K
RAM   : ORIGIN = 0x30000000, LENGTH = 32K
```

FLASH neden 512 KB? Fiziksel olarak 4 MB var. Bilerek kisitliyoruz ki
gercek bir MCU gibi davransin ve "region overflow" hatalari anlamli olsun.

## Yerlesim diyagrami

```text
0x10000000  +----------------------------+  <-- FLASH ORIGIN
            |  .vectors                  |      [0] = initial MSP
            |    [0] __stack_top         |      [1] = Reset_Handler
            |    [1] Reset_Handler       |      reset'te Secure VTOR buraya bakar
            |    ...                     |
            +----------------------------+
            |  .text     (kod)           |
            +----------------------------+
            |  .rodata   (sabitler)      |
            +----------------------------+
            |  .data'nin FLASH kopyasi   |  <-- __etext (LMA)
            +----------------------------+
0x10080000  |  (kullanilmayan)           |      512 KB siniri
            +----------------------------+


0x30000000  +----------------------------+  <-- RAM ORIGIN
            |  .data  (RAM'e kopyalanir) |  <-- __data_start .. __data_end
            +----------------------------+
            |  .bss   (sifirlanir)       |  <-- __bss_start .. __bss_end
            +----------------------------+
            |  .heap  (su an 0 byte)     |
            +----------------------------+
            |                            |
            |     BOS ALAN               |
            |                            |
            +----------------------------+  <-- __stack_limit
            |  STACK                     |
            |     |                      |      full descending:
            |     v  asagi buyur         |      SP azalarak buyur
0x30008000  +----------------------------+  <-- __stack_top = initial MSP
```

## Nasil dogrularim?

```bash
# Linker'in gercekte nereye koydugunu gor
arm-none-eabi-nm -n build/debug/lessons/02-programmer-model/02-programmer-model.elf | head -20

# Section'lar ve adresleri
arm-none-eabi-objdump -h build/debug/lessons/02-programmer-model/02-programmer-model.elf

# Map file icinde sembol ara
grep -n "__stack_top\|__data_start\|__bss_start" \
    build/debug/lessons/02-programmer-model/02-programmer-model.map
```

GDB'de:

```text
p/x &__stack_top
p/x &__bss_start
x/4wx 0x10000000        # vector table'in ilk 4 girdisi
```

## Onemli ayrinti: initial MSP nereden gelir?

Cortex-M33 reset'te:

1. Secure VTOR reset degeri `0x10000000` (QEMU'nun IoT Kit modeli boyle ayarlar)
2. Donanim `[0x10000000]` adresindeki word'u okur -> **MSP**
3. `[0x10000004]` adresindeki word'u okur -> **PC** (Reset_Handler)

Olculen (`arm-none-eabi-objdump -s -j .vectors`):

```text
Contents of section .vectors:
 10000000 00800030 21020010 1d020010 1d020010  ...0!...........
```

Little-endian cozumu:

```text
vector[0] = 0x30008000   -> initial MSP  (= __stack_top)
vector[1] = 0x10000221   -> Reset_Handler
```

`0x10000221` -> en dusuk bit 1: **Thumb state gostergesi**.
Gercek fonksiyon adresi `0x10000220`. Bu bit 0 olsaydi islemci reset'te
hemen UsageFault (INVSTATE) uretirdi.

Sembollerle dogrulama:

```text
10000220 T Reset_Handler
30007000 A __stack_limit
30008000 R __stack_top
```

---

## Kapsam notlari (onarim sonrasi eklendi)

**Reset'teki Secure VTOR mimari bir sabit degildir.** Bu hedefte deger
`0x10000000`'dir ve QEMU v8.2.2 `hw/arm/mps2-tz.c` icindeki
`mmc->init_svtor = 0x10000000;` satirindan gelir. Baska bir Cortex-M33
cihazinda farkli olabilir. Bu yuzden startup kodu VTOR'u her halukarda
acikca yazar; reset degerine guvenmez.

**Linker'in stack icin ayirdigi alan calisma zamani korumasi degildir.**
`__stack_size = 4K` ve ilgili `ASSERT` yalnizca **link anindaki** yerlesimi
denetler: stack bolgesi `.bss`/`.heap` ile cakisiyor mu? Calisma zamaninda
stack'in bu siniri asmasini engelleyen hicbir sey yoktur. Runtime korumasi
ayri mekanizmalardan gelir:

| Mekanizma | Ne yapar | Ders |
|---|---|---|
| `MSPLIM` / `PSPLIM` | Stack pointer guncellemesi siniri asarsa UsageFault | 04 |
| MPU | Bolge bazli erisim denetimi (genel bellek) | 06 |

**Harici kesmeler (external IRQ) su an KAPALI kalmalidir.** Vector table'da
cihaza ozel IRQ girdileri henuz tanimli degil (bkz. `platform/mps2_an505.h`
icindeki `IRQn_Type`). Bir cevre birimi kesmesi etkinlestirilirse islemci
tanimsiz bir vector girdisine dallanir. IRQ girdileri ders 07'de, AN505
uygulama notuna karsi dogrulandiktan sonra eklenecek.

## Lesson 16: ayrı iki imaj yerleşimi

Yukarıdaki 32 KiB Secure RAM düzeni ortak tek-imaj platformunun varsayımıdır.
[Lesson 16](../lessons/16-trustzone-advanced/) kendi linker script'leriyle fiziksel
SRAM'i Secure/NS olarak iki 16 KiB yarıya böler; NS kod 0x00080000'da, NSC
veneers 0x10070000'da bulunur. MPC blok boyutu runtime okunur. Önceki derslerin
paylaşılan linker/startup'ı değişmez; iki imajın aynı fiziksel belleğe çakışmadan
yerleşmesi ve attribution ayrı ayrı kurulur.
