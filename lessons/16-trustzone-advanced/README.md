# Lesson 16 â€” TrustZone & Advanced Features

`[DEEP]` Â· **STATUS: temel geliÅŸtirme doÄŸrulandÄ±; toplu kalite kapÄ±sÄ± bekliyor**

## TEACH â€” Ä°ki gÃ¼venlik durumu, aynÄ± iÅŸlemci

Secure/Non-secure state, privileged/unprivileged ayrÄ±mÄ±ndan farklÄ±dÄ±r.
SAU, platform IDAU'su ve bus tarafÄ±ndaki MPC birlikte Ã§alÄ±ÅŸÄ±r. YalnÄ±z SAU
ayarlamak Non-secure RAM'i kullanÄ±labilir yapmaz. NSC alanÄ±na giriÅŸ SG ile,
Secure'den Non-secure Ã§aÄŸrÄ± BLXNS ile, gateway dÃ¶nÃ¼ÅŸÃ¼ BXNS ile yapÄ±lÄ±r.
CMSE derleyicisi geÃ§iÅŸe ait register temizleme/koruma kodunu da Ã¼retir.

Bu ders tek Secure firmware iÃ§inde TrustZone varmÄ±ÅŸ gibi davranmaz: her
senaryo ayrÄ± link edilmiÅŸ iki ELF yÃ¼kler. [Secure baÅŸlangÄ±Ã§](secure/main.c),
[Non-secure baÅŸlangÄ±Ã§](nonsecure/startup.c) ve [NS uygulamasÄ±](nonsecure/main.c)
ayrÄ±dÄ±r. PaylaÅŸÄ±lan platform/startup ve Ã¶nceki derslerin linker script'i deÄŸiÅŸmez.

## SHOW â€” Bu hedefin bellek bÃ¶lÃ¼mÃ¼

- Secure kod: 0x10000000, 448 KiB bÃ¼tÃ§e.
- NSC veneers: 0x10070000, 4 KiB bÃ¼tÃ§e; gerÃ§ek kullanÄ±lan alan 32 byte.
- Non-secure kod: 0x00080000, 128 KiB.
- Secure RAM: 0x30000000â€“0x30003FFF; son 4 KiB MSP.
- Non-secure RAM: 0x20004000â€“0x20007FFF; son 4 KiB MSP_NS.

Bunlar AN505 alias'larÄ±dÄ±r; fiziksel RAM'ler ikinci kopya deÄŸildir.
Secure linker RAM bÃ¼tÃ§esini 16 KiB ile sÄ±nÄ±rlayarak NS yarÄ±sÄ±yla Ã§akÄ±ÅŸmayÄ± Ã¶nler.
Her iki imajÄ±n initialized .data LMA'sÄ± kendi FLASH bÃ¶lgesine tahsis edilir.

IoT Kit NSCCFG.CODENSC, IDAU'nun Code NSC kullanÄ±mÄ±na izin verir; SAU yalnÄ±z
veneer alanÄ±nÄ± NSC yapar. SSRAM MPC (0x58007000) NS kod bloklarÄ±nÄ±, SRAM MPC
(0x50083000) NS RAM bloklarÄ±nÄ± aÃ§ar. Blok boyu BLK_CFG'den okunur; QEMU'nun
blok boyutu FPGA iÃ§in sabit kabul edilmez. DiÄŸer bloklar reset'teki Secure
durumunda kalÄ±r. NS UART/PPC aÃ§Ä±lmaz; gÃ¼venilir sabit etiketleri Secure taraf basar.

[API](api.h) yalnÄ±z scalar alÄ±r; pointer, buffer veya secret dÃ¶ndÃ¼rmez.
Servis [0,15] girdilerini 2*x+1'e dÃ¶nÃ¼ÅŸtÃ¼rÃ¼r, diÄŸerlerini L16_DENIED ile reddeder.
Bu oyuncak politika kimlik doÄŸrulama, secure boot veya Ã¼rÃ¼n gÃ¼venliÄŸi deÄŸildir.

