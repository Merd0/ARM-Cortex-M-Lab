# Lesson 11 — Quick Check Cevapları

> Önce kendi cevabını yaz, sonra aç. Sayılar `debug` build'inden; her iddia
> etiketli.

---

### 1. `vector[0]` ve `vector[1]` reset'te ne olarak kullanılır?

- **`vector[0]`**: ilk MSP değeri. Bir **adres değeri**dir, fonksiyon değil.
  Bu repo'da `&__stack_top` = `0x30008000`. (**ARCHITECTURE** — Arm 107565:
  vektör tablosunun ilk word'ü MSP'nin başlangıç değeri.)
- **`vector[1]`**: reset vektörü. PC reset handler'ın başlangıç adresiyle
  yüklenir (DDI 0553 `RPLNS`) ve `EPSR.T` bu değerin bit 0'ından alınır
  (`RKSCH`). Bu repo'da `Reset_Handler`, tabloda `0x10000335`.

Tablonun geri kalanı (bu repo'da 2–15) iki farklı türden oluşur:

- **Handler girişleri** — 2 NMI, 3 HardFault, 4 MemManage, 5 BusFault,
  6 UsageFault, 7 SecureFault, 11 SVCall, 12 DebugMonitor, 14 PendSV,
  15 SysTick. Her biri bir handler **sembolünün** adresini taşır.
- **Ayrılmış girişler** — 8, 9, 10, 13. Bu repo'nun tablosunda **sabit `0`**;
  arkalarında sembol yok.

Handler'ların davranışı ders 07'nin konusu.

---

### 2. (Reasoning) `Reset_Handler`'ın ilk komutu `push`. Neden güvenli?

Çünkü MSP **ilk komut çalışmadan önce** donanım tarafından `vector[0]`'dan
yüklenmiştir (**ARCHITECTURE**). GDB durak A: `MSP = 0x30008000 =
vector[0] = &__stack_top`, PC henüz `Reset_Handler`'ın ilk komutunda.

`Reset_Handler` bir C fonksiyonu ve derleyici ona prolog koydu (`push {r3,
lr}` — **TOOLCHAIN OBSERVATION**). Stack kullanmak güvenli; güvenli
**olmayan** şey `.data`/`.bss`'e güvenmek olurdu — onlar henüz hazır değil.
Stack bir section değil: RAM'in tepesinde, `.data`/`.bss`'ten ayrı.

---

### 3. `vector[1] = 0x10000335`, komut adresi `0x10000334`. Fark ne?

Bit 0. Tabloda saklanan **giriş değeri** Thumb durumunu taşır; komutun
kendisi **hizalı** adreste (`0x10000334`) durur. Reset, PC'ye bit 0'ı
temizlenmiş adresi, `EPSR.T`'ye bit 0'ı yazar (`RKSCH`).

Bit 0 = 0 olsaydı `EPSR.T = 0` olurdu. DDI 0553 `RSQLX`: bu durumda
herhangi bir komutu çalıştırma girişimi, Main Extension'lı bir PE'de
**INVSTATE UsageFault** üretir. Reset sırasında bu çözülemez bir durumdur —
program hiç başlamaz.

"Reset_Handler tek bir adreste duruyor" ifadesi **yanlış** olur.

---

### 4. (Reasoning) `ENTRY(Reset_Handler)` silinse reset değişir mi?

**Hayır.** `ENTRY`, ELF başlığındaki `e_entry` alanını ayarlar
(**ELF / LINKER**). Onu debugger'lar ve bazı yükleyiciler okur. Cortex-M
işlemci reset'te `e_entry`'yi **okumaz**; PC'yi vektör tablosundan
(`vector[1]`) alır (**ARCHITECTURE**).

`ENTRY` silinse yalnızca `e_entry`'nin değeri değişebilir (GNU ld kendi
varsayılan kuralını uygular). Vektör tablosu `KEEP` sayesinde `0x10000000`'da
kaldıkça reset davranışı aynıdır. Ölçüm: `readelf -h` → `Entry point address`
= `Reset_Handler` sembol değeri; işlemci bunu hiç okumuyor.

