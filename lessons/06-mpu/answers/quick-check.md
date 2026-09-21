# Lesson 06 — Quick Check Cevapları

> Önce kendi cevabını yaz, sonra aç.

---

### 1. MPU neyi enforce eder? Neyi etmez?

**Eder:** Bir bellek erişiminin, erişimi yapan bağlamın ayrıcalık seviyesine
göre izinli olup olmadığını. Her erişim (veri okuma/yazma, komut getirme)
bölge tablosuna karşı denetlenir; izin yoksa `MemManage` üretilir.

Ayrıca erişim izninin yanında **bellek türünü** de tanımlar
(`RLAR.AttrIndx` → `MAIR`): Normal mi Device mi, cacheability, shareability.

**Etmez:**

- Kodun *mantıksal* doğruluğunu. İzinli bir bölgeye yanlış veri yazmak
  MPU'nun umurunda değildir.
- Güvenlik durumu ayrımını (Secure/Non-secure) — o TrustZone'un işi.
- Çevre birimi erişim denetimini SoC seviyesinde — o PPC'nin işi (soru 10).
- Ayrıcalıklı yazılımın kendisini. MPU'yu ayrıcalıklı kod yapılandırır;
  oradaki bir hata korumayı ortadan kaldırır.

---

### 2. Ayrıcalık ile MPU arasındaki ilişki nedir?

**Ayrıcalık seviyeyi verir, MPU o seviyeye göre belleği kapatır.** Tek
başlarına ikisi de yetersiz:

| Sadece ayrıcalık (ders 05) | Sadece MPU |
|---|---|
| "Bu kod ayrıcalıksız çalışıyor" | Anlamsız — bölge izinleri `NP` bitiyle **ayrıcalığa göre** tanımlanır |
| Ama tüm RAM'e erişebilir | Ayrıcalık kavramı olmadan "kim" sorusunun cevabı yok |

Ders 05 ile ders 06 arasındaki fark somut: aynı `g_service_counter`, aynı
ayrıcalıksız Thread. Ders 05'te doğrudan yazma **çalışıyordu** (sözleşme),
ders 06'da `MemManage` üretiyor (donanım kuralı). Değişen tek şey MPU
politikası.

---

### 3. Ayrıcalıksız Thread neden kendi stack'ine erişebiliyor?

Çünkü Region 1 tam olarak bunun için tanımlandı:

```text
RBAR = 0x30000023
       AP  = 01  -> RO=0 (read/write), NP=1 (ayricaliksiz erisime acik)
       XN  = 1   -> calistirma YOK
       BASE      -> 0x30000020
RLAR = 0x30000321
       LIMIT     -> son byte 0x3000033F (DAHIL)
       EN  = 1
```

`NP = 1` biti "ayrıcalıksız erişime de izin ver" demektir. Thread'in stack'i
ve kendi veri alanı bu aralığın içinde; ölçülen değer
`STACK_WRITE_MAGIC = 0xA5A5C0DE`.

`XN = 1` ise ayrı bir koruma: stack'e enjekte edilen veriden kod
çalıştırılamaz (soru 7).

---

### 4. Aynı Thread neden `g_service_counter`'a erişemiyor?

Çünkü **onu kapsayan hiçbir bölge tanımlanmadı.**

Bu ters gibi görünür: koruma bir bölge *eklemekten* değil, **eklememekten**
geliyor. Armv8-M kuralı şudur: MPU etkinken, ayrıcalıksız bir erişim eşleşen
bir bölge bulamazsa **reddedilir**. Ayrıcalıklı erişimlerin `PRIVDEFENA`
kurtarma yolu vardır, ayrıcalıksız erişimlerin yoktur.

Ölçülen adresler:

```text
COUNTER_ADDR  = 0x30000000
UNPRIV_BASE   = 0x30000020   <- Region 1 buradan basliyor
```

Sayaç, ayrıcalıksız bölgenin **altında** kalıyor ve başka bölge de yok.

---

### 5. `SVC_Handler` aynı değişkene neden erişebiliyor?

Üç adımlı zincir:

1. `SVC` bir exception'dır → işlemci **Handler Mode**'a geçer.
2. Handler Mode **her zaman ayrıcalıklıdır** (ders 05).
3. `MPU_CTRL.PRIVDEFENA = 1` olduğu için, ayrıcalıklı bir erişim hiçbir
   bölgeyle eşleşmediğinde **varsayılan bellek haritası** devreye girer ve
   erişim izinli olur.

Yani handler'ın ayrıcalığı kodundan değil bağlamından geliyor, erişim izni de
politikadan değil `PRIVDEFENA`'dan.

Ölçülen değerler: `SVC_INCREMENT_1 = 1`, `SVC_INCREMENT_2 = 2` — aynı değişken,
aynı firmware, farklı bağlam.

---

### 6. `PRIVDEFENA` ne sağlar? `0` olsaydı ne kırılırdı?

`MPU_CTRL.PRIVDEFENA = 1`: ayrıcalıklı erişimler, hiçbir bölge eşleşmezse
mimarinin **varsayılan bellek haritasını** kullanır.

