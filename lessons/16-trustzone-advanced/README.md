# Lesson 16 — TrustZone & Advanced Features

`[DEEP]` · **STATUS: temel geliştirme doğrulandı; toplu kalite kapısı bekliyor**

## TEACH — İki güvenlik durumu, aynı işlemci

Secure/Non-secure state, privileged/unprivileged ayrımından farklıdır.
SAU, platform IDAU'su ve bus tarafındaki MPC birlikte çalışır. Yalnız SAU
ayarlamak Non-secure RAM'i kullanılabilir yapmaz. NSC alanına giriş SG ile,
Secure'den Non-secure çağrı BLXNS ile, gateway dönüşü BXNS ile yapılır.
CMSE derleyicisi geçişe ait register temizleme/koruma kodunu da üretir.

Bu ders tek Secure firmware içinde TrustZone varmış gibi davranmaz: her
senaryo ayrı link edilmiş iki ELF yükler. [Secure başlangıç](secure/main.c),
[Non-secure başlangıç](nonsecure/startup.c) ve [NS uygulaması](nonsecure/main.c)
ayrıdır. Paylaşılan platform/startup ve önceki derslerin linker script'i değişmez.

## SHOW — Bu hedefin bellek bölümü

- Secure kod: 0x10000000, 448 KiB bütçe.
- NSC veneers: 0x10070000, 4 KiB bütçe; gerçek kullanılan alan 32 byte.
- Non-secure kod: 0x00080000, 128 KiB.
- Secure RAM: 0x30000000–0x30003FFF; son 4 KiB MSP.
- Non-secure RAM: 0x20004000–0x20007FFF; son 4 KiB MSP_NS.

Bunlar AN505 alias'larıdır; fiziksel RAM'ler ikinci kopya değildir.
Secure linker RAM bütçesini 16 KiB ile sınırlayarak NS yarısıyla çakışmayı önler.
Her iki imajın initialized .data LMA'sı kendi FLASH bölgesine tahsis edilir.

IoT Kit NSCCFG.CODENSC, IDAU'nun Code NSC kullanımına izin verir; SAU yalnız
veneer alanını NSC yapar. SSRAM MPC (0x58007000) NS kod bloklarını, SRAM MPC
(0x50083000) NS RAM bloklarını açar. Blok boyu BLK_CFG'den okunur; QEMU'nun
blok boyutu FPGA için sabit kabul edilmez. Diğer bloklar reset'teki Secure
durumunda kalır. NS UART/PPC açılmaz; güvenilir sabit etiketleri Secure taraf basar.

[API](api.h) yalnız scalar alır; pointer, buffer veya secret döndürmez.
Servis [0,15] girdilerini 2*x+1'e dönüştürür, diğerlerini L16_DENIED ile reddeder.
Bu oyuncak politika kimlik doğrulama, secure boot veya ürün güvenliği değildir.

## RUN

```bash
cmake --preset debug
cmake --build --preset debug
python tests/development-smoke.py 16
```

Bir çifti elle çalıştırmak için (tek Secure ELF yeterli değildir):

```bash
qemu-system-arm -M mps2-an505 -cpu cortex-m33 -nographic -monitor none -serial stdio \
  -kernel build/debug/lessons/16-trustzone-advanced/16-trustzone-secure.elf \
  -device loader,file=build/debug/lessons/16-trustzone-advanced/16-trustzone-nonsecure.elf
```

CMake Secure link'ten CMSE import nesnesini üretir; Non-secure link bu nesneye
bağımlıdır. Aynı senaryonun iki imajını kullan. GCC/ld SG veneers üretir;
gateway gövdesine elle normal branch yazılmaz.

## OBSERVE

Debug / QEMU 11.1.0 çalıştırmasında:

```text
[CHECK_PARTITION] PASS
[CHECK_NS_VECTORS] PASS
[CHECK_NS_STARTUP] PASS
[CHECK_SERVICE] PASS
[CHECK_POLICY_DENY] PASS
[CHECK_CALLER_NS] PASS
[SFSR] 0x00000008
[SFAR] INVALID
[CHECK_SECURE_REJECT] PASS
[CHECK_SECRET] PASS
[CHECK_SECURE_HANDLER] PASS
[CHECK_NO_ESCALATION] PASS
```

NS startup kendi .data/.bss'sini kurar. Beş gateway çağrısı 0,7,15 için doğru
sonuçları; 16,0xFFFFFFFF için reddi sınar. Secure taraf cmse_nonsecure_caller
ile çağrıların NS kökenini de kontrol eder. NS'nin bildirdiği sonuç bitleri
güvenlik attestation'ı değildir; bu kontrollü testin gözlemleridir.

