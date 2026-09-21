# Lesson 05 — Quick Check Cevapları

> Önce kendi cevabını yaz, sonra aç.

---

### 1. `CONTROL.nPRIV` neyi kontrol eder? Handler Mode'u kapsar mı?

`CONTROL.nPRIV` (bit 0) **Thread Mode'un** ayrıcalık seviyesini tanımlar:
`0` = privileged, `1` = unprivileged.

**Handler Mode'u kapsamaz.** Handler Mode her zaman ayrıcalıklıdır. Bir
handler içinde `CONTROL`'ü okuduğunda `nPRIV = 1` görebilirsin — bu,
handler'ın ayrıcalıksız olduğu anlamına **gelmez**; kesilen Thread bağlamının
ayrıcalıksız olduğunu söyler.

Bu derste ölçüldü: `svc_dispatch` içinde `CONTROL = 0x1` okundu ve `cm-mode`
şunu bastı:

```text
Mode      : Handler Mode (exception 11)
Privilege : Privileged (Handler Mode her zaman ayricaliklidir)
            CONTROL.nPRIV = 1 -> KESILEN Thread baglami ayricaliksizdi
```

Doğru sıra: **önce `IPSR`'den modu belirle, sonra `CONTROL`'ü yorumla.**

---

### 2. Ayrıcalıksız Thread `nPRIV`'i temizleyip ayrıcalık kazanabilir mi?

**Hayır.** Ayrıcalıksız Thread Mode'dan `CONTROL`'e yapılan yazma yok sayılır.

Ölçüm:

```text
CONTROL_UNPRIV          0x00000003    (nPRIV = 1)
--- __set_CONTROL(CONTROL & ~1) calistirildi ---
CONTROL_AFTER_ATTEMPT   0x00000003    (degismedi)
```

Fault yok, hata yok, uyarı yok — yazma sessizce etkisiz kalıyor. Ayrıcalık
kazanmanın tek yolu **ayrıcalıklı bir yürütme yolundan** geçmek: bir exception
(örneğin SVC) handler'a girer, handler ayrıcalıklıdır ve isterse dönecek
bağlam için `nPRIV`'i temizler.

Bu derste `SVC #4` servisi tam olarak bunu yapıyor — ama dikkat: o satırı
**handler** çalıştırıyor. Aynı satır Thread'den çalıştırıldığında yok
sayılmıştı. Kodu değil, **nereden çalıştığı** belirliyor.

---

### 3. `SVC_Handler` neden ayrıcalıklı iş yapabiliyor?

Handler'ın kodunda özel bir şey olduğu için değil. **Handler Mode'da çalıştığı
için.**

Bir exception alındığında işlemci Handler Mode'a geçer ve Handler Mode
mimari olarak ayrıcalıklıdır. Yani ayrıcalık, kodun kimliğinden değil,
**yürütme bağlamından** gelir.

Pratik sonucu önemli: `SVC_Handler` içinde çağırdığın herhangi bir fonksiyon
da ayrıcalıklı çalışır. Bu yüzden handler'ın ne yaptığı dikkatle
sınırlandırılmalıdır — bir syscall dispatcher'ının tanımadığı isteği
reddetmesinin (Break It bölümü) sebebi budur.

Vektör tablosundaki 11. girdinin `SVC_Handler`'ı göstermesi, "bu fonksiyon
SVCall exception'ında çağrılacak" demektir; ayrıcalığı veren tablo değil,
exception'ın kendisidir.

---

### 4. "Unprivileged" ile "Non-secure" aynı şey mi?

**Hayır.** İki ayrı mekanizma, iki dikey eksen.

| | Ayrıcalık | Güvenlik durumu |
|---|---|---|
| Kontrol | `CONTROL.nPRIV` + mod | TrustZone / SAU |
| Değerler | Privileged / Unprivileged | Secure / Non-secure |
| Var mı | Her Cortex-M'de | Yalnızca Security Extension varsa |

Dört kombinasyon da mümkündür: Secure-privileged, Secure-unprivileged,
Non-secure-privileged, Non-secure-unprivileged. Non-secure tarafın da kendi
ayrıcalıklı kodu (örneğin Non-secure bir RTOS çekirdeği) vardır.

Bu ders tek güvenlik durumunda (Secure) çalışır ve yalnızca ayrıcalık
eksenini değiştirir. Ayrıntı ders 16'da.

---

### 5. Thread PSP kullanırken SVC olursa frame nereye yazılır?

**PSP'ye** — yani exception anında aktif olan stack'e. Donanım, kesilen
bağlamın kullandığı stack'i kullanır.

Ölçüm: handler girişinde `$psp = 0x30000308` ve sarmalayıcının hesapladığı
`frame = 0x30000308`. Aynı adres.

Bunu `EXC_RETURN` bit 2 söylüyor: `0xFFFFFFFD` → bit 2 = 1 → frame PSP'de.
`svc_handler.S` tam olarak bu biti test ediyor (`tst lr, #4`).

