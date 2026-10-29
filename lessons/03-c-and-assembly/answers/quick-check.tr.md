**Dil:** [English](quick-check.md) | Türkçe

# Lesson 03 — Quick Check Cevapları

> Önce kendi cevabını yaz, sonra aç.

---

### 1. `BL` ile `B` farkı nedir? `BX lr` yerine `B lr` yazsan ne olur?

`B` sadece dallanır. `BL` dallanır **ve** `lr`'ye dönüş adresini yazar —
yani bir sonraki komutun adresini (Thumb biti set edilmiş halde). Fark tek
kelime: `BL` dönüş yolunu bırakır, `B` bırakmaz.

`B lr` ise register'daki adrese dallanmaz — çünkü `B`'nin operandı bir
register değil, komutun içine gömülü bir sapma (immediate offset) değeridir.
Register'daki bir adrese dallanmak için `BX` (veya `BLX`) gerekir.

Bu repoda yapılan denemede GNU assembler
`b lr` satırını **reddetmez**. `lr`'yi bir register değil, bir **etiket adı**
olarak okur ve tanımsız bir sembole dallanma üretir:

```text
$ arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -c tb.S -o tb.o     # hata yok
$ arm-none-eabi-objdump -d tb.o
   0:   f7ff bffe       b.w     0 <lr>
$ arm-none-eabi-nm tb.o
         U lr                                                    # undefined!
```

Link aşamasında karşılığı: `undefined reference to 'lr'`. Yani hata assembler'da
değil linker'da çıkar ve mesaj ilk bakışta anlamsız görünür. "Assembler kabul
etti, demek ki doğru" çıkarımının neden tehlikeli olduğunun küçük bir örneği.

Bu ayrım, derleyicinin ürettiği kodda da görünür: `B` genelde fonksiyon içi
döngü ve `if` dallanmalarında, `BL` fonksiyon çağrılarında kullanılır. Bir
disassembly'de `BL` gördüğünde "burada bir fonksiyon çağrısı var ve `lr`
değişti" diye okuyabilirsin.

---

### 2. `lr` neden önemli? `add_numbers` itmiyor, `accumulate` itiyor — fark ne?

`lr`, fonksiyonun **tek** dönüş yoludur. Kaybedilirse geri dönülecek yer
kalmaz.

- `add_numbers` bir **leaf (yaprak) fonksiyondur**: başka hiçbir fonksiyon
  çağırmaz. Dolayısıyla çalıştığı süre boyunca `lr`'yi kimse değiştirmez;
  saklamaya gerek yoktur.
- `accumulate` ise içinde `bl helper` yapar. `BL` komutu **tanımı gereği**
  `lr`'yi yeni bir değerle ezer. Eğer `accumulate` kendi `lr`'sini önceden
  stack'e itmemiş olsaydı, `helper` döndükten sonra `accumulate`'in kendi
  dönüş adresi kaybolmuş olurdu.

Disassembly'de bu ayrım hemen görünür:

```text
add_numbers:   adds r0, r0, r1 / bx lr           <- push yok
accumulate:    push {r4, lr} / ... / pop {r4, pc} <- push var
```

`pop {r4, pc}` ayrıca küçük bir zarafet içerir: `lr`'yi geri yükleyip sonra
`bx lr` yapmak yerine, kaydedilmiş dönüş adresini doğrudan `pc`'ye yazıyor.
Tek komutla hem geri yükleme hem dönüş.

---

### 3. İki `int` argüman hangi register'lara gelir? Beşinci argüman nereye gider?

İlk dördü sırayla `r0`, `r1`, `r2`, `r3`. Yani iki `int` için `r0` ve `r1`.

Beşinci ve sonrakiler **stack'e** konur. AAPCS32 bunu bir tahsis algoritması
olarak tanımlar: argümanlar sırayla register'lara bağlanır, register'lar
tükendiğinde kalanı bellekte stack üzerinde geçirilir.

Pratik sonucu: dört argümandan sonrası ücretsiz değildir. Her ek argüman en az
bir `str` (yazma) ve callee tarafında bir `ldr` (okuma) demektir. Sıcak bir
döngüde çağrılan fonksiyonda bu fark ölçülebilir hale gelir. Çok argümanlı
fonksiyonlar yerine struct pointer geçirmenin yaygın olmasının bir sebebi budur.

Dikkat: bu sayım **word** bazlıdır. `char` bile tam bir register işgal eder;
buna karşılık `int64_t` iki register kullanır ve hizalama kuralları devreye
girer. Ayrıntı ders 09'da.

---

### 4. Dönüş değeri neden `r0`'dan alınabiliyor? Donanım mı, anlaşma mı?

**Anlaşma.** Donanımda `r0`'ı "dönüş değeri register'ı" yapan hiçbir şey yok;
`r0`, diğer genel amaçlı register'lardan farklı değildir.

Bu bir sözleşmedir (AAPCS32) ve iki tarafın da aynı sözleşmeyi bilmesiyle
çalışır: callee sonucu `r0`'a koyar, caller `r0`'dan okur. Ortada donanımsal
bir zorlama olmadığı için, sözleşmeyi ihlal eden kod da **sorunsuz derlenir** —
sadece yanlış çalışır.

