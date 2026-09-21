# Lesson 01 — Referans Cevaplar

> Önce kendi cevabını yaz. Bu dosyayı ondan sonra aç.
>
> Senaryolarda **tek doğru cevap yok**. Aşağıdakiler "bu gerekçe savunulabilir
> mi" sorusunun cevabıdır, "doğru seçim budur"un değil. Farklı bir seçim
> yaptıysan ve gerekçen sağlamsa, cevabın doğrudur.

---

# Bölüm 1 — Üç senaryo

## Senaryo A — Secure Smart Lock

**Savunulabilir seçim:** M33 (ya da M23, aşağıdaki şartla).

**Birincil gerekçe — ama varsayımlarıyla birlikte.** Şu üçü doğruysa
TrustZone güçlü bir gerekçe kazanır: (a) cihaz kalıcı bir sır barındırıyor,
(b) sahadan güncellenebilir kod çalıştırıyor, (c) tehdit modeli "uygulama
katmanında bir gün bir açık olacak" varsayımını içeriyor. Bu koşullar altında
TrustZone, anahtarı Non-secure tarafın adres uzayından tamamen çıkarır.

Koşullar değişirse cevap da değişir: kod sahada güncellenmiyorsa, sır harici
bir güvenli elemanda duruyorsa veya sertifikasyon gerekmiyorsa, MPU + iyi
ayrılmış ayrıcalıklı katman savunulabilir bir seçim olur. "TrustZone şarttır"
cümlesi koşulsuz yazılamaz.

**MPU yeterli olur muydu?** Kısmen. MPU, unprivileged kodun bir bölgeye
erişmesini engelleyebilir. Ama MPU'yu privileged kod yapılandırır; privileged
katmanda bir açık varsa (ve bootloader, sürücüler, RTOS hep oradadır) MPU
konfigürasyonu değiştirilebilir. TrustZone'da ise secure tarafın belleği
non-secure taraf için **hiç adreslenebilir değildir** — privileged non-secure
kod bile göremez.

**Neyi feda ettin:** Parça maliyeti, karmaşıklık, iki ayrı imaj (secure +
non-secure) build etme külfeti, daha uzun geliştirme süresi.

**M23 ne zaman yeterli:** Ağır kripto yoksa ve DSP/FPU gerekmiyorsa. M23
(Armv8-M Baseline) TrustZone taşır ama performansı ve komut seti M33'ten
sınırlıdır. Kripto hızlandırıcısı ayrı bir çevre birimi olarak varsa M23 çok
makul bir seçimdir.

**2 yıl sonra pişmanlık senaryosu:** Ürün ek bir güvenlik sertifikasyonundan
geçmek zorunda kalırsa ve seçtiğin parçada Security Extension yoksa, yazılımla
telafi edemezsin. Silikon değişikliği gerekir.

---

## Senaryo B — Low-Power Wearable

**Savunulabilir seçim:** M0+ veya M23.

**Birincil gerekçe:** Güç bütçesi diğer tüm kriterleri eziyor. Adım sayma,
eşikleme ve hareketli ortalama sabit noktalı (fixed-point) aritmetikle
rahatça yapılır. FPU'nun buradaki katkısı marjinaldir; maliyeti ise sürekli
(alan, sızıntı akımı, daha fazla kapı).

**FPU gerçekten gerekli mi?** İvme verisi zaten tamsayı olarak geliyor.
Kayan noktaya çevirmek genellikle bir **kolaylık** tercihidir, bir gereklilik
değil. Q15/Q31 sabit noktalı formatla çalışmak biraz daha zahmetlidir ama
deterministik ve ucuzdur.