## RUN

```bash
cmake --preset debug
cmake --build --preset debug
python tests/development-smoke.py 16
```

Bir Ã§ifti elle Ã§alÄ±ÅŸtÄ±rmak iÃ§in (tek Secure ELF yeterli deÄŸildir):

```bash
qemu-system-arm -M mps2-an505 -cpu cortex-m33 -nographic -monitor none -serial stdio \
  -kernel build/debug/lessons/16-trustzone-advanced/16-trustzone-secure.elf \
  -device loader,file=build/debug/lessons/16-trustzone-advanced/16-trustzone-nonsecure.elf
```

CMake Secure link'ten CMSE import nesnesini Ã¼retir; Non-secure link bu nesneye
baÄŸÄ±mlÄ±dÄ±r. AynÄ± senaryonun iki imajÄ±nÄ± kullan. GCC/ld SG veneers Ã¼retir;
gateway gÃ¶vdesine elle normal branch yazÄ±lmaz.

## OBSERVE

Debug / QEMU 11.1.0 Ã§alÄ±ÅŸtÄ±rmasÄ±nda:

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

NS startup kendi .data/.bss'sini kurar. BeÅŸ gateway Ã§aÄŸrÄ±sÄ± 0,7,15 iÃ§in doÄŸru
sonuÃ§larÄ±; 16,0xFFFFFFFF iÃ§in reddi sÄ±nar. Secure taraf cmse_nonsecure_caller
ile Ã§aÄŸrÄ±larÄ±n NS kÃ¶kenini de kontrol eder. NS'nin bildirdiÄŸi sonuÃ§ bitleri
gÃ¼venlik attestation'Ä± deÄŸildir; bu kontrollÃ¼ testin gÃ¶zlemleridir.

ArdÄ±ndan NS kod 0x30000000 okumayÄ± dener. SFSR.AUVIOL=1, Secure handler IPSR=7
ve CFSR/HFSR=0 Ã¶lÃ§Ã¼ldÃ¼. **SFARVALID=0 olduÄŸundan SFAR bir fault adresi olarak
kullanÄ±lmaz.** Test SFARVALID set ise adresi de doÄŸrular; set olmasÄ±nÄ± zorlamaz.
Ä°zin verilen gateway ve reddedilen doÄŸrudan eriÅŸim ayrÄ± kanÄ±tlardÄ±r.

## MODIFY â†’ BREAK â†’ DEBUG

`16-invalid-entry` Ã§ifti SG'nin dÃ¶rt byte sonrasÄ±na dallanÄ±r. Ã–lÃ§Ã¼len
SFSR=0x00000001 (INVEP), SFAR=INVALID; servis gÃ¶vdesi Ã§alÄ±ÅŸmaz, secret korunur.
Beklenen fault bu senaryonun PASS sonucudur.

[AlÄ±ÅŸtÄ±rma](exercises/01-secure-service/README.md) yalnÄ±z scalar girdi politikasÄ±nÄ±
tamamlatÄ±r; fail-closed starter geÃ§erli girdiyi de reddeder.
[Referans Ã§Ã¶zÃ¼m](solutions/01-secure-service/README.md) ayrÄ±dÄ±r.

GDB'de isteÄŸe baÄŸlÄ± gÃ¶zlem (bu aÅŸamada kapsamlÄ± GDB kapÄ±sÄ± Ã§alÄ±ÅŸtÄ±rÄ±lmadÄ±):

```text
break SecureFault_Handler
x/wx 0xE000EDE4
x/wx 0xE000EDE8
print calls
print from_ns
disassemble l16_service
```

Secure ELF sembolleri gateway/handler'Ä± gÃ¶sterir; NS sembolleri iÃ§in
`add-symbol-file build/debug/lessons/16-trustzone-advanced/16-trustzone-nonsecure.elf`
kullanÄ±labilir. cm-frame/cm-fault helper'larÄ±nÄ±n tÃ¼m cross-security frame'leri
decode ettiÄŸi varsayÄ±lmaz. Bu ders frame pointer'Ä±nÄ± tahmin ederek dereference etmez.