---

### 5. `.data` neden kopyalanmalı, `.bss` neden sıfırlanmalı?

C şunu varsayar:

- `uint32_t g_l11_init = 0x11C0FFEE;` → program başladığında değer
  `0x11C0FFEE`.
- `uint32_t g_l11_zero;` → program başladığında `0`.

Değişkenler RAM'de yaşar (yazılabilir olmaları için). İlk değerler FLASH'ta
durur (güç kesilince silinmesinler diye). **Donanım ikisini birbirine
bağlamaz** ve RAM'in reset sonrası içeriği fiziksel donanımda tanımsızdır.

Bu yüzden startup:

- `.data`'yı `__etext`'ten (LMA) `__data_start`..`__data_end`'e (VMA) kopyalar,
- `.bss`'i `__bss_start`..`__bss_end` arasında sıfırlar.

Bu repo'da bunu paylaşılan `Reset_Handler` yapıyor (**REPOSITORY DESIGN**).

---

### 6. (Reasoning) `g_l11_zero` `main`'de sıfır. Kanıt mı?

**Hayır.** QEMU RAM'i sıfırla başlatıyor (**QEMU OBSERVATION**): sıfırlama
döngüsü hiç çalışmasa bile değer sıfır olurdu. `tests/build-probes.sh`
(L11b) bunu kalıcı olarak gösteriyor: `.bss` döngüsü **olmayan** bir startup,
zehirleme kapalıyken `CHECK_BSS_ZERO PASS` veriyor.

Gerçek kanıt iki yoldan:

1. **GDB (paylaşılan startup):** döngüden önce `g_l11_zero = 0xCAFEBABE`
   enjekte et, `watch g_l11_zero`, devam et. Ölçülen: `Old value =
   3405691582 → New value = 0`, yazan komut `Reset_Handler+28: str.w r2,
   [r3], #4`.
2. **Zehirleme (ders-yerel startup):** RAM önce `0xA5A5A5A5` ile doldurulur.
   Döngü eksikse `g_l11_zero = 0xA5A5A5A5` → FAIL.

---

### 7. Startup neden word word kopyalayabiliyor?

Linker script `.data` ve `.bss`'in başını ve sonunu 4'e hizalıyor
(`. = ALIGN(4)` satırları, `__data_start`, `__data_end`, `__bss_start`,
`__bss_end` atamalarının hemen önünde). Böylece:

- başlangıç adresleri word hizalı,
- uzunluklar 4'ün katı,
- `dst < __data_end` / `dst < __bss_end` koşulu tam sınırda durur.

Sınırlar hizalı olmasaydı word döngüsü son 1–3 byte'ı atlar veya komşu
belleğe taşardı. Bu, startup ile linker script arasındaki sözleşmenin bir
maddesi (**REPOSITORY DESIGN** + **LINKER**). Word kopyalama evrensel bir
kural değil.

---

### 8. `SystemInit` bu repo'da ne yapıyor? VTOR'u değiştiriyor mu?

`platform/mps2-an505/src/system_init.c`'deki **güçlü** `SystemInit`,
startup'taki **weak** boş tanımın yerini alıyor (map: weak olan *Discarded*).
İki şey yapıyor:

1. `SCB->VTOR = 0x10000000`. Ölçüm: öncesinde de sonrasında da
   `0x10000000`. **Görünür bir değişim yok**; adres açıkça beyan ediliyor.
2. `board_uart_init()`. Ölçüm: UART `BAUDDIV` `0` → `173`. Gözlenebilir etki
   bu; `board_uart_init`'in tek çağıranı `SystemInit`.

Saat/PLL kurulumu yok.

---

### 9. (Reasoning) SP mod 8 = 0 ve `SP = __stack_top − 8`: hangisi kural?

- **SP mod 8 = 0** `main` çağrılırken: **ABI** kuralı (AAPCS32, public
  interface'te SP 8'e hizalı). `Reset_Handler` bir C fonksiyonu ve AAPCS'e
  uyarak `main`'i çağırıyor.