Ardından NS kod 0x30000000 okumayı dener. SFSR.AUVIOL=1, Secure handler IPSR=7
ve CFSR/HFSR=0 ölçüldü. **SFARVALID=0 olduğundan SFAR bir fault adresi olarak
kullanılmaz.** Test SFARVALID set ise adresi de doğrular; set olmasını zorlamaz.
İzin verilen gateway ve reddedilen doğrudan erişim ayrı kanıtlardır.

## MODIFY → BREAK → DEBUG

`16-invalid-entry` çifti SG'nin dört byte sonrasına dallanır. Ölçülen
SFSR=0x00000001 (INVEP), SFAR=INVALID; servis gövdesi çalışmaz, secret korunur.
Beklenen fault bu senaryonun PASS sonucudur.

[Alıştırma](exercises/01-secure-service/README.md) yalnız scalar girdi politikasını
tamamlatır; fail-closed starter geçerli girdiyi de reddeder.
[Referans çözüm](solutions/01-secure-service/README.md) ayrıdır.

GDB'de isteğe bağlı gözlem (bu aşamada kapsamlı GDB kapısı çalıştırılmadı):

```text
break SecureFault_Handler
x/wx 0xE000EDE4
x/wx 0xE000EDE8
print calls
print from_ns
disassemble l16_service
```

Secure ELF sembolleri gateway/handler'ı gösterir; NS sembolleri için
`add-symbol-file build/debug/lessons/16-trustzone-advanced/16-trustzone-nonsecure.elf`
kullanılabilir. cm-frame/cm-fault helper'larının tüm cross-security frame'leri
decode ettiği varsayılmaz. Bu ders frame pointer'ını tahmin ederek dereference etmez.

## Advanced features — hedef sınırı

M33, MVE/Helium içermez; M55/M85 sonuçları bu hedefe taşınmaz. M33 çekirdeğinde
entegre cache bulunmaz; sistem seviyesinde cache bulunabilmesi ayrı platform
konusudur. PACBTI için burada bir runtime deney yapılmadı; mevcut capability
tablosundaki OLCULMEDI durumu korunur. Başka CPU'nun instruction desteği bu
laboratuvarda başarı kanıtı değildir.

FPU/DSP'nin çalışan kontrolleri [Lesson 13](../13-sync-fpu-dsp/), DWT'nin
ilerlemeyen CYCCNT sınırı [Lesson 12](../12-debug-dwt/) ve
[capability kaydı](../../docs/TARGET_CAPABILITIES.md) içindedir. Bu derste
cycle/performance, MVE, PACBTI veya cache hızlandırması gösterilmez.

## VERIFY

Debug build ve dört çiftin QEMU testi geçti: örnek/çözüm tüm kontroller PASS,
starter yalnız CHECK_SERVICE=FAIL; SG atlama beklenen INVEP ile yakalandı.
Disassembly'de SG / BLXNS / BXNS görüldü. Bu sonuçlar gerçek kart testi veya
tam güvenlik audit'i değildir. Ayrıntılı GDB, üç-preset karşılaştırması ve
geniş negatif testler toplu kalite kapısında yapılacak.

## Quick Check

1. SAU tek başına neden yeterli değildir?
2. NSC tüm Secure kodu Non-secure yapar mı?
3. AUVIOL görünce SFAR neden otomatik geçerli sayılamaz?
4. Scalar API bu örnekte hangi pointer problemini ortadan kaldırır?
5. Bu testin PASS olması secure boot kanıtı mı?

[Cevaplar](answers/quick-check.md)

## References

- [Cortex-M33 DUG](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d), 4.5: SAU, SFSR/SFAR.
- [IoT Kit FVP User Guide](https://documentation-service.arm.com/static/5ed13622ca06a95ce53f911b), Tables 15/21/49: SRAM MPC ve NSCCFG.
- [QEMU AN505 implementation](https://github.com/qemu/qemu/blob/v8.2.2/hw/arm/mps2-tz.c): SSRAM MPC ve alias yerleşimi.
- [GCC CMSE](https://gcc.gnu.org/onlinedocs/gcc/ARM-ARMv8-M-Security-Extensions.html): nonsecure_entry / nonsecure_call.
- [QEMU fault handling](https://github.com/qemu/qemu/blob/v8.2.2/target/arm/tcg/m_helper.c): NS data attribution fault'unda AUVIOL yolu; model gözleminin bağlamı.

**Previous:** [15](../15-mini-kernel/) · **Next:** toplu kalite kapısı, sonra [projeler](../../projects/).
