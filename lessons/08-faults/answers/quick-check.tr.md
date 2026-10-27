**Dil:** [English](quick-check.md) | Türkçe

# Lesson 08 — Quick Check Cevapları

> Önce kendi cevabını yaz, sonra aç.

---

### 1. "HardFault oluştu" neden her zaman kök sebep değildir?

Çünkü HardFault çoğu zaman bir **escalation hedefidir**. Yapılandırılabilir bir
fault (MemManage, BusFault, UsageFault) kendi handler'ı devre dışıyken veya
çalıştırılamazken oluşursa HardFault'a yükseltilir.

Bu derste ölçüldü — aynı `UDF` komutu:

| | UsageFault ETKİN | UsageFault DEVRE DIŞI |
|---|---|---|
| `IPSR` | 6 | **3** |
| `HFSR.FORCED` | 0 | **1** |
| `UFSR.UNDEFINSTR` | 1 | 1 — **aynı** |

Kök sebep her iki durumda da tanımsız bir komuttu. HardFault yalnızca onu
**gören** handler'dı. Ders 07'de MemManage ile aynı ilke ölçülmüştü; kural
fault sınıfından bağımsız.

---

### 2. `CFSR`'nin üç alt register'ı?

```text
CFSR (0xE000ED28)
├── MMFSR   bits [7:0]     MemManage
├── BFSR    bits [15:8]    BusFault
└── UFSR    bits [31:16]   UsageFault
```

Tek bir 32 bit register'ın üç parçası; ayrı ayrı byte/halfword olarak da
erişilebilir. Konumlar vendored CMSIS `core_cm33.h`'ten doğrulandı
(`SCB_CFSR_MEMFAULTSR_Pos = 0`, `BUSFAULTSR_Pos = 8`, `USGFAULTSR_Pos = 16`).

---

### 3. `MMFAR`'a ne zaman güvenilebilir?

**Yalnızca `MMFSR.MMARVALID` (CFSR bit 7) = 1 iken.**

`MMARVALID = 0` ise `MMFAR` içeriği eski bir değer veya çöp olabilir —
teşhis kanıtı **değildir**.

Ölçüm (Vaka 1): `MMARVALID = 1`, `MMFAR = 0x30000000` = korunan adres.
Karşı örnek (Vaka 3, BusFault): `MMARVALID = 0`, `MMFAR = 0x00000000` —
anlamsız, kullanılmadı.

---

### 4. `BFAR`'a ne zaman güvenilebilir?

**Yalnızca `BFSR.BFARVALID` (CFSR bit 15) = 1 iken.**

Ölçüm (Vaka 3): `BFARVALID = 1`, `BFAR = 0x30100000` = hedeflenen eşlenmemiş
adres.

Dikkat: `BFARVALID = 1` olması tek başına her şeyin güvenilir olduğunu
göstermez. Ders 02 vakasında `BFARVALID = 1` **ama** `STKERR` da set —
adres geçerli, frame değil (soru 5).

---

### 5. Frame'i çözmeden önce yığma hatası bitleri neden kontrol edilmeli?

Çünkü yığma hatası, **frame'in kendisinin** düzgün yazılamadığı anlamına
gelir.

```text
MMFSR.MSTKERR  (bit 4)    MemManage, giriste yigma hatasi
BFSR.STKERR    (bit 12)   BusFault,  giriste yigma hatasi
UFSR.STKOF     (bit 20)   stack limiti asildi
```

Bu bitlerden biri set ise bellekte güvenilir bir exception frame olmayabilir;
oradan okunan "R0" ve "stacked PC" değerleri kesilen bağlamı temsil etmeyebilir.

Doğru sıra:

1. fault durumunu oku
2. yığmanın başarılı olup olmadığına karar ver
3. giriş anında kaydedilen `EXC_RETURN` ile frame'in hangi stack'te olduğunu bul
4. **ancak sonra** frame'i çöz