- **`SP = __stack_top − 8`**: **TOOLCHAIN OBSERVATION**. GCC 13.2.1
  `Reset_Handler`'a `push {r3, lr}` (8 byte) koydu. Başka bir derleyici
  veya preset farklı bir prolog üretebilir; ABI yalnızca hizalamayı ister.

Ölçüm `main_entry.S` shim'iyle yapıldı: C prologu çalışmadan önce.

---

### 10. `main` dönerse ne olur?

`Reset_Handler`'ın sonundaki sonsuz döngüye girilir:

```text
0x10000362 <Reset_Handler+46>:  wfi
0x10000364 <Reset_Handler+48>:  b.n  0x10000362
```

`main`'in LR'si (`0x10000363`) tam bu `wfi`'yi gösteriyor. `11-startup`
bilerek döner; GDB durak G bu yolu gösteriyor. `exit()` yok, `argc/argv`
yok — bu hosted C değil (**REPOSITORY DESIGN**).

---

### 11. `NMI_Handler` neden `Default_Handler` ile aynı adreste?

Startup onu `__attribute__((weak, alias("Default_Handler")))` ile
tanımlıyor: aynı kodun ikinci, **weak** adı. Başka biri `NMI_Handler`
tanımlamadığı için linker weak alias'ı kullanıyor: `nm` → `W NMI_Handler`,
adres `Default_Handler` ile aynı.

`11-startup`'ta `main.c` bir **güçlü** `SysTick_Handler` tanımlıyor. Linker
güçlü tanımı seçiyor: `nm` → `T SysTick_Handler`, farklı adres, ve
`vector[15]` bu adresi (Thumb bitli) taşıyor. Bu bir **link zamanı**
mekanizması; kodda hiçbir `if` yok.

Bu mekanizma yalnızca **handler girişleri** için geçerli (2–7, 11, 12, 14,
15). Ayrılmış girişler (8, 9, 10, 13) tabloya sabit `0` olarak yazılmış;
arkalarında bir sembol olmadığı için weak alias da, override edilecek bir
şey de yok. `vector[0]` (bir stack pointer değeri) ve `vector[1]`
(`Reset_Handler`, güçlü bir tanım) da bu grubun dışında.

Ölçüm (`objdump -s -j .vectors`, üç preset): `vector[8]`, `[9]`, `[10]`,
`[13]` = `0x00000000`.

---

### 12. (Reasoning) `LR = 0xFFFFFFFF` ve `MSPLIM = 0`: hangisi mimari?

- **`LR = 0xFFFFFFFF`**: **ARCHITECTURE**. DDI 0553 `RPLRT`: Main Extension
  varsa LR, Warm reset'te `0xFFFFFFFF` olur. Cortex-M33 Main Extension'lı.
  QEMU da bunu gösteriyor — ama kaynağı Arm.
- **`MSPLIM = 0`**: **QEMU OBSERVATION**. Bu dersin okuduğu bölümlerde
  MSPLIM'in reset değeri yer almıyor; bu yüzden ders onu mimari diye
  sunmuyor. Ayrıca bu repo MSPLIM'i hiç programlamıyor — `__stack_limit`
  yalnızca bir linker sembolü.

Kural: bir değer emülatörde ölçüldüyse, birincil kaynakta görülmeden
"mimari" diye etiketlenmez.

---

## Referanslar

- Armv8-M ARM (Arm DDI 0553A.g) — `RPLNS`, `RKSCH`, `RSQLX`, `RCMQP`,
  `RHWFV`, `RMDXK`, `RPLRT`, `RXPWM`
- Arm 107565 — vektör tablosu ve ilk MSP
- AAPCS32 — public interface'te SP hizalaması
- Repo: `startup_mps2_an505.c`, `system_init.c`, `mps2-an505-secure.ld`

---

[← Ders 11](../README.md) · [Ders 12 →](../../12-debug-dwt/)