## Advanced features â€” hedef sÄ±nÄ±rÄ±

M33, MVE/Helium iÃ§ermez; M55/M85 sonuÃ§larÄ± bu hedefe taÅŸÄ±nmaz. M33 Ã§ekirdeÄŸinde
entegre cache bulunmaz; sistem seviyesinde cache bulunabilmesi ayrÄ± platform
konusudur. PACBTI iÃ§in burada bir runtime deney yapÄ±lmadÄ±; mevcut capability
tablosundaki OLCULMEDI durumu korunur. BaÅŸka CPU'nun instruction desteÄŸi bu
laboratuvarda baÅŸarÄ± kanÄ±tÄ± deÄŸildir.

FPU/DSP'nin Ã§alÄ±ÅŸan kontrolleri [Lesson 13](../13-sync-fpu-dsp/), DWT'nin
ilerlemeyen CYCCNT sÄ±nÄ±rÄ± [Lesson 12](../12-debug-dwt/) ve
[capability kaydÄ±](../../docs/TARGET_CAPABILITIES.md) iÃ§indedir. Bu derste
cycle/performance, MVE, PACBTI veya cache hÄ±zlandÄ±rmasÄ± gÃ¶sterilmez.

## VERIFY

Debug build ve dÃ¶rt Ã§iftin QEMU testi geÃ§ti: Ã¶rnek/Ã§Ã¶zÃ¼m tÃ¼m kontroller PASS,
starter yalnÄ±z CHECK_SERVICE=FAIL; SG atlama beklenen INVEP ile yakalandÄ±.
Disassembly'de SG / BLXNS / BXNS gÃ¶rÃ¼ldÃ¼. Bu sonuÃ§lar gerÃ§ek kart testi veya
tam gÃ¼venlik audit'i deÄŸildir. AyrÄ±ntÄ±lÄ± GDB, Ã¼Ã§-preset karÅŸÄ±laÅŸtÄ±rmasÄ± ve
geniÅŸ negatif testler toplu kalite kapÄ±sÄ±nda yapÄ±lacak.

## Quick Check

1. SAU tek baÅŸÄ±na neden yeterli deÄŸildir?
2. NSC tÃ¼m Secure kodu Non-secure yapar mÄ±?
3. AUVIOL gÃ¶rÃ¼nce SFAR neden otomatik geÃ§erli sayÄ±lamaz?
4. Scalar API bu Ã¶rnekte hangi pointer problemini ortadan kaldÄ±rÄ±r?
5. Bu testin PASS olmasÄ± secure boot kanÄ±tÄ± mÄ±?

[Cevaplar](answers/quick-check.md)

## References

- [Cortex-M33 DUG](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d), 4.5: SAU, SFSR/SFAR.
- [IoT Kit FVP User Guide](https://documentation-service.arm.com/static/5ed13622ca06a95ce53f911b), Tables 15/21/49: SRAM MPC ve NSCCFG.
- [QEMU AN505 implementation](https://github.com/qemu/qemu/blob/v8.2.2/hw/arm/mps2-tz.c): SSRAM MPC ve alias yerleÅŸimi.
- [GCC CMSE](https://gcc.gnu.org/onlinedocs/gcc/ARM-ARMv8-M-Security-Extensions.html): nonsecure_entry / nonsecure_call.
- [QEMU fault handling](https://github.com/qemu/qemu/blob/v8.2.2/target/arm/tcg/m_helper.c): NS data attribution fault'unda AUVIOL yolu; model gÃ¶zleminin baÄŸlamÄ±.

**Previous:** [15](../15-mini-kernel/) Â· **Next:** toplu kalite kapÄ±sÄ±, sonra [projeler](../../projects/).
