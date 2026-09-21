# Lesson 04 — Quick Check Cevapları

> Önce kendi cevabını yaz, sonra aç.

---

### 1. Full-descending stack ne demek? İki kelimenin her biri neyi söylüyor?

İki ayrı bilgi, tek terimde birleşmiş:

| Kelime | Ne söylüyor |
|---|---|
| **full** | `SP`, son yazılan **dolu** slotu gösterir — bir sonraki boş slotu değil |
| **descending** | Stack yüksek adresten aşağı büyür; `PUSH` `SP`'yi azaltır |

Alternatifleri de var (empty-ascending gibi) ve bazı mimariler onları kullanır.
Cortex-M'de seçim mimari tarafından sabitlenmiştir: full-descending.

Boş bir stack'te `SP`, kullanılabilir alanın **bir ötesini** gösterir — yani
o anda hiçbir geçerli veriyi göstermez. "SP her zaman anlamlı bir veri
gösterir" **yanlıştır**; bu yalnızca stack boş değilken doğrudur.

`SP`'nin altındaki alan da tahsis edilmiştir ama içeriği tanımsızdır: daha
önce itilip çıkarılmış verilerin kalıntısıdır.

---

### 2. `PUSH {r4, r5}` `SP`'yi ne kadar oynatır? Hangi register daha düşük adreste?

`SP` **8 byte azalır** (iki register × 4). Gözlemlenebilir sonuç:

```text
PUSH {r4, r5}:   SP yeni = SP eski - 8
                 [SP yeni + 0] = r4      <- dusuk numarali register, dusuk adres
                 [SP yeni + 4] = r5

POP  {r4, r5}:   r4 = [SP + 0]
                 r5 = [SP + 4]
                 SP yeni = SP eski + 8
```

Kural: **en düşük numaralı register en düşük adrese.** Yazdığın sıra
(`{r5, r4}`) hiçbir şeyi değiştirmez; assembler listeyi zaten sıralar.

Dikkat: mimari bunu bir *etkin adres* hesabı olarak tanımlar. "Önce SP iner,
sonra yazılır" bir mikro-işlem garantisi değil, net etkinin kabaca
anlatımıdır. Emin olabileceğin şey yukarıdaki son durumdur.

Bu, stack'i kurarken `SP`'ye vereceğin başlangıç adresini belirler. `uint32_t stack[128]` için doğru tepe adresi
`&stack[128]`'dir — yani **dizinin bir ötesi**. İlk `PUSH` önce 4 azaltıp
`&stack[127]`'ye yazar; dizi sınırı aşılmaz.

`&stack[127]` yazsaydın: ilk `PUSH` `&stack[126]`'ya yazardı — dizinin
**içine**, üstüne değil. Yani taşma olmazdı; iki şey kaybederdin. Birincisi
kapasite: son eleman (`stack[127]`) hiç kullanılmaz. İkincisi ve daha
önemlisi **hizalama**: `&stack[128]` 8 byte hizalıysa `&stack[127]` değildir,
ve çağrı sınırında `SP % 8 == 4` olur — AAPCS ihlali.

Asıl tehlikeli hata `&stack[0]` (taban) yazmaktır: ilk `PUSH` dizinin
**altına** yazar ve komşu değişkenleri ezer.

---

### 3. `PSP`'ye değer yazmak onu neden aktif stack yapmaz?

Çünkü bunlar iki ayrı soruya verilmiş iki ayrı cevap:

| Soru | Cevabı tutan yer |
|---|---|
| PSP'nin **içeriği** ne? | `PSP` register'ı |
| `SP` hangi register'ı **gösteriyor**? | `CONTROL.SPSEL` (bit 1) |

`__set_PSP(x)` birincisini değiştirir, ikincisine dokunmaz. Aktif seçimi
yapan `CONTROL.SPSEL`'dir.

Derste bunu ölçtük — aşama 2'de `$psp = 0x30000200` ama `$sp` hâlâ
`$msp` ile aynı (`0x30007fd0`), ve yerel değişkenlerin adresi hiç
değişmedi.

Bunun pratikteki karşılığı, sıranın **zorunlu** olmasıdır:

