# Lesson 10 — Quick Check Cevapları

> Önce kendi cevabını yaz, sonra aç. Sayılar `debug` build'inden
> (**OBSERVATION**); kurallar etiketli.

---

### 1. `.o` ile `.elf` arasındaki temel fark? `.o`'daki bir sembol değeri neyi ifade eder?

**ELF:** İkisi de ELF dosyasıdır, ama türleri farklı:

- **Relocatable (`.o`, `ET_REL`)**: section'lar, semboller ve relocation'lar
  taşır. Section'lara henüz son adres atanmamıştır.
- **Executable (`.elf`)**: section'lara gerçek adresler atanmış, semboller
  çözülmüş, relocation'lar uygulanmış; yükleyici için **program header'ları**
  (segment'ler) eklenmiş.

Bir `.o`'daki sembol değeri üç durumdan biridir:

| Durum | Değer | Ölçülen (`readelf -s l10_asm.S.obj`) |
|---|---|---|
| Tanımsız | Section `UND`, değer `0` — henüz tanım yok | `text_fn  00000000  NOTYPE  GLOBAL  UND` |
| Tanımlı | Tanımlandığı section'ın **başına göre ofset** | `local_helper  0000000f  FUNC  LOCAL  Ndx 1` |
| Son ELF'te | Section'ın yerleştiği adres + ofset | `local_helper  1000004f` |

"`.o`'da adresler hep `0`" **yanlış** olur. `local_helper` `.text` içinde
`0x0e` ofsetinde (`nm` `0e`; `readelf -s` `0f`, çünkü bit 0 Thumb biti).
ELF'te `.text` input section'ı `0x10000040`'a yerleşti: `0x10000040 + 0x0e`.

Bu derste birçok değerin `0` görünmesi `-ffunction-sections -fdata-sections`
yüzünden: her fonksiyon/nesne kendi section'ında, o section'ın başında.
`l10_asm.S`'te iki fonksiyon aynı `.text`'i paylaştığı için ikincisi `0x0e`'de.

Ölçüm: `main.c.obj`'da `bl 0 <text_fn>` (`.text.main` içinde ofset `2a4`);
ELF'te aynı komut `10000334: bl 10000494 <text_fn>`.

---

### 2. Sembol ile relocation arasındaki fark?

- **Sembol**: bir adrese verilen **isim** (ve bağlaması: LOCAL / GLOBAL,
  tanımlı / tanımsız).
- **Relocation**: "şu adresteki byte'ları, şu sembolün adresine göre, şu
  formülle **yamala**" talimatı.

`main.o`'da:

- `U text_fn` → "`text_fn` adında bir şey kullanıyorum ama tanımlamıyorum."
- `R_ARM_THM_CALL text_fn` @ `0x2a4` → "`.text.main`'in `0x2a4` ofsetindeki
  `BL`'yi, `text_fn`'in adresi belli olunca AAELF32'nin
  `((S + A) | T) – P` formülüyle doldur."

Biri **ne** eksik, diğeri **nereye ve nasıl** doldurulacak.

---

### 3. Section ile segment arasındaki fark?

| | Section | Segment |
|---|---|---|
| Kimin görüşü | Linker'ın (bağlama görünümü) | Yükleyicinin (yükleme görünümü) |
| ELF'te | Section header tablosu | Program header tablosu |
| Örnek | `.data`, `.bss` ayrı | İkisi tek bir `LOAD` segment'inde |

Ölçüm (`readelf -l`): iki `LOAD` segment'i. İlki `.vectors .text .rodata`
(FLASH, `R E`), ikincisi `.data .bss` (`VirtAddr 0x30000000`,
`PhysAddr 0x100009a8`, `RW`). `.o` dosyalarında segment yoktur.

---

### 4. (Reasoning) `.data`'nın VMA'sı ve LMA'sı neden farklı?

`.data` iki çelişen gereksinimi karşılamak zorunda:

- **İlk değer kalıcı olmalı** → güç kesilince silinmeyen bir yerde, FLASH'ta
  (**LMA** `0x100009A8`).
- **Değişken yazılabilir olmalı** → RAM'de (**VMA** `0x30000000`).

Kod `.data`'daki bir değişkene her zaman **VMA** üzerinden erişir. Startup
kodu çalışmanın başında LMA'dan VMA'ya kopyalar (ders 11).

İkisi **aynı** olsaydı — örneğin `.data` FLASH'ta — değişkene yazmak imkânsız
olurdu (FLASH normal store ile yazılamaz). İkisi RAM'de aynı olsaydı, ilk
değerler güç açılışında orada olmazdı.

Bu repo'da QEMU `.data`'yı **yalnızca LMA'ya** yükledi: `Reset_Handler`'ın ilk
komutunda RAM'de `g_init = 0`, FLASH'ta `0xC0DEFACE` ölçüldü.

---

### 5. `.data > RAM AT > FLASH` ile `.data : AT(__etext) > RAM` farkı?

**LINKER (GNU ld):**

- `AT > FLASH`: LMA'yı FLASH **bölgesinin içinde tahsis ederek** atar; bu
  byte'lar FLASH'ın kullanımına eklenir. Taşma → `region FLASH overflowed`.
- `AT(__etext)`: LMA'yı bir **adres** olarak söyler; hiçbir bölgeden
  düşmez. `.text` + `.rodata` + `.data` kopyası FLASH'ı aşsa bile link
  **sessizce** başarılı olabilir.

**Taşmayı yakalayan** `AT > FLASH`. Bu repo'nun eski sürümü `AT(__etext)`
kullanıyordu; bağımsız denetim bunu buldu (I5) ve `tests/build-probes.sh`
bunu artık kalıcı olarak test ediyor. Script'teki ikinci `ASSERT`
(`__flash_load_end <= ORIGIN(FLASH) + LENGTH(FLASH)`) aynı şeyi açık bir
mesajla söylüyor.

---

### 6. (Reasoning) "`.bss` ELF'te yok" neden yanıltıcı?

Çünkü `.bss` ELF'te **tarif ediliyor**, yalnızca **içeriği saklanmıyor**.

Ölçüm:

- Section başlığı var: `.bss  NOBITS  30000008  size 8  WA`.
  `NOBITS` = "dosyada içerik yok"; `A` = "bellekte yer kaplar".
- İkinci `LOAD` segment'i: `FileSiz 0x8`, `MemSiz 0x10`. Aradaki 8 byte
  `.bss` — yükleyiciye "bellekte bu kadar yer var, dosyada karşılığı yok".
- Çalışma zamanında RAM'de 8 byte.
- FLASH'ta **0** byte.

Doğru cümle: *"`.bss` ELF'te bir boyut ve adres olarak var; byte'ları yok."*

---

### 7. Map dosyasında bir sembol nereden geldi, nereye girdi?

Map'in *Linker script and memory map* bölümünde:

```text
.rodata         0x100005e4      0x3c4              <- CIKIS section'i
 .rodata.l10_table                                  <- INPUT section'i
                0x1000097c                k_custom   <- sembol, adres
```

Input section satırının devamında **katkı veren nesne dosyası** yazar
(örneğin `.data.g_init ... objects.c.obj`). Yani üç soru tek bakışta:
hangi çıkış section'ı, hangi input section, hangi `.o`.

Ayrıca: *Discarded input sections* (neler atıldı), *Memory Configuration*
(bölgeler), linker sembol atamaları (`__etext = LOADADDR (.data)`) ve
`ASSERT` satırları.

---

### 8. `KEEP` neden var? Vektör tablosu için neden gerekli?

`--gc-sections` bir input section'ı ancak **bir kökten erişilebiliyorsa**
tutar (giriş noktası, başka tutulan section'lardan gelen referanslar...).
`KEEP(...)` bir input section'ı **kök** yapar.

Vektör tablosuna hiçbir **kod** referans vermez — onu işlemci reset'te okur
(**ARCHITECTURE**). Linker bunu bilemez.

Ölçüldü (geçici kopya, repo'ya eklenmedi): `KEEP(*(.vectors))` yerine
`*(.vectors)` yazılınca `.vectors` map'te *Discarded* listesine girdi,
`.vectors` çıkış section'ı ELF'ten kayboldu ve firmware **hiç başlamadı**.

---

### 9. (Reasoning) `__attribute__((used))` bir section'ı GC'den korur mu?

**Bu toolchain'de (GCC 13.2.1 + binutils 2.42) hayır.**

- `used` **derleyiciye** söyler: "bu tanım kullanılmıyor gibi görünse de
  nesne dosyasına yaz."
- `KEEP` **linker'a** söyler: "bu input section'ı GC'de atma."

Ölçüm: `k_unused_used` (`used`) `objects.o`'da vardı (`nm`: `R k_unused_used`),
ama link'te `.rodata.k_unused_used` *Discarded* listesine girdi. Vektör
tablosu da `used` taşıyor — `KEEP` olmadan o da atıldı (soru 8).

`used` + `KEEP` birlikte gerekebilir: biri derleyicinin, diğeri linker'ın
silmesini engeller. GCC'nin `retain` niteliği ayrıca var; bu derste
denenmedi.

---

### 10. `-ffunction-sections` olmadan `--gc-sections` ne kadar etkili olur?

Çok daha az. GC'nin birimi **input section**'dır. Bu bayraklar olmadan bir
C dosyasının bütün fonksiyonları tek bir `.text` input section'ında toplanır;
o dosyadan **tek bir** fonksiyon kullanılsa bile hepsi tutulur.

Bu derste `unused_function` atılabildi, çünkü `placement.c`
`-ffunction-sections` ile derlendi ve kendi `.text.unused_function`
section'ında duruyordu.

Kontrast: `l10_asm.S` bir C dosyası değil; `-ffunction-sections` ona
uygulanmaz. Oradaki kod tek bir `.text` input section'ında.

---

### 11. `MEMORY` ve `SECTIONS` ne iş yapar?

- **`MEMORY`**: fiziksel bölgeleri tanımlar — ad, öznitelik, başlangıç, boyut.
  Bu repo'da `FLASH (rx) 0x10000000 512K`, `RAM (rwx) 0x30000000 32K`. Bir
  bölge taşarsa link hata verir.
- **`SECTIONS`**: input section'ları çıkış section'larına nasıl
  toplayacağını (wildcard'lar), hangi bölgeye koyacağını (`> FLASH`,
  `> RAM AT > FLASH`), hizalamayı ve linker sembollerini tanımlar.

`MEMORY` "nerede yer var", `SECTIONS` "ne nereye gider".

---

### 12. (Reasoning) ELF 94 KB, FLASH 2.5 KB — fark nerede? FLASH neden `.text`'ten büyük?

**ELF dosya boyutu** (94 316 byte) yüklenmeyen şeyleri de içerir:
`.debug_*` section'ları (`-g3` makro bilgisi dahil), sembol tablosu,
string tablosu, section/program header'ları. Bunların hiçbiri FLASH'a gitmez.
Aynı kaynak `minsize`'da (`-g`) 27 964 byte.

**FLASH kullanımı** (2480 byte) `.text`'ten (1444 byte) büyük, çünkü FLASH
yük görüntüsü şunların **hepsi**:

```text
.vectors      64
.text       1444
.rodata      964
.data kopyasi  8     <- .data'nin ilk degerleri
-----------------
            2480  = __flash_load_end - 0x10000000
```

Berkeley `size`'ın `text` sütunu (2472) da `.text` değil: `.vectors` +
`.text` + `.rodata`.

---

### 13. (Reasoning) "undefined reference to `asm_fn`" ama `asm_fn` tanımlı. Nasıl?

Tanım **var** ama **LOCAL**.

```text
nm l10_asm_broken.o   ->  00000000 t asm_fn     (kucuk 't')
nm main.o             ->           U asm_fn
```

GAS'ta bir etiket `.global` / `.globl` ile işaretlenmedikçe yereldir. Yerel
bir sembol yalnızca kendi nesne dosyası içinden görülür. `main.o`'nun `U`
referansı **başka bir dosyadaki GLOBAL** bir tanım arıyor; bulamayınca
linker "undefined reference" veriyor.

İlk refleks: tanımlandığı nesnede `nm` ile **bağlamasına** bak. Büyük harf mi,
küçük harf mi?

(Diğer yaygın sebepler: dosya hiç link'e verilmemiş, ad C++'ta mangled,
`static` tanımlanmış, ya da ad yanlış yazılmış.)

---

## Referanslar

- AAELF32 — relocation türleri ve formülleri, Thumb sembolleri, veneer'lar
- GNU ld — *Linker Scripts*, `--gc-sections`, `-Map`, `--print-memory-usage`
- GNU binutils — `readelf`, `nm`, `objdump`, `size`
- System V gABI — section'lar ve program header'ları

---

[← Ders 10](../README.md) · [Ders 11 →](../../11-startup/)
