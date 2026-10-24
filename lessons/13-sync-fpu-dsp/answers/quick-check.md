# Lesson 13 — Quick Check Cevapları

## 1. Volatile sayaç artışı neden bir güncelleme kaybediyor?

Load ve store ayrı komutlardır. Handler arada 0→1 yapar; Thread'in önceden
okuduğu 0'dan ürettiği 1, handler sonucunun üzerine yazılır. Volatile bu
işlemleri atomik yapmaz. Deneyde DSB/ISB de kayıp güncellemeyi engellemez.

## 2. STREX niçin başarısız ve retry neyi değiştiriyor?

Exception giriş/çıkış süreci local exclusive monitor'ı temizler. İlk STREX
1 döndürür ve yazmaz. Retry yeni değeri LDREX ile tekrar okur, başarılı
STREX iki artışı korur. Yalnız store'u tekrarlamak yeterli değildir.
Lab'ın sınırlı attempt sayısı tükenirse fonksiyon başarısızlık döndürür;
genel ilerleme veya WCET garantisi yoktur. DMA/global monitor kapsamı ayrı
bir sistem problemidir; bu test onları doğrulamaz.

## 3. Önceki PRIMASK neden geri yükleniyor?

Çağıran zaten maskeliyse koşulsuz enable dıştaki kritik bölümü açar.
Sürücünün ikinci çağrısı bunu yakalar. PRIMASK NMI ve HardFault'u maskelemez.
Memory barrier atomiklik veya karşılıklı dışlama sağlamaz: DMB bellek
sıralamasını, DSB tamamlanmayı, ISB sonraki komutların güncel yürütme
bağlamını kullanmasını ele alır. Bu modelde barrier kaldırınca reordering
görülmemesi mimari gereksizliğin kanıtı değildir.

## 4. FPU'yu açmak tek başına extended frame üretir mi?

Hayır. Basic örnekte CPACR açılır fakat SVC öncesi FP komutu yoktur:
FPCA=0, EXC_RETURN=0xFFFFFFF9, 32 byte. FP kullanan Thread'de FPCA=1,
EXC_RETURN=0xFFFFFFE9, 104 byte ölçüldü. Bu boyutlar Secure→Secure,
MSP, TS=0, 8-byte hizalı giriş ve padding olmayan bu deney içindir.

## 5. Lazy frame'de ayrılmış alan neden henüz FP verisi değil?

LSPACT=1 iken hardware yer ayırmıştır ama register'ları henüz yazmamış
olabilir. Deneyde ham S0 slot'u 0xA5A5A5A5 kalır. Handler'ın ilk FP komutu
saklamayı tetikler; LSPACT=0 olur ve slot'ta eski Thread S0=0x3FC00000
görülür. Handler'ın canlı S0=8.0 değeri ile stack'teki 1.5 farklıdır;
exception dönüşünde 1.5 geri gelir. Reserved word geçerli FP verisi değildir.

## 6. NOCP mi, FORCED HardFault mu?

UsageFault bu deneyde etkin. İlk izinsiz FP komutunda CFSR=0x00080000
(UFSR.NOCP), HFSR=0, IPSR=6. PC aynı komutu gösterir. Otomatik olarak
HardFault/FORCED demek yanlıştır; escalation, enable/priority koşullarına bağlıdır.

## 7. Soft, softfp ve hard aynı şey mi?

Soft C FP işlemleri için yazılım yardımcıları kullanır; softfp hardware FP'ye
izin verir ve base AAPCS çağrı düzenini korur. Hard FP parametre/sonuçları
VFP düzeniyle aktarır. CPACR izni bunlardan ayrı bir runtime koşuludur. FPSCR kontrol bitleri de
çağrı sözleşmesinin parçasıdır: lab'da ortam destek fonksiyonu FZ=1 seçer;
aritmetik worker bunu değiştirmez ve stacked/call-return FPSCR karşılaştırılır.
Bu lab assembly adaptöründe iki convention'ı açıkça seçer; rastgele karışık
nesneleri güvenli yapmaz. Link probe'ları iki yöndeki uyumsuzluğu reddeder.
S16–S31 normal AAPCS'de callee-saved'dir; bu assembly yalnız S0–S2'yi kullanır.
TS=0 hardware frame'i tüm FP register'larının scheduler için saklandığı
anlamına gelmez. Ders 15'teki context switching ayrı bir kabul kapısıdır.

## 8. DSP neden bir hız kanıtı değil?

SMLAD iki signed 16-bit lane'in çarpımlarını toplar; QADD taşmayı saturate
edip sticky Q bitini set edebilir. Düz C referansı bu dört sınırlı örnekte
signed overflow üretmez. Her iki yol -36000 verir; QADD iki yönde sınanır.
Disassembly, komutların üretildiğini gösterir. QEMU'da cycle ölçümü yapılmadı;
daha hızlı veya gerçek zamanlı olduğu sonucu çıkarılmaz.

[← Ders](../README.md)