Bu dersin `fault_record()`'u tam olarak bunu yapıyor (`frame_valid`), ve
`cm-frame` yığma hatası görünce reddediyor — ders 02 vakasında ölçüldü:
`REDDEDILDI [STACKING HATASI]: CFSR = 0x00009200`.

---

### 6. Yığılmış PC her zaman hata veren komut mudur?

**Hayır.** Anlamı exception türüne bağlıdır.

| Durum | Yığılmış PC | Bu repoda |
|---|---|---|
| Precise fault | Hata veren komutun **kendisi** | Ölçüldü: `udf` / `ldr` / `str` |
| SVC (ders 07) | `svc`'den **sonraki** komut | Ölçüldü: `0x100000b8` → `0x100000ba` |
| Imprecise fault | Hataya sebep olan erişimi **göstermeyebilir** | Ölçülmedi — mimari uyarı |

Precise bir fault'ta komut tamamlanamadığı için PC ona işaret eder. `SVC`
tamamlanmış bir komuttur, dönüş devam adresinedir. Imprecise bir hatada
raporlama gecikebilir ve işlemci o arada ilerlemiş olabilir.

---

### 7. `HFSR.FORCED` sana ne söyler?

Yapılandırılabilir öncelikli bir exception'ın HardFault'a **yükseltildiğini**
(escalated) — ya da mimarinin tanımladığı başka bir escalation koşulunun
oluştuğunu.

Bu HardFault bir escalation sonucudur; asıl sebep `CFSR` üzerinden incelenir.

`FORCED` "işlemci rastgele bir çöküşü zorladı" demek **değildir**. Mimari bir
yönlendirme kuralının işlediğini söyler.

Bu repoda üç kez ölçüldü: ders 02 (`STKERR` → HardFault), ders 07
(`DACCVIOL` → HardFault), ders 08 (`UNDEFINSTR` → HardFault).

---

### 8. Yapılandırılabilir bir fault devre dışıyken oluşursa ne olur?

Kavramsal olarak: fault **kaybolmaz**, **yükseltilir**. HardFault handler'ı
çalışır ve `HFSR.FORCED` set olur.

Durum register'larını yazan donanım handler seçiminden bağımsız çalıştığı için
`CFSR` ve (geçerliyse) adres register'ları yine doğru yazılır.

> **OBSERVATION**, universal değil: bu derste ölçülen `UNDEFINSTR` escalation'ı
> `HFSR = 0x40000000` ve korunmuş `CFSR = 0x00010000` üretti. Her devre dışı
> fault'un **aynı** register değerlerini üreteceği iddia edilmiyor.

---

### 9. (Reasoning) Precise ve imprecise BusFault teşhis açısından nasıl farklı?

**Precise (`PRECISERR`):** işlemci hatayı belirli bir komutla
ilişkilendirebilir. Yığılmış PC genellikle o **komutun** adresini gösterir;
`BFARVALID` set ise `BFAR` o komutun erişmeye çalıştığı **veri** adresini
verir. İkisi farklı adreslerdir ve birlikte tek bir erişimi tarif eder —
Vaka 3'te ölçüldüğü gibi:

```text
stacked PC = 0x100000F8   komut adresi  (ldr r0, [r0])
BFAR       = 0x30100000   veri adresi   (ldr'nin okumaya calistigi yer)
```

**Imprecise (kavram):** raporlama gecikir. Tipik örnek, yazma
tamponundan geçen bir yazmanın hata vermesi — işlemci o sırada başka
komutlara geçmiştir. Sonuç:

- yığılmış PC hataya sebep olan erişimi **göstermeyebilir**,
- `BFAR` genellikle **geçerli değildir**.

Teşhis bu yüzden daha zordur: şüpheli erişimi kodu inceleyerek, zamanlama
penceresini daraltarak bulmak gerekir. Geliştirme sırasında yazma
tamponlamasını devre dışı bırakmak bazı imprecise hataları precise hale
getirmek için kullanılan bir tekniktir.