`r0` seçilmesinin pratik sebebi: zaten birinci argümanın register'ı. Çoğu
fonksiyon ilk argümanı işleyip sonucu üretir, dolayısıyla sonuç genellikle
hâlihazırda `r0`'dadır ve fazladan bir `mov` gerekmez. `add_numbers`'daki
`adds r0, r0, r1` tam olarak bunu gösteriyor: argüman ve sonuç aynı yerde.

---

### 5. (Reasoning) Elle yazılmış assembly neden AAPCS'i görmezden gelemez?

"Benim fonksiyonum, istediğim register'ı kullanırım" argümanı iki yerde
kırılır.

**Birincisi: fonksiyonunu sen çağırmıyorsun.** Onu çağıran C kodunu derleyici
üretti ve derleyici AAPCS'e göre üretti. Argümanları `r0-r3`'e koydu, sonucu
`r0`'dan okuyacak. Senin fonksiyonun farklı bir yerden okursa hiçbir hata
mesajı almazsın — sadece çöp okursun.

**İkincisi: fonksiyonun tek başına yaşamıyor.** Linker, derleyicinin ürettiği
nesne dosyalarıyla seninkini yan yana koyuyor. İki nesne dosyasının
birleşebilmesinin ön koşulu aynı sözleşmeye uymalarıdır. ABI, tam olarak
"ayrı derlenmiş parçalar nasıl birlikte çalışır" sorusunun cevabıdır.

Daha incesi: sözleşme **iki yönlüdür.** `r0-r3`'ü istediğin gibi kullanabilirsin
(caller onları zaten kaybolmuş sayar). Ama `r4-r8`, `r10`, `r11` ve `SP`'yi
teslim aldığın gibi geri vermek zorundasın. `r9`'un rolü **platforma özeldir**:
statik taban (SB), thread register (TR) ya da fazladan bir korunan register
(v6) olabilir — hangisi olduğunu platform standardı söyler. Bu yüzden `r9`'u
her iki listeden de ayrı tutuyoruz. Break It bölümü ikinci yönün ihlaliydi.

Küçük bir onarım notu: bu dersin ilk sürümünde `accumulate`, `a`'yı `r4`'te
saklarken `b`'yi `r1`'de bırakıyordu. Mevcut `helper()` `r1`'e dokunmadığı
için **çalışıyordu** — ama bu gizli bir yerel sözleşmeydi, AAPCS garantisi
değil. `helper`'a `r1`'i bozan tek bir satır eklense sessizce yanlış sonuç
verirdi. Artık iki değer de korunan register'larda.

Özet: AAPCS bir stil tercihi değil, **birlikte çalışabilirlik koşuludur.**
Tek istisna, hiçbir C kodunun çağırmadığı ve hiçbir C kodunu çağırmayan,
tamamen kendi içinde kapalı assembly parçalarıdır — ve gerçek projelerde
böyle bir parça neredeyse hiç bulunmaz.

---

### 6. (Reasoning) HardFault mı, sessiz yanlış sonuç mu daha tehlikeli?

Savunulabilir cevap: **sessiz yanlış sonuç**, ve sebebi teşhis edilebilirlik.

Bir HardFault gürültülüdür. Tam olarak nerede olduğunu söyler, `CFSR`/`HFSR`
ile sebebini anlatır, `PC` ile yerini verir. Geliştirme sırasında ortaya
çıkar, test ortamında ortaya çıkar, sahada bile watchdog ile yakalanır.
Kötü bir gün, ama **görünür** bir gün.

Sessiz yanlış sonuç hiçbirini yapmaz:

- Testten geçebilir, çünkü bozulan register belirli bir kod yolunda canlı
  değilse hiçbir belirti vermez.
- Optimizasyon seviyesi veya kodun başka bir yeri değiştiğinde ortaya çıkıp
  kaybolabilir — "bazen oluyor" sınıfı hatalar.
- Belirtisi hatanın yerinden **başka bir fonksiyonda** görünür. Break It'te
  hatayı `helper` yaptı, yanlış sonucu `accumulate` üretti, şikâyeti `main`
  bastı. Üç ayrı yer.

Karşı argüman da meşru: güvenlik-kritik bir sistemde HardFault, kontrolsüz bir
duruş (ve dolayısıyla bir tehlike) anlamına gelebilir; oysa hafif yanlış bir
sonuç tolere edilebilir. Yani "hangisi kötü" sorusunun cevabı, sistemin
**güvenli durumunun** ne olduğuna bağlıdır.

Bu yüzden gerçek cevap şudur: sessiz hataları gürültülü hâle getirmek en iyi
stratejidir. Assertion, MPU, stack limit kontrolü, statik analiz ve derleyici
uyarıları hep bunun içindir — hatayı erken ve **yüksek sesle** ortaya
çıkarmak.

---

## Referanslar

- AAPCS32 (ARM-software/abi-aa) — argüman tahsisi, korunması gereken
  register'lar
- Armv8-M ARM (Arm DDI 0553) — `B`, `BL`, `BX` komut tanımları
- [`../README.md`](../README.tr.md) — dersin kendisi

---

[← Ders 03](../README.tr.md) · [Ders 04 →](../../04-stack-msp-psp/README.tr.md)