1. Önce `PSP`'ye geçerli bir adres yaz.
2. Sonra `SPSEL`'i 1 yap.

Ters sırada yaparsan, `SPSEL = 1` olduğu anda `SP` sıfır (veya çöp) değerli
`PSP`'yi gösterir ve bir sonraki `PUSH` geçersiz adrese yazmaya çalışır. Ders
02'deki Break It tam olarak buydu.

---

### 4. Reset sonrası Thread Mode hangi stack'i kullanır? Kod yazmadan nereden bilirsin?

**MSP.**

Kod yazmadan bilmenin iki yolu var:

1. **`CONTROL`'ün reset değeri.** Mimari, reset sonrası `CONTROL = 0`
   olduğunu söyler. `SPSEL` bit 1'dir, yani 0 → MSP.
2. **Vector table'ın ilk girdisi.** Reset'te donanım, vector table'ın 0.
   girdisini (`0x00000000` offset'indeki word) okuyup **MSP**'ye yükler.
   PSP için böyle bir mekanizma yoktur — çünkü reset sırasında PSP'nin
   kullanılması diye bir durum yoktur.

İkincisini ders 02'de ölçmüştük: `vector[0] = 0x30008000` ve reset'te
`MSP = 0x30008000`.

Mantıklı olan da bu: PSP'yi kurmak yazılımın işidir, ve reset anında henüz
hiçbir yazılım çalışmamıştır. Sistem bir stack'le açılmak zorunda, o da MSP.

---

### 5. Thread Mode PSP kullanırken exception oluşursa: Handler hangi SP'yi kullanır, donanım nereye yazar?

**İki ayrı cevap, ve karıştırılması yaygın bir hatadır.**

| Soru | Cevap |
|---|---|
| Donanımın ittiği durum (exception frame) nereye yazılır? | **PSP'ye** — kesilen kodun kullandığı stack'e |
| Handler hangi SP üzerinde çalışır? | **MSP'ye** — çünkü giriş sırasında `SPSEL` temizlenir |

Sıra şöyle: işlemci önce kesilen bağlamı **o an aktif olan** stack'e yazar
(burada PSP), sonra Handler Mode'a geçer ve `SPSEL`'i temizler. Yani frame
PSP'de kalır, handler MSP'de çalışır.

Derste bunu ölçtük: fault anında `Stack: MSP (0x30007fd0)` ve
`$control = 0` — oysa fault'tan hemen önce `SPSEL = 1` idi. `SPSEL`'i
yazılım temizlemedi; **donanım** temizledi.

Tasarımın sebebi: handler kodu, hatalı bir görevin stack'ine bağımlı
olmamalıdır. Görev stack'i taşmışsa veya bozulmuşsa handler yine de temiz bir
MSP üzerinde çalışmaya devam eder.

İki uyarı:

**1. Yazmaya çalışmak ile yazmış olmak aynı şey değildir.** PSP geçersizse
veya `PSPLIM` sınırını aşıyorsa stacking'in kendisi fault üretir
(`BFSR.STKERR`, `UFSR.STKOF`, `MMFSR.MSTKERR`). Bu derste her iki durumu da
ölçtük. O yüzden `cm-frame <adres> <EXC_RETURN>` bu bitleri görünce çözmeyi
**reddediyor**: bellekte geçerli bir frame olmayabilir. Komutun bir frame'i
kabul etmesi de onun gerçek olduğunu doğrulamaz; komut geçersiz durumları
eleyen sınırlı bir çözücüdür.

**2. Bu açıklama tek güvenlik durumu içindir.** TrustZone devredeyken hangi
stack'in kullanılacağı güvenlik durumu geçişine de bağlıdır (ders 16).

---

### 6. (Reasoning) Her C fonksiyonu mutlaka stack kullanır mı?

**Hayır.** Bu derste iki karşıt örneği birlikte ölçtük.

Stack kullanan (`frame_using_function`):

```text
10000070:  b086    sub   sp, #24
10000074:  9305    str   r3, [sp, #20]
```

Stack'e hiç dokunmayan (`active_stack_name`):

```text
10000054:  f3ef 8314   mrs   r3, CONTROL
10000058:  f013 0f02   tst.w r3, #2
1000005e:  4802        ldr   r0, [pc, #8]
10000060:  4770        bx    lr
```

İkincisinde ne `push` var ne `sub sp`. Fonksiyon tamamen register'larla
çalışıyor.

Bir fonksiyonun stack'e dokunmasını gerektiren üç durum var:

1. **Başka fonksiyon çağırıyorsa** — `BL`, `lr`'yi ezer, saklanması gerekir.
2. **Callee-saved register kullanıyorsa** — `r4-r8`, `r10`, `r11` teslim
   alındığı gibi geri verilmeli (ders 03).
3. **Yerel durumu register'lara sığmıyorsa** — fazlası stack'e taşar (spill).

Üçü de yoksa stack'e hiç dokunulmaz.

Önemli ek: bu, **bu derleyici, bu kaynak ve bu optimizasyon seviyesi** için
doğrudur. `-Og`'de stack'e taşan bir değişken `-O2`'de tamamen register'da
kalabilir; başka bir derleyici tamamen farklı kod üretebilir. Yani "bu
fonksiyon stack kullanıyor mu?" sorusunun cevabı **kaynak koda değil,
üretilen makine koduna** bakılarak verilir. Doğru refleks: `objdump`.

Terim notu: **leaf function**, başka fonksiyon *çağırmayan* fonksiyondur —
"stack kullanmayan" değil. Bir leaf fonksiyon rahatlıkla `sub sp, #N` ile
yerel alan açabilir; `frame_using_function` tam olarak bunu yapıyor.

---

### 7. (Reasoning) 8 byte hizalama neden önemli olabilir?

AAPCS32, bir genel arayüz sınırında — yani normal bir fonksiyon çağrısı
anında — `SP`'nin 8 byte hizalı olmasını şart koşar. Sebepleri katmanlı:

**0. Önce bir ayrım.** Bu bir **ABI** (çağrı sözleşmesi) kuralıdır, komut
hizalama kuralı değildir. Cortex-M33'te `LDRD`/`STRD` **word (4 byte)**
hizalama ister; 8 byte istemez. İkisini karıştırmak yaygın bir hata — ve bu
dersin ilk sürümünde de yapılmıştı.

**1. Veri tipleri ve yapılar.** `double` / `int64_t` gibi 8 byte hizalama
isteyen tipler stack üzerinde yer alır. Çağrı sınırında `SP` 8 hizalı
olmazsa derleyicinin yerleşim varsayımları bozulur.

**2. ABI birlikte çalışabilirliği.** Derleyici, çağrılan fonksiyonun `SP`'sini
8 hizalı **varsayarak** kod üretir; ona göre offset hesaplar. Sen elle
yazılmış assembly'de bu varsayımı bozarsan, derleyicinin ürettiği koda yalan
söylemiş olursun — ve bu ders 03'teki callee-saved ihlali gibi sessizce yanlış
çalışır.

**3. Exception stacking.** Cortex-M, exception'a girerken frame'i hizalamak
için gerekirse araya bir dolgu (padding) word'ü ekler ve bunu `xPSR`'deki bir
bitle işaretler. Yani hizalama yalnızca senin kodunun değil, donanımın da
umursadığı bir şey.

`SP % 8 == 4` olan bir çağrı sınırında ne olur? **Doğrudan bir komut hatası
beklemek yanlış olur.** `LDRD`/`STRD` word hizalaması ister ve `SP % 8 == 4`
hâlâ word hizalıdır — yani bu tek başına bir `LDRD`/`STRD` fault'u üretmez.
Bozulan şey **ABI varsayımıdır**: çağrılan kod, kendi yerel yerleşimini
8 hizalı bir `SP` varsayarak hesaplar. Olası sonuçlar, 8 hizalama isteyen bir
tipin yanlış hizalanması, derleyicinin ürettiği offset'lerin beklenmedik
davranması, ya da **hiçbir şey olmaması**. Sonuncusu en tehlikelisidir —
kod testten geçer ve sorun ancak başka bir derleyici sürümünde veya başka bir
kod yolunda ortaya çıkar.

Pratik kural: bir `bl` çağrısına girerken `SP % 8 == 0` olsun. Bunun en kolay
yolu prolog'da itilen toplam baytı 8'in katı tutmaktır — bu yüzden
derleyicinin ürettiği prologlarda sık sık gereksiz görünen bir register daha
itildiğini görürsün. Bu derste `accumulate` (ders 03) ve `psp_start` hedefi
`thread_entry` aynı yöntemi kullanıyor.

Dikkat: bu, **her tek tek `PUSH`'un** çift sayıda register içermesi gerektiği
anlamına gelmez. Kural fonksiyonun çağrı sınırındaki net `SP` değeri
üzerinedir; arada tek register itip sonra dengelemek serbesttir.

---

### 8. (Reasoning) `CONTROL.SPSEL`'i C'nin ortasında değiştirirsen ne bozulur?

Derleyici, o fonksiyon için **MSP üzerinde** bir frame ayırmıştır ve yerel
değişkenlere `[sp, #offset]` ile erişen komutlar üretmiştir. `SPSEL`
değiştiğinde `SP` artık başka bir bölgeyi gösterir — **aynı offset'ler
tamamen başka adreslere denk gelir.**

`CONTROL` yazmak:

- mevcut frame'i yeni stack'e **taşımaz**,
- yeni stack'te bir frame **oluşturmaz**,
- derleyiciye bir şey **bildirmez**.

Sonuç: fonksiyonun geri kalanı, yeni stack'in tepesinin **üstüne** veya
rastgele yerlerine yazar. Bu dersin ilk sürümünde tam olarak bu oldu; `-O2`
ve `-Os` altında `task_stack`'in bir üstündeki word eziliyordu ve
`CHECK_REGION` FAIL veriyordu.

**Neden `volatile` düzeltmez?** `volatile` erişimlerin optimize edilip
silinmesini engeller; erişimin *hangi taban register'a göre* hesaplandığını
değiştirmez. Offset'ler yine `sp`'ye göredir.

**Neden fazladan `ISB` düzeltmez?** `ISB` bir **senkronizasyon** sorununu
çözer: bağlam değişikliğinin sonraki komutlarca görülmesini garanti eder.
Buradaki sorun senkronizasyon değil, **frame geçerliliği**. Zaten
`__set_CONTROL()` sarmalayıcısı kendi içinde `ISB` çalıştırıyor — yani
senkronizasyon zaten vardı ve hata yine de oluşuyordu.

**Neden `noinline` düzeltmez?** Fonksiyonun ayrı kalması, frame'inin hâlâ
MSP'de olduğu gerçeğini değiştirmez.

Doğru çözüm tek: geçişi derleyicinin frame varsayımı olmayan bir yerde
(küçük bir assembly parçası) yapıp **taze bir C fonksiyonuna** dallanmak.
O fonksiyon kendi prolog'unu yeni `SP` üzerinde kurar.

Bir ek ayrıntı: `CONTROL`'ü oku-değiştir-yaz ile güncellemek bit 0'ı
(`nPRIV`) korumak içindir. Ama dikkat — bu, ayrıcalıksız kodun `CONTROL`'e
`2` yazarak kendini yeniden ayrıcalıklı yapabileceği anlamına **gelmez**.
Ayrıcalıksız Thread Mode kendi `nPRIV` bitini temizleyemez; yetki yükseltme
yalnızca ayrıcalıklı bir exception yolundan (örneğin SVC handler'ı) geçer.
Bu ders 05'in konusu.

---

## Referanslar

- Armv8-M Architecture Reference Manual (Arm DDI 0553) — stack pointer
  banking, `CONTROL.SPSEL`, exception entry davranışı, `PSPLIM`/`MSPLIM`
- Cortex-M33 Devices Generic User Guide (Arm 100235) — `CONTROL`, MSP/PSP
- AAPCS32 (ARM-software/abi-aa) — 8 byte stack hizalaması
- CMSIS-Core (M) — `__set_PSP`, `__set_CONTROL` (kendi icinde `ISB` calistirir), `__ISB`

---

[← Ders 04](../README.md) · [Ders 05 →](../../05-privilege-svc/)
