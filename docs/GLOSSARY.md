# Glossary - Embedded/Arm Terimleri

Terimler, resmi dokümantasyonu takip etmeyi kolaylaştırmak için İngilizce
biçimleriyle verilmiştir. Yanlarında kısa Türkçe açıklamaları yer alır.

## Mimari

| Terim | Kisa aciklama |
|---|---|
| **programmer model** (programci modeli) | Yazilimin gordugu register/mode/state kumesi |
| **Thread Mode** (is parcacigi modu) | Normal uygulama kodunun calistigi mod |
| **Handler Mode** (isleyici modu) | Exception islerken girilen mod, her zaman privileged |
| **privileged / unprivileged** (ayricalikli / ayricalisiz) | Sistem register'larina erisim hakki olan/olmayan calisma seviyesi |
| **T32 / Thumb** | Cortex-M'in kullandigi komut seti; 16 ve 32 bit komutlar karisik |
| **Thumb bit** | Adreslerin bit 0'i; 1 olmak zorunda, 0 ise UsageFault |
| **banked register** (bolunmus register) | Mode/state'e gore farkli kopyasi olan register (orn. MSP/PSP) |

## Register'lar

| Terim | Kisa aciklama |
|---|---|
| **R0-R12** | Genel amacli register'lar |
| **SP (R13)** | Stack pointer; aktif olan MSP veya PSP'dir |
| **MSP** (ana stack pointer) | Main Stack Pointer - reset ve handler'larda kullanilir |
| **PSP** (surec stack pointer) | Process Stack Pointer - genelde task/uygulama stack'i |
| **LR (R14)** (baglanti register'i) | Donus adresi; exception icinde EXC_RETURN tutar |
| **PC (R15)** (program sayaci) | Calisan komutun adresi |
| **xPSR** (program durum register'i) | APSR + IPSR + EPSR birlesimi |
| **APSR** | N/Z/C/V gibi kosul bayraklari |
| **IPSR** | Calisan exception numarasi (0 = Thread Mode) |
| **CONTROL** | nPRIV (privilege) ve SPSEL (hangi stack) bitleri |

## Stack ve cagri

| Terim | Kisa aciklama |
|---|---|
| **full descending stack** (tam azalan yigin) | SP once azalir sonra yazilir; yukaridan asagi buyur |
| **AAPCS** | Arm fonksiyon cagri sozlesmesi (argumanlar, donus, korunan register'lar) |
| **caller-saved** (cagiran korur) | R0-R3, R12; cagrilan fonksiyon bozabilir |
| **callee-saved** (cagrilan korur) | R4-R11; kullanilacaksa kaydedilip geri yuklenmeli |
| **stack frame** (yigin cercevesi) | Bir fonksiyonun stack uzerindeki alani |
| **stack alignment** (yigin hizalama) | AAPCS geregi 8 byte hizalama |

## Exception

| Terim | Kisa aciklama |
|---|---|
| **exception** (istisna) | Normal akisi kesen donanim olayi |
| **exception entry** (istisnaya giris) | Donanimin otomatik yigma + Handler Mode'a gecis sureci |
| **hardware stacking** (donanim yigmasi) | R0-R3, R12, LR, PC, xPSR'nin otomatik yigilmasi |
| **EXC_RETURN** | Exception'da LR'ye konan ozel deger; donus sekliyle ilgili bilgi tasir |
| **NVIC** | Nested Vectored Interrupt Controller - kesme onceliklendirme |
| **vector table** (vektor tablosu) | Exception handler adresleri tablosu |
| **escalation** (yukselme) | Islenemeyen bir fault'un HardFault'a donusmesi |
| **PendSV** | Dusuk oncelikli, context switch icin kullanilan exception |
| **SVC** (supervisor call) | Yazilimin bilerek uretttigi exception - sistem cagrisi |

## Fault

| Terim | Kisa aciklama |
|---|---|
| **HardFault** | En yuksek oncelikli, her seyi yakalayan fault |
| **MemManage** | MPU ihlali |
| **BusFault** | Bellek/bus erisim hatasi |
| **UsageFault** | Gecersiz komut/durum, hizalama, sifira bolme |
| **CFSR** | Configurable Fault Status Register - hatanin turu |
| **MMFAR / BFAR** | Hataya sebep olan adres |

## Bellek ve build

| Terim | Kisa aciklama |
|---|---|
| **MPU** (bellek koruma birimi) | Bolge bazli erisim izni donanimi |
| **linker script** (baglayici betigi) | Section'larin hangi adrese gidecegini tanimlar |
| **section** (bolum) | `.text`, `.rodata`, `.data`, `.bss` gibi kod/veri gruplari |
| **VMA / LMA** | Calisma adresi / yuklenme adresi (`.data` icin farklidir) |
| **map file** (harita dosyasi) | Linker'in nereye ne koydugunu anlatan rapor |
| **ELF** | Derlenmis ciktinin standart dosya formati |
| **startup code** (baslatma kodu) | Reset'ten main'e kadar calisan kod |
| **scatter-loading** | Arm Compiler'in bellek yerlesimi tanimlama yontemi (GNU ld'de linker script) |

## Ileri konular

| Terim | Kisa aciklama |
|---|---|
| **memory barrier** (bellek bariyeri) | DSB/DMB/ISB - erisim ve komut siralamasini zorlar |
| **exclusive access** (ozel erisim) | LDREX/STREX ile atomik guncelleme mekanizmasi |
| **lazy stacking** (tembel yigma) | FPU register'larinin gerekene kadar yigilmamasi |
| **TrustZone** | Secure / Non-secure ayrimi saglayan guvenlik uzantisi |
| **SAU** | Security Attribution Unit - hangi adres secure |
| **NSC** (non-secure callable) | Non-secure kodun cagirabildigi secure giris noktalari |
| **PACBTI** | Pointer Authentication + Branch Target Identification |
| **MVE / Helium** | M-profile vektor uzantisi (Cortex-M55/M85) |
| **DWT** | Data Watchpoint and Trace - watchpoint ve sayaclar |
| **semihosting** | Debugger uzerinden host I/O kullanma yontemi |
