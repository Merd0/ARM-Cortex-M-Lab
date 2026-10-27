**Dil:** [English](README.md) | Türkçe

# Solution 01 — Add an SVC Service

Exercise: [`../../exercises/01-add-svc-service/`](../../exercises/01-add-svc-service/README.tr.md)

## Çözüm

```c
case SVC_GET_VERSION:
    result = 0x00010002u;
    break;
```

Tek satırlık iş. Değerli olan, o satırın **nereye** ve **neden** oraya
konduğu.

## Build & Run

```bash
cmake --build --preset debug --target 05-solution-svc2
./scripts/run-qemu.sh build/debug/lessons/05-privilege-svc/05-solution-svc2.elf
```

Gerçek çıktı (ilgili satırlar):

```text
[5] SVC uzerinden ayricalikli servis
[  SVC1_RESULT] 1
[  SVC1_RESULT_AGAIN] 2
[  CHECK_SVC_SERVICE] PASS
[  CONTROL_IN_THREAD_AFTER_SVC] 0x00000003
[  CHECK_SVC_DID_NOT_PROMOTE] PASS
[6] SVC #2 (get_service_version)
[  SVC2_VERSION] 0x00010002
[  CHECK_SVC2_VERSION] PASS
```

## Zincirin tamamı

### 1. `SVC` komutu nasıl üretildi?

`SVC`'nin immediate'i **derleme zamanı sabitidir**; bir değişken olamaz. Bu
yüzden her servis için satır içi assembly gerekiyor:

```c
#define SVC_CALL(num)                                                   \
    ({                                                                  \
        register uint32_t _r0 __asm__("r0");                            \
        __asm__ volatile ("svc %[imm]"                                  \
                          : "=r" (_r0)                                  \
                          : [imm] "I" (num)                             \
                          : "memory");                                  \
        _r0;                                                            \
    })
```

`"I"` kısıtı derleyiciye "bu bir immediate sabit olmalı" der. `register ...
__asm__("r0")` ile sonucu `r0`'dan alıyoruz — çünkü exception return `R0`'ı
frame'den geri yükler.

Üretilen kod (ölçüldü):

```text
10000194:  df01   svc  1
100001a2:  df04   svc  4
```

16 bit, `0xDF` + `imm8`.

### 2. Immediate nasıl çözüldü?

```c
const uint8_t *svc_instr = (const uint8_t *)(frame->pc - 2u);
uint32_t svc_number = (uint32_t)svc_instr[0];
```

Yığılmış PC, `SVC`'den **sonraki** komutu gösterir. `SVC` 16 bit olduğu için
komut `pc - 2`'dedir ve düşük byte `imm8`'dir.

GDB'de doğrulandı:

```text
(gdb) x/2i (*(unsigned int*)($psp+24))-2
   0x10000194 <thread_entry+104>:  svc  1
   0x10000196 <thread_entry+106>:  str  r0, [r4, #28]
```

Bu hesap **yalnızca 16 bitlik `SVC` için** geçerlidir; genel bir "hatalı
komut `pc - 2`'dedir" kuralı yoktur (ders 08).

### 3. Argüman ve sonuç nerede yaşıyor?

Exception frame'inin ilk word'ü `R0`'dır. Donanım exception girişinde `R0`'ı
oraya yazar, exception return'de oradan geri yükler.

```c
frame->r0 = result;      /* Thread bunu R0'da gorecek */
```

Yani servis sonucu, AAPCS'in dönüş değeri register'ına (`r0`) frame üzerinden
yerleştiriliyor. Çağıran C kodu `SVC_CALL(...)` ifadesini normal bir fonksiyon
dönüş değeri gibi okuyabiliyor.

Argüman geçirmek isteseydin aynı yol: Thread `R0`/`R1`'e koyar, dispatcher
`frame->r0` / `frame->r1`'den okur.

### 4. Kesilen frame hangi stack'te?

PSP'de — çünkü Thread PSP kullanıyordu. `svc_handler.S` bunu `EXC_RETURN`
bit 2'den bulur:

```asm
    tst     lr, #4
    ite     eq
    mrseq   r0, msp
    mrsne   r0, psp
```

Ölçüm: handler girişinde `$lr = 0xfffffffd` (bit 2 = 1) ve
`frame == $psp == 0x30000308`.

### 5. Handler neden ayrıcalıklı?

Kodunda özel bir şey olduğu için değil — **Handler Mode'da çalıştığı için.**
Handler Mode mimari olarak her zaman ayrıcalıklıdır. `cm-mode` çıktısı:

```text
Mode      : Handler Mode (exception 11)
Privilege : Privileged (Handler Mode her zaman ayricaliklidir)
            CONTROL.nPRIV = 1 -> KESILEN Thread baglami ayricaliksizdi
```

`CONTROL = 0x1` okunması handler'ın ayrıcalıksız olduğu anlamına gelmez;
kesilen Thread'in durumudur.

### 6. Ayrıcalıksız Thread neden kendini yükseltemedi?

Ayrıcalıksız Thread Mode'dan `CONTROL`'e yapılan yazma yok sayılır. Ölçüm:

```text
CONTROL_UNPRIV           0x00000003
CONTROL_AFTER_ATTEMPT    0x00000003    (degismedi)
```

Fault bile oluşmaz — sessizce etkisiz kalır.

### 7. Exception return Thread'i nasıl devam ettiriyor?

`svc_dispatch` normal bir C fonksiyonu gibi dönüyor. Ama `svc_handler.S` ona
`bl` ile değil **`b` ile** dallandığı için `lr` hâlâ `EXC_RETURN`'ü
(`0xFFFFFFFD`) taşıyor. Fonksiyonun epilogu o değeri `pc`'ye yazdığında
işlemci bunu bir exception return olarak yorumluyor: frame PSP'den geri
yükleniyor, Thread Mode'a dönülüyor.

Ölçüm — dönüşten hemen sonra:

```text
Mode      : Thread Mode
Privilege : Unprivileged (CONTROL.nPRIV = 1)
r0 = 0x1
```

`SVC` Thread'i otomatik yükseltmedi ve servis sonucu `R0`'da geldi.

## Sınırlar

Bu servis sınırı **mimari ayrıcalık akışıdır, bellek izolasyonu değildir.**
`g_service_counter` hâlâ sıradan RAM; ayrıcalıksız kod ona `SVC`'siz de
yazabilir. Ders 06 MPU ile bu doğrudan erişimi engelleyecek.

---

[← Ders 05](../../README.tr.md)