> Bu ortamda deterministik bir imprecise BusFault kurulamadı; bu cevap genel
> fault modeline ait bir açıklamadır, ölçüm değil.
>
> **Cortex-M33 notu:** Genel Armv8-M/CMSIS materyali imprecise durumu BFSR
> bit 2 `IMPRECISERR` ile adlandırır. Bu laboratuvar bu biti **Cortex-M33'te
> gerçekleştirilmiş bir alan olarak sunmuyor**: bağımsız denetim M33 BFSR
> register tablosunun bit 2'yi Reserved/RES0 olarak işaretlediğini bildirdi,
> ulaşılabilen M33 kılavuz metninin özet tablosu ise terimi hâlâ listeliyor.
> Kaynaklar tutarlı olmadığı için bit 2 yalnızca "`bit2 (generic
> IMPRECISERR)`" diye etiketleniyor.

---

### 10. (Reasoning) Durum register'ları neden kapsamlı iş yapılmadan **önce** yakalanmalı?

Üç sebep:

1. **Handler bağlamı değiştirir.** C prologu `SP`'yi değiştirir, herhangi bir
   `bl` `LR`'yi (`EXC_RETURN`'ü) ezer, register'lar yeniden kullanılır.
2. **Handler'ın kendisi fault üretebilir.** Konsol, bellek erişimi, çağrılan
   fonksiyonlar — hepsi yeni bir fault'un kaynağı olabilir ve sticky bitler
   **birikir**. Sonradan okunan `CFSR` iki fault'un karışımı olur.
3. **Zaman.** Konsol işi uzun sürer; bu arada başka exception'lar gelebilir.

Bu dersin sarmalayıcıları `LR`/`MSP`/`PSP`'yi assembly'de, **ilk iş** olarak
kaydeder; `fault_record()` de ilk satırlarında `CFSR`/`HFSR`/`MMFAR`/`BFAR`'ı
sabitler. Konsol işi ondan sonra gelir.

---

### 11. (Reasoning) Canlı handler `$sp` neden otomatik olarak kesilen frame değil?

İki bağımsız sebep (ikisi de ders 07'de ölçüldü):

1. **Frame başka bir stack'te olabilir.** Thread PSP kullanıyorsa frame PSP'de,
   handler MSP'de çalışır. Bu dersin dört vakasında frame hep PSP'deydi
   (`0x300002D8`/`0x300002F8`), handler `$sp` = `0x30007FF0`.
2. **Aynı stack olsa bile handler kendi verisini yığar.** Prolog ve yerel
   değişkenler MSP'yi frame'in altına, daha düşük adreslere iter.

Doğru yöntem: giriş anında `EXC_RETURN`'ü kaydet, bit 2'ye bak, giriş
MSP/PSP'sini kullan. `cm-frame <adres> <EXC_RETURN>` bu yüzden iki argüman
ister.

---

### 12. (Reasoning) HardFault'a girdikten sonra ilk bakacağın şey?

**`HFSR.FORCED`.**

- `FORCED = 1` → bu bir escalation; asıl sebebi `CFSR` üzerinden incele.
- `FORCED = 0` → HardFault kendi başına oluştu (örneğin vector table okuma
  hatası, `VECTTBL`). `HFSR`'nin diğer bitlerine bak.

Hemen ardından, **frame'e dokunmadan önce**: `CFSR`'deki yığma hatası
bitleri. Onlar temiz değilse frame'i çözmeye çalışma.

Bu, triage listesinin 2. ve 6. adımlarıdır — ve sırası önemlidir.

---

### 13. (Reasoning) `CFSR`'yi temizlemek için ona `0` yazmak neden işe yaramaz?

Çünkü `CFSR` ve `HFSR` **write-one-to-clear (W1C)** register'lardır: bir bite
**1** yazmak o biti temizler; **0** yazmak hiçbir şey yapmaz.

```c
SCB->CFSR = 0;             /* HICBIR SEY TEMIZLENMEZ */
SCB->CFSR = SCB->CFSR;     /* set olan her bite 1 yaz -> hepsini temizle */
SCB->CFSR = SCB_CFSR_DACCVIOL_Msk;   /* yalnizca DACCVIOL'u temizle */
```

**HFSR için aynı geri yazma kalıbı kullanılmaz.** Cortex-M33 HFSR bit
tablosu (bağımsız denetimin aktardığı haliyle) `DEBUGEVT`'ye (bit 31) HFSR
her yazıldığında **1** yazılmasını ister; aksi halde davranış UNPREDICTABLE.
`SCB->HFSR = SCB->HFSR`, `DEBUGEVT` temizken bit 31'e 0 yazar. Bu dersin
kullandığı açık maske:

```c
uint32_t hfsr = SCB->HFSR;
SCB->HFSR = SCB_HFSR_DEBUGEVT_Msk |
            (hfsr & (SCB_HFSR_FORCED_Msk | SCB_HFSR_VECTTBL_Msk));
