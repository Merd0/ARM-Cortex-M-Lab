**Dil:** [English](quick-check.md) | Türkçe

# Lesson 07 — Quick Check Cevapları

> Önce kendi cevabını yaz, sonra aç.

---

### 1. Exception ile external interrupt aynı şey midir?

Hayır — ama **aynı mekanizmayı** kullanırlar.

`Exception` üst kümedir. İçinde iki grup var:

- **System exception'lar**: mimari tarafından tanımlı ve numaralandırılmış —
  Reset (1), NMI (2), HardFault (3), MemManage (4), BusFault (5),
  UsageFault (6), SecureFault (7), SVCall (11), DebugMonitor (12),
  PendSV (14), SysTick (15).
- **External interrupt'lar**: cihaza özel. IRQ *n*'in exception numarası
  `16 + n`'dir ve vector table'daki `[16 + n]` girdisini kullanır.

**Ortak yanları:** Reset dışındaki bütün exception'lar aynı giriş/çıkış
yolundan geçer — pending →
kabul → vector lookup → donanımsal yığma → Handler Mode → `EXC_RETURN` →
geri yükleme. Bu derste SVCall üzerinden ölçtüğümüz her şey bir cihaz IRQ'su
için de geçerlidir.

**Farkları:** kimin tanımladığı (mimari vs. çip üreticisi), nasıl
tetiklendiği (komut/hata vs. donanım sinyali), ve hangi register ailesiyle
yapılandırıldığı (`SHPR*` vs. `NVIC` register'ları).

Günlük konuşmada "interrupt" geniş kullanılabilir; mimari anlatımda ayrım
korunmalıdır.

---

### 2. Vector table'ın 0. girdisi neden handler adresi değildir?

Çünkü reset anında henüz **geçerli bir stack yoktur**. Donanım, ilk komutu
çalıştırmadan önce bir `MSP` değerine ihtiyaç duyar ve onu `vector[0]`'dan
okur.

Sıra şu: `vector[0]` → `MSP`, sonra `vector[1]` → `PC`.

Bu build'de ölçülen:

```text
[0] initial MSP  0x30008000     <- cift sayi
[1] Reset        0x10000535     <- tek sayi (Thumb biti)
[11] SVCall      0x10000041     <- tek sayi
```

`vector[0]` bir **handler adresi değildir**; başlangıç `MSP` değerini taşır.
`0x30008000` çift, çünkü bir **stack pointer değeri**: word/8-byte hizalıdır ve
Thumb biti kavramı ona uygulanmaz. (Bir bellek adresidir — sadece dallanma
hedefi değildir.)

Reset'in kendisi de özel bir durum: `Reset_Handler` olağan bir handler gibi
Handler Mode'da değil, **ayrıcalıklı Thread Mode**'da başlar. Bu build'de
ilk komutunda ölçülen: `IPSR = 0`, `LR = 0xffffffff`, `MSP = 0x30008000`
(`vector[0]` ile aynı). Yığılmış bir frame yoktur, `EXC_RETURN` yoktur,
dönülecek bir bağlam yoktur.

Bu yüzden "vector table'daki bütün değerler tektir" kuralı **yanlıştır**.
İki istisna: `vector[0]` ve ayrılmış (reserved) girdiler — sonuncular `0`
olabilir.

---

### 3. Hangi register'lar basic frame'e otomatik yazılır? Neden tam olarak onlar?

Sekiz word, en düşük adresten başlayarak:

```text
+0x00 R0   +0x04 R1   +0x08 R2   +0x0C R3
+0x10 R12  +0x14 LR   +0x18 PC   +0x1C xPSR
```

**Neden tam olarak bunlar?** Sekiz word iki farklı gruptan oluşur:

- **`R0-R3` ve `R12`** — AAPCS'de (ders 03) normal bir çağrıda çağrılan
  tarafın serbestçe bozabildiği **volatile (caller-clobbered)** çalışma
  register'ları. Donanım onları kaydettiği için handler **sıradan bir C
  fonksiyonu** gibi yazılabilir: derleyici `r4-r11`'i zaten kendi prologunda
  korur (callee-saved), `r0-r3`/`r12`'yi ise serbestçe kullanabilir.
- **`LR`, `PC`, `xPSR`** — bunlar bir ABI kategorisi yüzünden değil, işlemcinin
  **kesilen kontrol akışını ve mimari durumu** geri yükleyebilmesi için
  yığılır: nereye dönülecek, hangi bayraklar, hangi yürütme durumu.

Yani frame "AAPCS'in caller-saved kümesi" **değildir**. Doğru özet: sıradan
kodu sürdürmek için en acil gereken volatile çalışma register'ları, artı
kesilen yürütmeyi devam ettirmek için gereken kontrol akışı/durum bilgisi.

Bu düzen, exception mekanizmasının ABI ile uyumlu çalışmasını sağlar.

Bu dersteki ölçümde `SVC`'den önce yüklenen işaret değerleri frame'de
birebir duruyordu (`0x11111111` `+0x00`'da, `0xCCCCCCCC` `+0x10`'da).

---

### 4. Thread PSP kullanırken `SVC` oluşursa frame hangi stack'e gider?

**PSP'ye** — yani exception anında **aktif olan** stack'e.

Ölçüm: `SVC` öncesi `PSP = 0x300002F0`, handler girişinde
`PSP = 0x300002D0`. Tam 32 byte (8 word) aşağı.

Bunu `EXC_RETURN` bit 2 teyit eder: `0xFFFFFFFD` → bit 2 = 1 → frame PSP'de.

Deney 1'de MSP kaynaklı durumda `EXC_RETURN = 0xFFFFFFF9` (bit 2 = 0) ve
frame MSP'ye yazıldı. Kural aynı: **aktif olan stack**.

---

### 5. Handler hangi stack'i kullanır? Yazılım mı seçer, donanım mı?

**MSP**, ve seçimi **donanım** yapar.

Exception girişinde işlemci `CONTROL.SPSEL`'i temizler. Ölçüm:

```text
Thread'de:   CONTROL = 0x2   (SPSEL = 1, PSP aktif)
Handler'da:  $sp == $msp == 0x30007ff0
```

Hiçbir yazılım satırı `SPSEL`'i temizlemedi.

**Tasarımın sebebi:** handler kodu, hatalı bir Thread bağlamının stack'ine
bağımlı olmamalıdır. Thread stack'i taşmış veya bozulmuş olsa bile handler
temiz bir MSP üzerinde çalışır.

Bu soru ile soru 4 arasındaki fark: **frame PSP'de olabilir ama handler
MSP kullanır.** İkisi ayrı sorudur.

(Tek güvenlik durumu varsayımıyla. TrustZone'da bu, *mevcut güvenlik
durumunun* MSP'sidir — ders 16.)

---

### 6. Handler girişinde `LR` neden normal bir dönüş adresi değildir?

Çünkü handler'a bir `BL` ile gelinmedi. Donanım, handler'ı çağırırken `LR`'ye
bir dönüş adresi yerine **`EXC_RETURN`** adı verilen özel bir değer yazar.

Bu değer bir adres değildir; üst 25 biti sabittir (`0xFFFFFF80` deseni) ve alt
bitleri dönüşün **nasıl** yapılacağını tarif eder:

```text
bit3 Mode   1 = Thread'e donus,  0 = Handler'a donus
bit2 SPSEL  1 = frame PSP'de,    0 = frame MSP'de
bit4 FType  1 = standart frame,  0 = genisletilmis (FP)
```

Ölçülenler: PSP kaynaklı `0xFFFFFFFD`, MSP kaynaklı `0xFFFFFFF9`.

Pratik sonuç: handler içinde bir `bl` çağrısı yapılırsa `LR` ezilir ve
`EXC_RETURN` kaybolur. Bu derste sarmalayıcının **ilk işi** onu kaydetmek —
ve bu tam olarak ders 04'te `cm-frame`'in onarılmasına yol açan meselenin
diğer yüzü.

---

### 7. `BX LR` nasıl exception return başlatabilir?

İşlemci, `PC`'ye yazılan değerin `EXC_RETURN` kodlamasında olup olmadığına
bakar. Öyleyse bunu bir dallanma hedefi olarak **yorumlamaz**; exception
return işlemini yürütür.

Bu derste tek adımda ölçüldü. `bx lr` öncesi:

```text
$lr = 0xfffffffd
Mode: Handler Mode (exception 11), Stack: MSP
```

Tek bir `si` sonrası:

```text
Mode: Thread Mode, Stack: PSP (0x300002f0)
r0  = 0x11111112   <- handler'in frame'e yazdigi yeni deger
r1  = 0x22222222   <- degismeden geri yuklendi
r12 = 0xcccccccc   <- degismeden geri yuklendi
```

Tek komut şunların hepsini yaptı: `IPSR` 11 → 0, aktif stack MSP → PSP, sekiz
word geri yükleme, `PC` yığılmış değere atlama.

Not: exception return'ü başlatan şey `BX` komutunun kendisi değil,
**`EXC_RETURN` değerinin `PC`'ye yazılmasıdır**. `POP {..., pc}` ile de olur —
ders 05'teki dispatcher bunu kullanıyordu.

---

### 8. `Pending` ile `Active` farkı nedir?

```text
Inactive --(olay/istek)--> Pending --(kabul)--> Active --(donus)--> Inactive
```

- **Pending:** exception talep edildi, henüz çalışmıyor.
- **Active:** handler'ı şu anda çalışıyor (veya iç içe bir exception
  tarafından kesilmiş durumda).

Bir exception aynı anda hem pending hem active olabilir: handler çalışırken
aynı exception yeniden talep edilirse.

**Pending her zaman "hemen çalışacak" demek değildir.** Beklemesinin sebepleri
maskeleme, eşit veya daha yüksek öncelikli bir exception'ın aktif olması, ya
da mimari uygunluk koşulları olabilir.

Active durumunu bu derste ölçtük — `SHCSR` register'ından:

```text
MemManage etkinken:  SHCSR = 0x00010001  (bit0 MEMFAULTACT = 1)
MemManage kapaliyken: SHCSR = 0x00000004  (bit2 HARDFAULTACT = 1)
```

İki farklı handler aktif, aynı ihlalden.

---

### 9. Daha düşük bir öncelik sayısı ne demek?

**Daha yüksek aciliyet.** Sayı küçüldükçe öncelik artar.

Sabit ve programlanamaz olanlar:

```text
Reset      -3   (en yuksek)
NMI        -2
HardFault  -1
```

Geri kalan system exception'lar ve bütün external IRQ'lar yapılandırılabilir
ve varsayılan olarak `0`'dır — yani HardFault'tan daha düşük öncelikli.

Bu yüzden bir fault handler'ı çalışırken aynı öncelikli başka bir exception
onu kesemez; ama NMI keser.

Sezgiye ters gelebilir ("1. öncelik" günlük dilde en önemlidir). Mimaride
öncelik bir **sıra numarası** gibi düşünülmeli: 0 önce gelir.

---

### 10. 8 öncelik biti neden evrensel bir Cortex-M33 gerçeği değildir?

Çünkü **uygulanan öncelik biti sayısı bir uygulama seçeneğidir** — çip
üreticisi çekirdeği sentezlerken belirler. Mimari yalnızca bir üst sınır
tanımlar.

Bu hedefte ölçüldü:

```text
NVIC_IPR0_READBACK       0x000000FF
NVIC_PRIO_BITS_MEASURED  8
NVIC_PRIO_BITS_COMPILED  8
NVIC_PRIO_BITS_MATCH     YES
```

Ölçüm yöntemi: bir öncelik register'ına `0xFF` yazıp geri okumak. Uygulanmayan
**alt** bitler sıfır okunur.

Gerçek MCU'larda 2, 3 veya 4 bit çok yaygındır. Yani 8 bit varsayan kod başka
bir cihazda sessizce yanlış öncelik atar — `0x20` yazdığın yerde donanım
`0x00` görür.

> **TARGET CONFIG** ile **ARCHITECTURE** ayrımı: "8 bit" bu QEMU hedefinin
> konfigürasyonudur, mimari bir sabit değil. Bu repo'da `__NVIC_PRIO_BITS`
> bu yüzden ölçüme dayandırıldı ve `capability-probe` her çalıştırmada
> derleme zamanı değeriyle karşılaştırıyor.

---

### 11. (Reasoning) `cm-frame $sp ...` handler içinde neden otomatik doğru değildir?

İki bağımsız sebep, ikisi de bu derste ölçüldü.

**Sebep 1: frame başka bir stack'te olabilir.** Thread PSP kullanıyorsa frame
PSP'ye yığılır, ama handler MSP üzerinde çalışır. Bu durumda `$sp` (= MSP)
frame'in bulunduğu bölgeyle hiç ilgili değildir:

```text
$sp / $msp    = 0x30007ff0
frame ($psp)  = 0x300002d0
```

**Sebep 2: aynı stack olsa bile handler kendi verisini yığar.** Handler'ın
prologu ve yerel değişkenleri `SP`'yi frame'in başından uzaklaştırır. Ölçüm:

```text
SVC_Handler ilk komut:   $sp = 0x30007ff0
svc_dispatch icinde:     $sp = 0x30007fe0    <- 16 byte asagi
```

MSP kaynaklı bir exception'da frame `0x30007FC0`'daydı. Stack
full-descending olduğu için handler'ın prologu ve yerel verisi MSP'yi **daha
da aşağı** itti; handler'ın stack kullanımı kaydedilmiş frame'in **altında,
daha düşük adreslerde** büyüdü (dispatcher yerel verisi `0x30007FB0`'da) —
`$sp` ile frame arasında hiçbir sabit ilişki
yok.

Bu yüzden `cm-frame` frame adresini **argüman olarak ister** ve `$sp`'den
tahmin etmez. Bağımsız denetim, eski sürümün tam olarak bu yüzden alakasız
belleği "exception frame" diye çözdüğünü ve `PC = 0` gibi uydurma değerler
bastığını bulmuştu.

Doğru yöntem: handler'a **girer girmez** `LR`'yi kaydet, `EXC_RETURN` bit 2'ye
bak, giriş MSP'sini veya PSP'sini kullan. Bu dersin sarmalayıcısı tam olarak
bunu yapıyor.

---

### 12. (Reasoning) MemManage devre dışıyken MPU ihlali neden başka exception'a dönüşür?

Çünkü bir fault **kaybolmaz**. Mimari, o fault için atanmış handler
kullanılamıyorsa onu **HardFault'a yükseltir** (escalation).

Bu derste aynı ihlal iki kez ölçüldü:

| | MemManage ETKİN | MemManage DEVRE DIŞI |
|---|---|---|
| `IPSR` | 4 | **3** |
| `MMFSR` | `0x82` | `0x82` — **aynı** |
| `MMFAR` | `0x30000000` | `0x30000000` — **aynı** |
| `HFSR` | `0x00000000` | `0x40000000` |
| `HFSR.FORCED` | 0 | **1** |
| Yığılmış PC | `0x10000110` | `0x10000110` — **aynı** |

**`MMFSR`'a hiçbir şey olmaz.** Durum register'larını yazan donanım, hangi
handler'ın çağrılacağından bağımsız çalışır. Yani escalation olmuş bir
HardFault'ta bile `CFSR` ve `MMFAR` gerçek sebebi söyler — teşhis için önce
`HFSR.FORCED`'a, sonra `CFSR`'a bakılır.

`HFSR.FORCED = 1` tam olarak şunu anlatır: *"bu HardFault kendi başına
oluşmadı; başka bir fault yükseltildi."*

Escalation'ın diğer sebepleri de vardır (örneğin bir fault, eşit veya daha
yüksek öncelikli bir handler çalışırken oluşursa). Ders 02'deki `STKERR`
vakasında `FORCED = 1`'i başka bir sebeple görmüştük. Ayrıntı ders 08'de.

---

### 13. (Reasoning) Handler `r4-r11`'i kaydetmeli mi? Donanım neden yığmıyor?

**Donanım yığmaz, ama handler'ın da özel bir şey yapmasına gerek yoktur** —
çünkü işi derleyici zaten yapar.

`r4-r11` AAPCS'de **callee-saved**'dir (ders 03). Handler bir C fonksiyonuysa,
derleyici onları kullanacaksa prologunda push eder, epilogunda pop eder. Yani
kesilen bağlamın `r4-r11` değerleri korunur.

Donanımın onları yığmaması bilinçli bir tasarım kararı: **exception latency**.
Daha büyük bir bağlam yığmak giriş/çıkış işini ve gecikmeyi artırabilir — ama
toplam gecikme uygulamaya ve başka mimari/mikro-mimari etkenlere bağlıdır;
"16 word = tam iki kat gecikme" diye bir hesap çıkmaz. Mimari, ABI'nin zaten
garanti ettiği şeyi tekrarlamıyor.

Ne zaman elle ilgilenmen gerekir? **Assembly handler yazarken.** Bu dersin
sarmalayıcısı `r0-r3` ve `r12`'yi serbestçe kullanıyor (donanım yedekledi) ama
`r4-r11`'e hiç dokunmuyor. Dokunsaydı kendisi push/pop etmek zorunda kalırdı.

Ders 15'teki context switch bu boşluğu bilerek doldurur: PendSV handler'ı
`r4-r11`'i **elle** yığar, çünkü görevler arası geçişte donanımın yığdığı
sekiz word yeterli değildir.

---

## Exercise 02 — `EXC_RETURN` sınıflandırma cevapları

| Değer | Dönülen mod | Frame nerede | Frame türü | `cm-frame` destekliyor mu? |
|---|---|---|---|---|
| `0xFFFFFFF9` | Thread | MSP | basic (FType=1) | ✅ Evet |
| `0xFFFFFFFD` | Thread | PSP | basic (FType=1) | ✅ Evet |
| `0xFFFFFFED` | Thread | PSP | **genişletilmiş** (FType=0) | ❌ `[DESTEKLENMIYOR]` |
| `0xFFFFFFF1` | **Handler** | MSP | basic | ✅ Evet |
| `0xFFFFFFF5` | — | — | — | ❌ `[BICIMSIZ EXC_RETURN]` |

Son satır önemli: `0xFFFFFFF5` bit 3 = 0 (Handler'a dönüş) **ve** bit 2 = 1
(frame PSP'de) demek olurdu. Handler Mode her zaman Main stack kullandığı için
böyle geçerli bir kodlama **yoktur**; `cm-frame` bunu biçimsiz olarak
reddeder ve "Thread'e dönüş" diye raporlamaz.

`0xFFFFFFED` ise **mimari olarak geçerlidir** — yalnızca bu laboratuvarın
çözücüsü genişletilmiş FP frame'ini desteklemiyor. Bu ayrım önemli:
"geçersiz" ile "desteklenmiyor" farklı şeylerdir.

Kendin kontrol et:

```gdb
(gdb) cm-exc-return 0xFFFFFFED
```

---

## Referanslar

- Armv8-M ARM (Arm DDI 0553) — exception modeli, `EXC_RETURN`, escalation
- Armv8-M Exception Model User Guide (Arm 107706) — exception numaraları,
  `EXC_RETURN` bit alanları
- Cortex-M33 Devices Generic User Guide (Arm 100235) — `SHCSR`, öncelik
  register'ları
- AAPCS32 — volatile / callee-saved register ayrımı
- [`docs/DEBUGGING.md`](../../../docs/DEBUGGING.tr.md) — `cm-frame` /
  `cm-exc-return` kullanımı ve sınırları

---

[← Ders 07](../README.tr.md) · [Ders 08 →](../../08-faults/README.tr.md)