Uyarı: "PSP'ye yazılır" ile "yazma başarılı oldu" aynı şey değildir. PSP
geçersiz veya `PSPLIM` sınırının altındaysa stacking'in kendisi fault üretir
(ders 02 ve ders 04'teki Break It bölümleri).

---

### 6. Handler Mode hangi stack'i kullanır? Kim ayarlar?

**MSP.** Ve bunu **donanım** yapar, yazılım değil.

Exception girişinde işlemci `CONTROL.SPSEL`'i temizler; bu yüzden handler MSP
üzerinde çalışır. Ölçüm bunu doğruluyor:

```text
Thread'de:   CONTROL = 0x3   (SPSEL = 1, PSP)
Handler'da:  CONTROL = 0x1   (SPSEL = 0, MSP)
             $sp == $msp == 0x30007ff0
```

`SPSEL` bitini hiçbir yazılım satırı temizlemedi.

Tasarımın sebebi: handler kodu, hatalı bir görevin stack'ine bağımlı
olmamalıdır. Görev stack'i taşmış veya bozulmuşsa bile handler temiz bir MSP
üzerinde çalışmaya devam eder.

(Tek güvenlik durumu varsayımıyla. TrustZone varsa bu, *mevcut güvenlik
durumunun* MSP'sidir.)

---

### 7. `SVC` dönen Thread'i otomatik ayrıcalıklı yapar mı?

**Hayır.** Bu dersin en önemli ölçümü.

`SVC #1`'den döndükten hemen sonra:

```text
Mode      : Thread Mode
Privilege : Unprivileged (CONTROL.nPRIV = 1)
$control = 0x3
```

`SVC` yalnızca **kapıdır**: ayrıcalıklı bir bağlama giriş sağlar. Dönerken ne
olacağına **handler'ın politikası** karar verir.

Bu derste iki farklı politika aynı anda gösteriliyor:

- `SVC #1` (increment): iş yapar, ayrıcalığa dokunmaz → Thread ayrıcalıksız
  döner.
- `SVC #4` (restore): handler `nPRIV`'i temizler → Thread ayrıcalıklı döner.

Gerçek bir RTOS'ta ikincisi genellikle yapılmaz; görevler ayrıcalıksız kalır.
Burada mekanizma görünsün diye bilerek yapıldı.

---

### 8. (Reasoning) `SVC` immediate'i neden `PC - 2`'den okunabiliyor?

İki gerçeğin birleşimi:

1. **Yığılmış PC, `SVC` komutundan sonraki komutu gösterir.** Donanım,
   kesilen bağlamın **devam edeceği** adresi yığar.
2. **`SVC`'nin Thumb T1 kodlaması 16 bittir:** `0b11011111:imm8`, yani
   `0xDFxx`. Komut 2 byte uzunluğunda, düşük byte `imm8`.

Dolayısıyla `pc - 2` komutun başlangıcı ve o adresteki byte `imm8`.

Bu build'de ölçüldü:

```text
10000194:  df01   svc  1        <- pc - 2
10000196:                       <- yigilmis pc
```

**Her exception için geçerli mi? Hayır.** Bu hesap yalnızca 16 bitlik `SVC`
komutu için doğrudur. Genelleme iki yerde kırılır:

- **32 bitlik komutlar**: hatalı komut 4 byte uzunluğundaysa `pc - 2` komutun
  ortasına düşer.
- **Fault'larda yığılmış PC'nin anlamı değişir.** Precise bir fault'ta
  yığılmış PC hatalı komutu gösterebilir; imprecise bir fault'ta hatanın
  gerçekleştiği komut çoktan geride kalmış olabilir. Ders 08 bunu açacak.

Doğru refleks: varsayma, `objdump` ile üretilen kodlamaya bak.

---

### 9. (Reasoning) Gerçek bir izole syscall sınırı için ne eksik?

**Bellek izolasyonu.** Bu ders ayrıcalık *akışını* kurdu, korumayı değil.

`g_service_counter` sıradan bir RAM değişkenidir. Ayrıcalıksız Thread onu
`SVC` yolundan geçmeden **doğrudan** artırabilir — ve hiçbir şey olmaz.

Nasıl kontrol edersin? Ayrıcalıksız bölüme şu satırı ekle ve çalıştır:

```c
g_service_counter = 1000u;      /* SVC'siz, dogrudan */
```

Sonra `SVC #1` çağır ve dönen değere bak. `1001` görüyorsan sözleşme
delinmiştir. (Denedikten sonra satırı geri al.)

Eksik olan şey listesi:

| Şey | Şu anki durum |
|---|---|
| Kernel verisi | Ayrıcalıksız koddan yazılabilir |
| Thread stack'i ↔ kernel stack'i | Aynı adres uzayı, donanımsal ayrım yok |
| Kod bölgesi | Ayrıcalıksız koddan okunabilir |
| Bir görevin diğerine yazması | Engellenmiyor |

Ölçülen tek gerçek kısıt çevre birimi erişimiydi (`UART_BAUDDIV_UNPRIV = 0`)
— ve o da MPU'dan değil, SoC'nin **Peripheral Protection Controller**'ından
geliyor. Farklı blok, farklı mekanizma.

**Ders 06 — MPU** bunu tamamlar: `g_service_counter` ayrıcalıksız koda
kapatılır ve doğrudan yazma denemesi bir **MemManage fault** üretir. O zaman
"SVC yolundan geçmek zorundasın" bir sözleşme değil, **donanım kuralı** olur.

---

## Referanslar

- Armv8-M ARM (Arm DDI 0553) — `CONTROL`, Thread/Handler Mode, exception
  entry, `SVC` T1 kodlaması
- Armv8-M Exception Model User Guide (Arm 107706) — SVCall = exception 11,
  `EXC_RETURN`
- Cortex-M33 Devices Generic User Guide (Arm 100235)
- AAPCS32 — `r0` dönüş değeri

---

[← Ders 05](../README.md) · [Ders 06 →](../../06-mpu/)
