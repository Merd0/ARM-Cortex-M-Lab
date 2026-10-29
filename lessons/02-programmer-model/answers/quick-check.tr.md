**Dil:** [English](quick-check.md) | Türkçe

# Lesson 02 — Quick Check Cevapları

> Buraya, kendi cevabını **yazdıktan sonra** bak. Hazır cevabı okumak
> bildiğini sanmanın en hızlı yoludur.

---

### 1. `CONTROL = 0` ve `IPSR = 0` okudun. Hangi mod, hangi yetki, hangi stack?

Privileged Thread Mode, MSP aktif.

Üç bilgi iki register'dan geliyor:

| Alan | Değer | Anlam |
|---|---|---|
| `IPSR` | 0 | Thread Mode (sıfırdan farklı olsaydı Handler Mode ve değer exception numarası olurdu) |
| `CONTROL` bit 0 (`nPRIV`) | 0 | Privileged |
| `CONTROL` bit 1 (`SPSEL`) | 0 | MSP kullanılıyor |

**Sorunun ikinci kısmı:** hayır, yalnızca `CONTROL = 0` ile cevaplayamazdın.
Mod bilgisi `CONTROL`'de **değil**, `IPSR`'dedir. `CONTROL = 0` Handler
Mode'da da okunabilir — ve o durumda `SPSEL = 0` görsen bile bu bilgi
gereksizdir, çünkü Handler Mode zaten MSP kullanır ve zaten ayrıcalıklıdır.

Kısaca: mod önce `IPSR`'den belirlenir, `CONTROL` ondan sonra yorumlanır.

İkinci bir düzeltme: `CONTROL`'ün **yalnızca iki alanı yoktur**. `nPRIV`
(bit 0) ve `SPSEL` (bit 1) bu derste **kapsam içine aldıklarımızdır**;
mimariye, FP bağlamına ve güvenlik uzantısına göre `FPCA` ve `SFPA` gibi
başka tanımlı alanlar da bulunur (ders 13 ve 16). Bu yüzden `CONTROL` her
zaman oku-değiştir-yaz ile güncellenir.

---

### 2. Vector table'daki `Reset` girdisi `0x10000205`, sembol `0x10000204`. Bu 1 nereden geliyor?

En düşük bit (bit 0), adresin parçası değil — **Thumb state göstergesidir.**

Cortex-M yalnızca Thumb komut setini çalıştırır. İşlemci bir dallanma
hedefinin bit 0'ına bakar: 1 ise "Thumb state'e geç" der. Fonksiyon adresleri
2 byte hizalı olduğu için bit 0 zaten her zaman boştur; mimari bu boş biti
bayrak olarak kullanır.

Bit 0 = 0 olsaydı: işlemci Arm state'e geçmeye çalışırdı. Cortex-M'de Arm
state **yoktur**, dolayısıyla bir INVSTATE (UsageFault) koşulu doğar.

Bunun **reset'te** ne ile sonuçlanacağı koşullara bağlıdır. Reset sırasında
fault yükseltme (escalation) kurallarına göre işlemci genellikle lockup
durumuna girer; ama "her zaman lockup" demek fazla kesin bir ifadedir —
sonucu belirleyen, hatanın hangi bağlamda ve hangi öncelik düzeyinde
oluştuğudur. Kesin kurallar için Armv8-M ARM'ın fault yükseltme bölümüne bak.

Dikkat, bir genelleme tuzağı: "vector table'daki bütün girdiler tek sayıdır"
**doğru değildir**. Yalnızca **handler adresi taşıyan** girdiler için geçerli.
İki istisna:

- **`[0]` girdisi bir adres değildir**; reset'te `MSP`'ye yüklenen başlangıç
  değeridir. Bizim build'imizde `0x30008000` — çift sayı.
- **Ayrılmış (reserved) girdiler** 0 olabilir ve sıfır tek sayı değildir.

`cm-vectors` çıktısında `[0]`'ın yanında bu not artık yazılı.

---

### 3. `MSP` reset'te `0x30008000`, `main` içinde `0x30007ff8`. Bu 8 byte nereye gitti?

`Reset_Handler`, `main`'i çağırmadan önce stack'e veri itti (push): dönüş
adresi ve kullandığı callee-saved register'lar.

Yön: Cortex-M stack'i **full descending** (tam azalan). "Full" = SP her zaman
son yazılan geçerli veriyi gösterir; "descending" = push işlemi SP'yi
**azaltır**. Yani stack, RAM'in tepesinden aşağı doğru büyür.

Bu yüzden linker script'te `__stack_top = ORIGIN(RAM) + LENGTH(RAM)` yazar —
stack'in başlangıcı RAM'in en üstüdür, tabanı değil.

---

### 4. (Reasoning) PSP'ye adres yazdın ama `SPSEL`'i değiştirmedin. `SP` ne gösterir?

Hâlâ **MSP**'nin değerini.

`SP` (R13) fiziksel olarak tek bir register değil, bir **takma addır**
(banked register). Hangi fiziksel register'ın `SP` olarak göründüğüne
`CONTROL.SPSEL` karar verir:

```
SPSEL = 0  ->  SP, MSP'yi gösterir
SPSEL = 1  ->  SP, PSP'yi gösterir
```

`__set_PSP()` sadece PSP'nin içeriğini değiştirir; hangi register'ın aktif
olduğunu değiştirmez. Değer yazmak ≠ aktif etmek.

Pratik sonucu: bir RTOS'ta task stack'ini kurmak iki ayrı adımdır — önce
PSP'ye geçerli bir adres yaz, **sonra** SPSEL'i 1 yap. Sırayı ters çevirirsen
Break It bölümündeki fault'u alırsın.

---

### 5. (Reasoning) Thread Mode'da PSP kullanılırken exception oluşursa frame hangi stack'e yazılır?

**PSP'ye** — yani exception anında aktif olan stack'e. Donanım, exception'a
girerken kesilen kodun kullandığı stack'i kullanır.

Ardından işlemci Handler Mode'a geçer ve `SPSEL`'i temizler, böylece
handler'ın kendisi **MSP** üzerinde çalışır. Break It deneyindeki gözlem de
bunu gösteriyor: fault anında `CONTROL = 0` idi, çünkü donanım SPSEL'i sıfırladı.

Neden böyle tasarlanmış? Çünkü kernel/handler kodunun, hatalı bir task'ın
stack'ine bağımlı olmaması gerekir. Task stack'i taşmışsa veya bozuksa
handler yine de temiz bir MSP üzerinde çalışmaya devam eder. Bu ayrım,
ders 15'teki context switch'in ve `EXC_RETURN` değerinin temelidir.

Not: frame'in PSP'ye yazılması, **stacking'in başarılı olacağı anlamına
gelmez.** PSP geçersizse stacking'in kendisi fault üretir (BusFault `STKERR`).
Tam olarak bunu Break It bölümünde gördün.

---

[← Ders 02](../README.tr.md) · [Ders 03 →](../../03-c-and-assembly/README.tr.md)