```

Ölçüm (QEMU, kaçış vakası): yazılan değer `0xC0000000`, HFSR `0x40000000` →
`0x00000000`. QEMU bu yazma kuralını **doğrulamaz** (HFSR'yi düz W1C
uygular); gerçek donanımda test edilmedi. Cortex-M3/M4 kılavuzlarının
`DEBUGEVT` için "0 yaz" demesi bu çekirdek için geçerli değildir.

Tasarımın sebebi: donanımın yeni bir fault için set ettiği bir biti, yazılımın
okuma-değiştirme-yazma sırasında yanlışlıkla silmesini önlemek. Yazılım yalnızca
**gördüğü** bitleri temizler.

Pratik sonuç: aynı firmware oturumunda birden fazla fault deneyi yapıyorsan,
her deney arasında W1C ile temizlemezsen `CFSR` önceki deneyin bitlerini de
taşır ve iki fault'u birbirine karıştırırsın. Bu dersin her hedefi reset'ten
başlayıp tek bir fault ürettiği için bu sorun yaşanmıyor.

---

## Deney 2 cevapları — hangi kanıt güvenilir?

| Kanıt | 1. Temiz `DACCVIOL` | 2. Yükseltilmiş `UNDEFINSTR` | 3. `UsageFault` | 4. Yığma hatası (ders 02) |
|---|---|---|---|---|
| Aktif handler | ✅ MemManage | ✅ HardFault — ama **kök sebep değil** | ✅ UsageFault | ✅ HardFault — kök sebep değil |
| `HFSR.FORCED` | ✅ 0 | ✅ 1 → `CFSR`'ye in | ✅ 0 | ✅ 1 → `CFSR`'ye in |
| `CFSR` sebep biti | ✅ `DACCVIOL` | ✅ `UNDEFINSTR` | ✅ `UNDEFINSTR` | ✅ `PRECISERR` + `STKERR` |
| Adres register'ı | ✅ `MMFAR` (VALID) | — yok | — yok | ✅ `BFAR` (VALID) |
| Frame | ✅ geçerli | ✅ geçerli | ✅ geçerli | ❌ **`STKERR` → güvenme** |
| Yığılmış PC | ✅ `str` | ✅ `udf` | ✅ `udf` | ❌ okunmaz |

Son sütundaki ince nokta: `BFARVALID = 1` olduğu için **adres** güvenilir,
ama `STKERR` olduğu için **frame** güvenilmez. Bu nedenle adresin ve
frame'in geçerliliği ayrı ayrı değerlendirilmelidir.

---

## Referanslar

- Armv8-M ARM (Arm DDI 0553) — `CFSR`, `HFSR`, `MMFAR`, `BFAR`, `SHCSR`,
  W1C, escalation, precise/imprecise
- Armv8-M Exception Model User Guide (Arm 107706)
- Cortex-M33 Devices Generic User Guide (Arm 100235) — 4.2.11 CFSR,
  4.2.12 HFSR (W1C / sticky; `DEBUGEVT` yazma kuralı), 2.5.1 fault türleri
- CMSIS-Core `core_cm33.h` — bit konumları (genel Armv8-M tanımları; BFSR
  bit 2 dahil)

---

[← Ders 08](../README.tr.md) · [Ders 09 →](../../09-compiler-aapcs/README.tr.md)