`0` olsaydı, ayrıcalıklı kod için de "eşleşme yoksa reddet" kuralı geçerli
olurdu. Bu derste her şey kırılırdı:

- `main()` kendi MSP stack'ine (`0x30007FF0` civarı) erişemezdi — o bölge
  tanımlı değil.
- Handler `g_service_counter`'a erişemezdi.
- Konsol çıktısı için UART'a erişilemezdi.

Yani `PRIVDEFENA = 0` seçersen **her ayrıcalıklı erişim için de** bölge
tanımlaman gerekir. Bazı yüksek güvenlikli tasarımlar bunu bilerek yapar
(ayrıcalıklı kodun da sınırlanması için), ama o zaman politika çok daha
büyür — 8 bölge hızla yetmez.

Ölçülen: `MPU_CTRL = 0x00000005` = `ENABLE | PRIVDEFENA`.

---

### 7. `XN` neyi engeller? Thread bloğunda neden `XN = 1`?

`XN` (execute never) o bölgeden **komut getirilmesini** engeller. İhlal
edildiğinde `MMFSR.IACCVIOL` set olur — bu derste ölçtüğümüz `DACCVIOL`'den
farklı bir bit.

Thread bloğu stack ve veri içeriyor; oradan kod çalıştırmak için meşru bir
sebep yok. `XN = 1` klasik bir sertleştirme önlemidir: stack taşması veya
tampon aşımı ile enjekte edilen veri **çalıştırılamaz**. Masaüstündeki
W^X / DEP ile aynı fikir.

Simetrik olarak Region 0 (FLASH) `XN = 0` ve `RO = 1`: çalıştırılabilir ama
yazılamaz. İkisi birlikte "yazılabilir olan çalıştırılamaz, çalıştırılabilir
olan yazılamaz" kuralını kurar.

---

### 8. Armv8-M `RBAR`/`RLAR` modeli Armv7-M'den nasıl farklı?