**M0+ seçersen ne kaybedersin:**
- MPU yok (M0+'da opsiyoneldir ve çoğu ucuz parçada yoktur) → task izolasyonu
  yapamazsın
- Sınırlı atomik işlem desteği (LDREX/STREX yok) → lock-free yapılar zorlaşır
- Daha az debug imkânı (DWT/ITM sınırlı veya yok)
- Bit-band, saturating aritmetik gibi kolaylıklar yok

**2 yıl sonra pişmanlık senaryosu:** Ürün "sağlık verisi" saklamaya başlarsa
(kalp ritmi, uyku), veri koruma mevzuatı devreye girer ve TrustZone'suzluk
seni bir anda M23/M33'e taşınmak zorunda bırakır. Bu, yazılımın büyük kısmının
yeniden doğrulanması demektir.

**Ara yol:** M23, M0+'a göre çok az güç maliyetiyle TrustZone ve daha iyi
debug getirir. Ürün yol haritası belirsizse M23 sigortadır.

---

## Senaryo C — Robotics / Signal Processing Node

**Savunulabilir seçim:** M33 (FPU + DSP konfigürasyonlu) veya M7.

**Birincil gerekçe:** Kontrol döngüsü sert gerçek zamanlıdır. Buradaki kritik
metrik ortalama hız değil, **en kötü durum gecikmesidir (worst-case
latency)**. Kinematik ve filtre matematiği yoğun kayan noktalı olduğu için
FPU burada **güçlü bir gerekçe kazanır** — ama koşullu: kinematik ve filtre
matematiğinin kayan noktalı olması, döngü bütçesinin sabit noktalı yazımla
karşılanamaması ve hata analizi maliyetinin kabul edilemez olması gerekir.
Bu üçü ölçülmeden "FPU şarttır" denemez; sabit noktalı (Q15/Q31) bir çözüm
birçok kontrol döngüsü için yeterlidir.

**Cache'li M7 daha hızlı, neden tercih edilmesin?** Çünkü cache
**determinizmi bozar.** Aynı kod, cache hit ile miss arasında kat kat farklı
sürede çalışır. Ortalaman iyileşir, ama en kötü durumun sınırını tahmin etmek
zorlaşır. Sert gerçek zamanlı bir sistemde ölçtüğün şey ortalama değil, tavan
olmalıdır. M7 kullanılacaksa cache'in kritik bölgeler için kilitlenmesi
(lockdown) veya TCM kullanımı gerekir — yani ek mühendislik.

**Nasıl ölçerek savunursun:** Kontrol döngüsünün yürütme süresini çok sayıda
iterasyonda örnekle ve **gözlenen maksimumu** raporla, ortalamayı değil.
Deadline'a göre bir pay (headroom) belirle. Bunun kanıtlanmış bir WCET
sınırı **olmadığını** da not et. Bu ölçümü QEMU'da yapamazsın — cycle counter
çalışmıyor (`docs/TARGET_CAPABILITIES.md`). Gerçek zamanlama için gerçek
donanım + DWT/trace gerekir. FVP'ler de otomatik çözüm değildir: sıradan bir
FVP **fonksiyonel** bir modeldir ve cycle sayısı vermez; zamanlama bilgisi
ancak açıkça "cycle-approximate" olarak belgelenmiş modellerden gelir. Hangi
modelin ne sunduğunu ürün dokümantasyonundan doğrulamak gerekir.

**MVE gerekli mi?** Muhtemelen hayır. Sensör füzyonu ve PID ölçeğindeki iş
için DSP komutları (tek çevrimde MAC, SIMD) genellikle yeterlidir. MVE, ML
çıkarımı veya çok kanallı ağır filtreleme ölçeğinde anlam kazanır.

**Güvenlik:** Robot ağa bağlıysa cevap değişir. Uzaktan komut alan bir eklem
kontrolcüsünde firmware imzası ve güvenli boot ihtiyacı doğar; o noktada
TrustZone tekrar masaya gelir.

---

# Bölüm 2 — Quick Check cevapları

### 1. TrustZone ile MPU arasındaki fark nedir? Birbirinin yerine geçer mi?

**Hayır — ama sebebi "biri güvenilirlik, diğeri güvenlik" değildir.**
Farklı **sınırlar** çizerler.

| | MPU | TrustZone (Security Extension) |
|---|---|---|
| Neyi ayırır | Privileged ↔ unprivileged erişim | Secure ↔ Non-secure **durum** |
| Kime karşı koruma sağlar | Ayrıcalıksız yazılıma karşı — hatalı da olsa, düşmanca da olsa | Non-secure tarafın **tamamına** karşı, ayrıcalıklı kodu dahil |
| Kim yapılandırır | Ayrıcalıklı yazılım | Secure yazılım (SAU) |
| Neye güvenmek zorunda | Ayrıcalıklı katmanın doğruluğuna | Secure yazılımın doğruluğuna |

MPU **gerçek bir güvenlik sınırı da olabilir**: ayrıcalıklı katman güvenilir
olduğu sürece, ayrıcalıksız kodu — düşmanca olsa bile — kendi bölgesine
hapseder. Sandbox'lanmış eklenti kodu, ağdan gelen veriyi ayrıştıran bir
görev veya kullanıcı uygulaması için bu tam anlamıyla güvenlik korumasıdır.

TrustZone farklı bir sınır çizer: Non-secure tarafın **ayrıcalıklı** kodu
bile Secure belleği göremez. Bu koruma **Secure yazılıma güvenildiği**
varsayımına dayanır; oradaki bir hata veya yanlış SAU yapılandırması
korumayı ortadan kaldırır.

Yani ikisi de "güvenilir bir yazılım katmanı" varsayımını **kaldırmaz**,
yalnızca o katmanı farklı yere koyar. İkisi birlikte kullanılır: Armv8-M'de
zaten ayrı MPU_S ve MPU_NS konfigürasyonları vardır.

---

### 2. FPU var ama firmware hiç kayan nokta kullanmıyor. Maliyeti nedir?

Üç ayrı maliyet:

1. **Silikon maliyeti** — alan ve sızıntı akımı, kullanılmasa bile ödenir.
2. **Exception latency riski — ama koşullu.** `CPACR` ile FP erişimine
   **izin vermek**, her exception'da genişletilmiş frame yazılacağı anlamına
   gelmez. İzin ile **aktif FP bağlamı** ayrı şeylerdir: belirleyici olan
   `CONTROL.FPCA`'dır. Ayrıca Armv8-M *lazy* FP durum koruması sunar
   (`FPCCR.LSPEN`): yer ayrılır ama register'lar ancak gerçekten gerekirse
   yazılır. Yani maliyet gerçek FP kullanımına ve koruma yapılandırmasına
   bağlıdır; "FPU'yu açtın, her frame 26 word oldu" fazla kesin bir cümledir.
3. **ABI ve bütçe riski.** Donanım FP erişimine izin verilmemişken bir FP
   komutu çalıştırılırsa **otomatik yazılım emülasyonuna düşülmez**; bir
   fault (UsageFault, NOCP) oluşur. Yazılım FP rutinleri bir **derleme ve
   link** kararıdır: `-mfloat-abi=soft` derleyiciye FP işlemlerini libgcc
   çağrılarına çevirtir. Gerçek risk "sessiz emülasyon" değil, **yavaş ama
   doğru** yazılım FP kodunun bütçeyi aşması — ya da farklı FP ABI'leriyle
   derlenmiş nesneler karışırsa link/çalışma zamanı hatalarıdır.

Bu repoda tam olarak bu yüzden `-mfloat-abi=soft` kullanılıyor: exception
frame'i 8 word'de tutmak ders 07/08'i basitleştiriyor. FPU ders 13'te
**bilerek** açılacak.

---

### 3. `MVFR1.MVE = 0` — simülatörün eksikliği mi, işlemcinin özelliği mi?

İşlemcinin özelliği. Daha doğrusu: **mimarinin sınırı.**

MVE (Helium), Armv8.1-M ile gelen bir vektör uzantısıdır. Cortex-M33 ise
Armv8-M Mainline uygular. Yani hiçbir Cortex-M33 implementasyonunda MVE
bulunmaz — bu konfigürasyonla açılabilecek bir seçenek değildir.

**Nasıl ayırt edersin:** İki belgeye bakarsın.
- *Armv8-M Architecture Reference Manual* → uzantı bu mimaride var mı?
- *Cortex-M33 Technical Reference Manual / Integration and Implementation
  Manual* → varsa, bu çekirdekte opsiyonel mi, zorunlu mu?

Karşılaştır: `MPU_REGIONS = 0` okusaydın bu bir **konfigürasyon** kararı
olurdu; başka bir M33'te 8 veya 16 çıkabilirdi. `MVE = 0` ise mimari gerçektir.

---

### 4. (Reasoning) Sert gerçek zamanlı döngü: M7 mi M33 mü?

Cevap Senaryo C'de ayrıntılı. Özeti:

M33 daha doğru seçimdir **eğer** en kötü durum gecikmesi ortalama
performanstan daha kritikse. M7'nin cache'i ortalamayı iyileştirir ama
varyansı artırır; sert gerçek zamanlı sistemde karar verdiren şey varyanstır.

Savunma yöntemi: çok sayıda döngü iterasyonunun **maksimum** süresini ölç,
ortalamayı değil. "M7 %40 daha hızlı" cümlesi, maksimumu ölçmeden hiçbir şey
ifade etmez.

**Gözlenen maksimum, kanıtlanmış bir WCET
(worst-case execution time) sınırı değildir.** Ölçüm yalnızca çalıştırdığın
girdileri ve durumları kapsar; en kötü yolu hiç tetiklememiş olabilirsin.
Gerçek WCET analizi statik analiz araçları, donanım modelleri ve genellikle
sertifikasyon süreci ister. Doğru ifade: "gözlenen maksimum X idi, bütçe
Y'ydi" — "WCET X'tir" değil.

---

### 5. (Reasoning) "TrustZone pahalı, MPU ile hallederiz" — neresi doğru, neresi tehlikeli?

**Doğru olan kısım:** MPU gerçek bir izolasyon sağlar ve birçok üründe
yeterlidir. Maliyet kaygısı da meşrudur. Tehdit modeli "kendi bug'larımızdan
korunmak" ise MPU doğru araçtır ve TrustZone fazladan karmaşıklıktır.

**Tehlikeli olan kısım:** İki farklı tehdit modelinin birbirine karıştırılması.
MPU'yu privileged yazılım yapılandırır. Saldırgan privileged koda ulaşırsa
(bootloader, bir sürücü, RTOS çekirdeği — hepsi privileged'dır) MPU'yu
yeniden yapılandırıp korumayı kaldırabilir. Yani MPU, **privileged katmanın
güvenilir olduğu** varsayımına dayanır.

TrustZone bu varsayımı kaldırır: non-secure tarafın privileged kodu bile
secure belleği göremez.

**Doğru soru:** "Tehdit modelimiz ne ve saldırgan hangi ayrıcalık düzeyinde
kod çalıştırabilir?"

- Saldırgan yalnızca **ayrıcalıksız** kod çalıştırabiliyorsa (sandbox'lanmış
  eklenti, ayrıştırıcı görev, kullanıcı uygulaması), MPU gerçek bir güvenlik
  sınırıdır ve yeterli olabilir.
- Saldırgan **ayrıcalıklı** koda ulaşabiliyorsa, ya da cihaz fiziksel erişime
  açıksa, ya da sertifikasyon Non-secure yazılımdan bağımsız bir sınır
  istiyorsa, MPU tek başına yetmez.

Yani "MPU güvenlik değildir" de yanlış, "MPU her şeyi çözer" de. Belirleyici
olan sınırın **nereye** çizildiği ve hangi katmana güvenildiğidir.

---

## Referanslar

- Arm Cortex-M33 Technical Reference Manual — opsiyonel uzantılar, MPU/SAU
- Arm Cortex-M33 Integration and Implementation Manual — `FPU`, `DSP`,
  `SECEXT`, `MPU_S`, `MPU_NS` konfigürasyon parametreleri
- Armv8-M Architecture Reference Manual (Arm DDI 0553)
- [`docs/TARGET_CAPABILITIES.md`](../../../docs/TARGET_CAPABILITIES.md)

---

[← Ders 01](../README.md)
