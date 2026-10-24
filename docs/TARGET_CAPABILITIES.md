# Target Capabilities

Tablodaki bilgiler `tools/capability-probe` firmware'i ile ölçülmüş
veya araç çıktısından (`qemu-system-arm -machine help`) doğrulanmıştır.
Henüz kontrol edilmeyen özellikler `OLCULMEDI` olarak işaretlenmiştir.

## Olcum yontemi

```bash
cmake --preset debug
cmake --build --preset debug --target capability-probe
./scripts/run-qemu.sh build/debug/tools/capability-probe/capability-probe.elf
```

Probe firmware'i ID register'larini okur (CPUID, MPU_TYPE, SAU_TYPE, MVFR0-2,
ID_ISAR, DWT_CTRL) ve DWT cycle counter'ın sayıp saymadığını test eder.

## Capability matrix

| Feature | QEMU M33 (mps2-an505) | QEMU M55 (mps3-an547) | QEMU M85 | Arm FVP | Gercek donanim |
|---|---|---|---|---|---|
| Makine mevcut mu | EVET (olculdu) | EVET (`-machine help`) | **HAYIR** (bu QEMU'da yok) | OLCULMEDI | - |
| Programmer model / registers | EVET (olculdu) | OLCULMEDI | - | OLCULMEDI | EVET |
| Thread / Handler Mode | EVET (olculdu) | OLCULMEDI | - | OLCULMEDI | EVET |
| MSP / PSP | EVET (olculdu) | OLCULMEDI | - | OLCULMEDI | EVET |
| Privilege (CONTROL.nPRIV) | EVET (olculdu) | OLCULMEDI | - | OLCULMEDI | EVET |
| MPU | EVET - 8 region (olculdu) | OLCULMEDI | - | OLCULMEDI | EVET |
| TrustZone / SAU | EVET - 8 region (olculdu) | OLCULMEDI | - | OLCULMEDI | Cihaza bagli |
| Exceptions / NVIC / faults | EVET (olculdu, bkz. asagi) | OLCULMEDI | - | OLCULMEDI | EVET |
| FPU (single precision) | VAR ama CPACR kapali (olculdu) | OLCULMEDI | - | OLCULMEDI | EVET |
| DSP extension | VAR (ID_ISAR, olculdu) | OLCULMEDI | - | OLCULMEDI | EVET |
| MVE / Helium | YOK (M33'te mimari olarak yok) | Mimaride var, OLCULMEDI | - | OLCULMEDI | Cihaza bagli |
| **DWT cycle counter (CYCCNT)** | **SAYMIYOR** (olculdu) | OLCULMEDI | - | OLCULMEDI | EVET |
| Cache | M33'te yok (mimari) | OLCULMEDI | - | OLCULMEDI | Cihaza bagli |
| PACBTI | OLCULMEDI | OLCULMEDI | - | OLCULMEDI | Cihaza bagli |
| Gercekci timing | **HAYIR** | HAYIR | - | Daha iyi | EVET |
| Trace (ITM/ETM) | OLCULMEDI | OLCULMEDI | - | OLCULMEDI | Cihaza bagli |

`OLCULMEDI` = henuz test edilmedi. Test edildikce bu tablo guncellenir.
`-` = makine bu QEMU surumunde mevcut olmadigi icin uygulanamaz.

## Olculen ham degerler (QEMU 8.2.2, mps2-an505, cortex-m33)

`capability-probe` ciktisi:

```text
===== TARGET CAPABILITY PROBE =====
[CPUID] 0x410FD213
[CPUID.PARTNO] 0x00000D21
[CPUID.VARIANT] 0x00000000
[CPUID.REVISION] 0x00000003
[MPU_TYPE] 0x00000800
[MPU_REGIONS] 8
[SAU_TYPE] 0x00000008
[SAU_REGIONS] 8
[MVFR0] 0x10110021
[MVFR1] 0x11000011
[MVFR2] 0x00000040
[MVFR1.MVE] 0x00000000
[CPACR] 0x00000000
[ID_ISAR0] 0x01101110
[ID_ISAR1] 0x02212000
[ID_ISAR2] 0x20232232
[ID_ISAR3] 0x01111131
[ID_ISAR4] 0x01310132
[ID_ISAR5] 0x00000000
[DWT_CTRL] 0x00000000
[DWT_CYCCNT_IMPLEMENTED] YES
[DWT_CYCCNT_T0] 0
[DWT_CYCCNT_T1] 0
[DWT_CYCCNT_COUNTING] NO
[SYSTICK_CALIB] 0x80000000
===== PROBE COMPLETE =====
```

Yorumlar:

- `CPUID.PARTNO = 0xD21` -> Cortex-M33, revision r0p3.
- `MPU_TYPE = 0x800` -> DREGION alani (bit 15:8) = 8 -> **8 MPU region**.
- `SAU_TYPE = 8` -> 8 SAU region -> TrustZone mevcut.
- `MVFR1.MVE = 0` -> MVE/Helium yok. Bu QEMU eksigi degil: **Cortex-M33 mimarisi MVE icermez.**
- `CPACR = 0` -> FPU reset'te kapali. Kullanmak icin CP10/CP11 acilmalidir (ders 13).

## DWT: en onemli uyari

`DWT_CTRL` okundugunda `NOCYCCNT` biti temiz, yani register "cycle counter var"
diyor. Ancak `CYCCNTENA` acildiktan sonra bile **CYCCNT 0'da kaliyor**.

```text
[DWT_CYCCNT_T0] 0
[DWT_CYCCNT_T1] 0
[DWT_CYCCNT_COUNTING] NO
```

Yani bu target'ta DWT cycle counter ile profiling **yapilamaz**.

### Bu bir gozlem, kesin hukum degil

Yukaridaki satirlarin hepsi **bu makinede olculmus davranistir**: QEMU 8.2.2,
`-M mps2-an505 -cpu cortex-m33`. Sonuçlar bu yapılandırma için geçerlidir.

Yorumlama sınırları:

- Başka QEMU sürümlerinde ve machine/cpu kombinasyonlarında cycle counter
  davranışı farklı olabilir.
- Bu gözlem tek başına bir QEMU hatası göstermez. QEMU **fonksiyonel** bir
  emülatördür: komutları yürütür, pipeline ve saat çevrimini modellemeyi
  hedeflemez. Cycle sayacı bu modelleme kapsamının dışında kalır.

Bagimsiz destek: RTIC (cortex-m-rtfm) projesinin dokumantasyonu, zamanlama
orneklerinin QEMU'da duzgun calismayacagini, cunku Cortex-M cycle counter
islevinin `qemu-system-arm` icinde uygulanmadigini belirtir
(<https://docs.rs/crate/cortex-m-rtfm/latest/source/book/en/src/by-example/timer-queue.md>).
Ayni sikayet QEMU tabanli baska projelerde de kayitlidir. Bu, bizim olcumumuzu
**destekler** — ama yine de birincil kaynak bizim kendi olcumumuzdur.

Resmi QEMU dokumantasyonunda mps2 board'lari icin DWT cycle counter destegine
dair acik bir ifade bulunamadi; "belgelenmedi" ile "desteklenmiyor" ayni sey
olmadigi icin bu satir da bir iddia olarak degil, arama sonucu olarak
yaziliyor.

### Sonuc

```text
DWT CYCCNT PROFILING: NOT SUPPORTED ON CURRENT TARGET (QEMU mps2-an505)
```

Profiling dersi ve `projects/performance-profiler/` bu kisitin **etrafinda**
kuruldu: statik kod boyutu, statik komut sayisi ve SysTick tabanli **hedef
yerel** aralik/sayim deneyleri kullanilir. Bu ölçümler cycle sayısı vermez ve
bunlardan **donanım performans sıralaması çıkarılmaz** - fonksiyonel
emulator zamani donanim cycle zamani degildir.

Gelecek yol (ikisi de belgelendi, ikisi de denenmedi):

| Yol | Ne saglar | Durum |
|---|---|---|
| Arm FVP (Fixed Virtual Platform) | Fonksiyonel modelleme; zamanlama bilgisi **yalnizca** acikca oyle belgelenmis modellerde | Planli — `docs/REAL_HARDWARE_MIGRATION.md` |
| Gercek donanim (orn. Nucleo, MPS2+) | Gercek CYCCNT + SWO/ETM trace (KART BAZINDA degisir) | Planli — ayni dosya |

Son satirda bir uyari: **her kartta DWT cycle sayaci calisir diye bir kural
yoktur.** DWT'nin kendisi opsiyoneldir, cycle sayaci DWT icinde ayrica
opsiyoneldir (`DWT_CTRL.NOCYCCNT`), ve bazi cihazlarda debug altyapisi
uretici tarafindan kisitlanmis olabilir. Bu repodaki probe zaten dogru soruyu
soruyor: "register var mi" ile "sayiyor mu" ayri seylerdir. Gercek donanima
gecerken ayni probe'u calistir.

Deneyler planlanırken bu simülatör sınırları dikkate alınmalıdır.

## Not: SAU nasil okundu

`tools/capability-probe` SAU_TYPE'i CMSIS uzerinden degil, mimari sabit
adresinden (`SCS_BASE + 0x0DD4`) okur. Sebep: CMSIS-Core, SAU tanimlarini
yalnizca derleyici `-mcmse` ile cagrildiginda acar ve bu repo su an `-mcmse`
kullanmiyor. Ders 16'da secure/non-secure ayrimi kurulurken bu degisecek.
Ayrinti: [`docs/ARCHITECTURE.md`](ARCHITECTURE.md).

## Exception/fault davranisi (olculdu)

Ders 02'deki bilerek bozulmus firmware (`02-programmer-model-broken`)
QEMU + GDB uzerinde su degerleri uretti:

```text
IPSR  = 3            -> HardFault
CFSR  = 0x00009200   -> BFSR = 0x92 (BFARVALID + STKERR + PRECISERR)
HFSR  = 0x40000000   -> FORCED (escalation)
BFAR  = 0xFFFFFFF8
LR    = 0xFFFFFFFD   -> EXC_RETURN
```

Yani fault escalation, fault status register'lari ve EXC_RETURN bu target'ta
çalışıyor. Ders 07/08 deneyleri bu davranışları kullanıyor.

## M55 / M85 / FVP neden bos?

- **M55 (mps3-an547):** Makine bu QEMU'da var. Ancak farkli bir bellek haritasi
  kullaniyor (ITCM/DTCM, farkli UART adresi), yani ayri bir platform portu
  gerekiyor. Port yazilana kadar tablo `OLCULMEDI` kalir. DEEP TRACK gorevi.
- **M85:** Bu QEMU surumunde (8.2.2) `-cpu help` ciktisinda cortex-m85 **yok**
  ve karşılık gelen bir MPS3 makinesi listelenmiyor.
- **Arm FVP:** Ayri kurulum/lisans gerektirir, bootstrap sirasinda kurulmadi.
  Ayrica dikkat: "FVP" otomatik olarak zamanlama-dogru veya cycle-approximate
  bir model anlamına gelmez. Siradan bir FVP fonksiyonel bir modeldir. Bir
  modelin zamanlama ozelligi ancak kendi dokumantasyonunda acikca yaziyorsa
  vardir; bu repoda hicbir FVP dogrulanmamistir.

Kontrol komutlari:

```bash
qemu-system-arm -machine help | grep mps
qemu-system-arm -cpu help | grep cortex-m
```

## Lesson 12 tekrar ölçümü — 2026-10-23

Windows / QEMU `11.1.0 (v11.1.0-12130-ge470268ff4)`, Cortex-M33 AN505:
DEMCR.TRCENA ve DWT.CTRL.CYCCNTENA yazma denemesinden sonra her iki
register readback'i `0x00000000`; CYCCNT başlangıç/bitiş `0/0`.
GDB write/read watchpoint ise gdbstub üzerinden çalıştı. Bu, fiziksel DWT
comparator desteğinin kanıtı değildir. Ayrıntılar ve tekrar komutları:
[Lesson 12](../lessons/12-debug-dwt/).


## Lesson 13 fonksiyonel tekrar — 2026-10-24

Yerel Windows / GCC 15.2.1 / QEMU 11.1.0, `mps2-an505 / cortex-m33`:

- CPACR=0 iken ilk FP komutu NOCP UsageFault (CFSR=0x00080000,
  HFSR=0, IPSR=6). CP10/CP11 açıldıktan sonra FP toplama sonucu 3.5.
- Aktif FP context olmadan basic 32-byte; aktif context ile TS=0,
  Secure→Secure/MSP deneyinde 104-byte extended frame ölçüldü.
- Lazy LSPACT 1→0 ve raw stack slot'unun ilk handler FP komutuyla
  dolması, ardından S0–S2 restore edilmesi GDB'de doğrulandı.
- Çağıranın FZ=1 FPSCR kontrol durumu worker/exception dönüşünde korundu.
  Denormal aritmetik davranışı bu deneyde ölçülmedi.
- SMLAD sonucu bounded C referansıyla eşleşti; QADD iki yönde saturation
  ve Q flag üretti. Bu fonksiyonel doğrulamadır; cycle/hız ölçümü değildir.
- Exception ile bozulan exclusive reservation ve retry; PRIMASK'ın önceki
  değere geri yüklenmesi üç preset'te doğrulandı. DMA/global monitor veya
  çok çekirdekli atomiklik doğrulaması yapılmadı.

Tam komutlar, kapsam ve kayıtlar: [Lesson 13](../lessons/13-sync-fpu-dsp/).
Bu sonuçlar önceki ID snapshot'ını değiştirmez; başka QEMU sürümü veya
fiziksel hedefte yeniden ölçülmelidir. Ders 13 uzak CI sonucu ayrıca izlenir.