| | Armv7-M (PMSAv7) | Armv8-M (PMSAv8) |
|---|---|---|
| Bölge tanımı | `base` + **kodlanmış boyut alanı** (2'nin kuvveti) | `RBAR.BASE` + `RLAR.LIMIT` |
| Hizalama | Bölge **kendi boyutuna** hizalı olmalı | 32 byte granularite yeter |
| Alt bölgeler | `SRD` (subregion disable) alanı var | **Yok** |
| Bellek türü | `TEX`/`C`/`B`/`S` alanları bölge içinde | `RLAR.AttrIndx` → `MAIR` |
| Boyut | 2'nin kuvveti olmak zorunda | Herhangi bir 32 byte katı |

`LIMIT` **dahildir** (bölgenin son byte'ı), "bitiş + 1" değil.

**Neden internetteki örnekler kullanılamaz:** Armv7-M örnekleri `SIZE`
alanını doldurur, `SRD` kullanır ve boyuta hizalama varsayar. Bu alanların
hiçbiri Armv8-M'de yoktur. Kopyalarsan ya derlenmez ya da tamamen farklı bir
bölge programlarsın. Doğru refleks: Armv8-M ARM'a veya vendored
`armv8m_mpu.h`'a bakmak.

---

### 9. MemManage exception'ı neden önemli? `MEMFAULTENA` kapalıysa ihlal kaybolur mu?

**Kaybolmaz.** MPU ihlali her zaman bir fault üretir; `MEMFAULTENA` yalnızca
onu **hangi handler'ın** göreceğini belirler.

`MEMFAULTENA = 1` (bu dersteki durum): ihlal doğrudan MemManage handler'ına
gider. Ölçülen: `IPSR = 4`, `HFSR = 0` — escalation yok, `MMFSR`/`MMFAR`
doğrudan okunabiliyor.

`MEMFAULTENA = 0`: MemManage etkin değildir, dolayısıyla fault **HardFault'a
yükseltilir** (escalation) ve `HFSR.FORCED` set olur. Ders 02'de bunu
`HFSR = 0x40000000` olarak görmüştük.

Neden önemli? Çünkü teşhis kalitesi değişir. HardFault "bir şey oldu" der;
MemManage "şu adrese şu tür erişim reddedildi" der. Ayrıca bir RTOS,
MemManage'i yakalayıp **yalnızca hatalı görevi** sonlandırabilir; HardFault
genelde sistem geneli bir çöküştür.

> Bu cevaptaki `MEMFAULTENA = 0` davranışı bu derste **ölçülmedi** —
> Deney 3 olarak sana bırakıldı. Mimari kuralı yazıyorum, ölçümü sen yap.

---

### 10. (Reasoning) Ders 05'teki UART engeli neden MPU örneği değildi?

Çünkü o sırada **MPU hiç etkinleştirilmemişti.** Ders 05 firmware'i
`MPU->CTRL`'e hiç dokunmadı; ölçüm de bunu doğruluyor (ders 06 `[1]`
bölümünde `MPU_CTRL = 0x00000000`).

Engelin kaynağı SoC'nin **Peripheral Protection Controller**'ıydı. QEMU
v8.2.2 `hw/misc/tz-ppc.c` içindeki `tz_ppc_check()` erişim "usermode" ise ve
o portun `cfg_ap` biti sıfırsa işlemi bloklar; `cfg_sec_resp = 0` iken
okumaya `0` döndürür.

Ayırt edici işaret **davranış farkı**:

| | PPC (ders 05) | MPU (ders 06) |
|---|---|---|
| İhlal sonucu | Sessiz: `UART_BAUDDIV_UNPRIV = 0` | Exception: `IPSR = 4` |
| Hata adresi | Yok | `MMFAR = 0x30000000` |
| Nerede | İşlemcinin dışında | İşlemcinin içinde |

Pratik kural: beklenmedik bir erişim hatasında **hangi bloğun** reddettiğini
sor. Exception geldiyse MPU; sessizce yanlış veri geldiyse sistem seviyesinde
bir denetleyici olabilir.

---

### 11. (Reasoning) MPU, TrustZone'un yerine geçer mi?

Hayır — **farklı eksenlerde** çalışırlar.

| | MPU | TrustZone |
|---|---|---|
| Eksen | Ayrıcalık (privileged / unprivileged) | Güvenlik durumu (Secure / Non-secure) |
| Kim yapılandırır | Ayrıcalıklı yazılım | Secure yazılım (SAU) |
| Neye güvenir | Ayrıcalıklı katmanın doğruluğuna | Secure yazılımın doğruluğuna |
| Koruduğu | Ayrıcalıksız koda karşı | **Non-secure ayrıcalıklı koda karşı dahil** |

**MPU'nun yeterli olduğu tehdit modeli:** Saldırgan yalnızca ayrıcalıksız kod
çalıştırabiliyor — sandbox'lanmış eklenti, ağdan gelen veriyi ayrıştıran bir
görev, kullanıcı uygulaması. Ayrıcalıklı katman güvenilir olduğu sürece MPU
gerçek bir güvenlik sınırıdır.

**MPU'nun yetmediği tehdit modeli:** Saldırgan ayrıcalıklı koda ulaşabiliyor
(bootloader, sürücü, RTOS çekirdeği — hepsi ayrıcalıklıdır). MPU'yu
ayrıcalıklı kod yapılandırır; oraya ulaşan saldırgan politikayı yeniden
yazabilir. Cihaz fiziksel erişime açıksa veya sertifikasyon Non-secure
yazılımdan bağımsız bir sınır istiyorsa da yetmez.

İkisi birlikte kullanılır: TrustZone Secure/Non-secure sınırını, MPU her iki
tarafın kendi içindeki görev izolasyonunu sağlar. Armv8-M'de zaten ayrı
`MPU` ve `MPU_NS` bankaları vardır.

---

### 12. (Reasoning) MPU açıkken yazılım otomatik güvenli mi olur?

Hayır. MPU bir **araçtır**, politika değil. Koruma, yazdığın politikanın
doğruluğu kadardır.

Sessizce her şeyi açabilecek hatalar:

- Fazla geniş bir `LIMIT` — korunan veri yanlışlıkla izinli bölgeye girer.
- Unutulmuş `XN` — veri bölgesinden kod çalıştırılabilir.
- `RO` yerine `RW` — salt-okunur olması gereken bölge yazılabilir.
- Bölge önceliği: Armv8-M'de bölgeler **çakışmamalıdır**; çakışma tanımsız
  davranıştır.
- Bölge sayısının yetmemesi — bu hedefte 8 tane var ve gerçek bir sistemde
  çevre birimleri + görev başına stack hızla tüketir.
- 32 byte granularite: "tek bir değişkeni" koruyamazsın, komşuları da aynı
  bölgeye girer. Bu derste `~0x1Fu` maskelemesi tam olarak bunu gösteriyor.

Ayrıca MPU'nun **hiç göremediği** şeyler var: mantık hataları, izinli bir
bölgeye yazılan yanlış veri, zamanlama hataları, yan kanal saldırıları.

Doğru cümle: *"MPU, doğru yapılandırıldığında belirli bir tehdit modeline
karşı donanımsal bir sınır sağlar."* — *"MPU açık, artık güvenliyiz"* değil.

---

## Referanslar

- Armv8-M ARM (Arm DDI 0553) — PMSAv8, `MPU_RBAR`/`MPU_RLAR`/`MPU_CTRL`,
  `MMFSR`, `MMFAR`, `SHCSR.MEMFAULTENA`
- Cortex-M33 Devices Generic User Guide (Arm 100235)
- CMSIS-Core Armv8-M MPU API — `external/CMSIS/Core/Include/m-profile/armv8m_mpu.h`
- QEMU v8.2.2 `hw/misc/tz-ppc.c` — ders 05 PPC gözlemi

---

[← Ders 06](../README.md) · [Ders 07 →](../../07-exceptions/)
