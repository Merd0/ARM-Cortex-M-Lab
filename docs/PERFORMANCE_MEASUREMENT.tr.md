**Dil:** [English](PERFORMANCE_MEASUREMENT.md) | Türkçe

# Performance Measurement Methodology

## Bu hedefteki sınır

Bu target'ta DWT cycle counter **saymiyor** (olculdu, bkz.
`docs/TARGET_CAPABILITIES.md`). Yani en yaygin Cortex-M profiling yontemi
burada kullanilamaz.

```text
DWT CYCCNT PROFILING: NOT SUPPORTED ON CURRENT TARGET (QEMU mps2-an505)
```

Bu nedenle aşağıdaki ölçüm yöntemlerini kullanıyoruz.

## QEMU'da gecerli olculer

### 1. Kod boyutu (guvenilir)

```bash
arm-none-eabi-size build/debug/.../firmware.elf
arm-none-eabi-nm --size-sort -S build/debug/.../firmware.elf | tail -20
```

`-O0` ve `-Os` arasindaki fark burada gercek ve tekrarlanabilirdir.

### 2. Uretilen komut sayisi ve komut sekli (guvenilir)

```bash
arm-none-eabi-objdump -d -S firmware.elf > firmware.lst
```

Bir fonksiyonun `-O0` ve `-O2` hallerindeki komut sayisini saymak,
optimizasyonun ne yaptigini anlamak icin en dogrudan yoldur.

### 3. Statik komut sayimi

```bash
arm-none-eabi-objdump -d firmware.elf | grep -c "^\s*1[0-9a-f]*:"
```

### 4. QEMU `-d in_asm` — çevrilen blokların kaydı

```bash
qemu-system-arm -M mps2-an505 -kernel firmware.elf -nographic \
                -d in_asm -D trace.log
```

`-d in_asm`, QEMU'nun **çevirdiği** temel blokların (translation
block) disassembly'sini loglar; icra edilen komut sayısını vermez. QEMU bir
bloğu bir kez çevirir, sonra onu kaç kez çalıştırırsa çalıştırsın **tekrar
loglamaz**. Yani:

- Bir dongu 1.000.000 kez donse bile log'da bir kez gorunur.
- Log'daki satır sayısı, icra edilen komut sayısıyla orantılı değildir.
- Hicbir kod yolu ceviriye girmezse hic gorunmez.

`-d in_asm` su isin aracidir: **hangi kodun gercekten calistirildigini**
gormek (kapsam, coverage). "Kac komut calisti" sorusunun cevabi degildir.

Dinamik komut sayimi istiyorsan QEMU'nun bunun icin ayri mekanizmalari
vardir (ornegin TCG plugin'leri). Bunlar bu reponun kapsamı dışındadır
ve kurulmamıştır. Kullanılacaksa ölçümün amacı önceden belirlenmelidir.
QEMU fonksiyonel bir emulatordur; komut sayisi cycle sayisi degildir.

### 5. SysTick — kullanım alanı ve sınırları

SysTick QEMU'da calisir ve hedef uzerinde **yerel bir aralik/sayim deneyi**
kurmak icin kullanilabilir: "su kadar tick icinde kac iterasyon donduk",
"iki tick arasi bu islem tamamlandi mi". Bunun icin timer modelinin uygun
olmasi ve neyi saydiginin bilinmesi gerekir.

Bu ölçümden, bir algoritmanın fiziksel Cortex-M donanımında diğerinden
daha hızlı olduğu sonucu çıkarılamaz:

- Fonksiyonel emülatör zamanı, donanım cycle zamanı değildir. QEMU komutlari
  dogru yurutur, pipeline/bellek gecikmesi/cache modellemez.
- Bu yuzden SysTick sonucu ne donanim performans siralamasi ne de WCET kurar.
- Ayni firmware icinde iki kod yolunu karsilastirmak istiyorsan bile,
  çıkan oranın donanımda korunacağı varsayılamaz; orada bellek gecikmesi
  ve komut zamanlamalari tamamen farklidir.

Kisacasi SysTick burada bir **deney araci**dir, bir **kiyaslama (benchmark)
araci** degil.

## Yedi ölçüm kavramı

Asagidaki yedi kavram sik sik ayni cumlede kullanilir ve birbirinin yerine
gecmez. Bir olcum paylasirken hangisini yaptigini acikca yaz.

| # | Ne | Ne verir | Bu repoda |
|---|---|---|---|
| 1 | Statik komut/disassembly analizi | Kod boyutu, komut sayisi (statik) | ✅ kullaniliyor |
| 2 | Cevrilmis blok loglama (`-d in_asm`) | Hangi kod cevrildi | ⚠️ sayac DEGIL |
| 3 | Dinamik icra edilen komut sayimi | Kac komut calisti | ⬜ kurulmadi |
| 4 | Fonksiyonel emulator zamani (duvar saati) | Hicbir sey (host'a bagli) | ❌ kullanilmaz |
| 5 | Acikca zamanlama/cycle-approximate model | Yaklasik cycle | ⬜ yok |
| 6 | Gercek donanimda cycle olcumu | Gercek cycle | ⬜ donanim yok |
| 7 | En kotu durum yurutme analizi (WCET) | Kanitlanmis ust sinir | ⬜ kapsam disi |

Iki siklikla yapilan hata:

- **Gozlenen maksimum ≠ WCET.** Olcum yalnizca calistirdigin girdileri
  kapsar. "Gozlenen maksimum X" yazilabilir; "WCET X'tir" yazilamaz. Gercek
  WCET analizi statik analiz araclari ve genellikle sertifikasyon gerektirir.
- **FVP otomatik olarak cycle-approximate degildir.** Siradan bir Fixed
  Virtual Platform **fonksiyonel** bir modeldir ve cycle sayisi vermez.
  Zamanlama bilgisi yalnizca acikca oyle belgelenmis modellerden gelir.
  Hangi modelin ne sundugunu urun dokumantasyonundan dogrula.

## Olcum yaparken kacinilmasi gerekenler

1. **Olcum bolgesinde console I/O yapma.** UART bekleme dongusu her seyi golgeler.
2. **Derleyici sonucu atmasin.** Sonucu `volatile` bir degiskene yaz, yoksa
   `-O2` tum hesabi silebilir.
3. **Isinma/tekrar.** Tek olcum yerine birkac tekrar.
4. **Ayni flag'lerle karsilastir.** `-O0` bir firmware ile `-O2` digerini
   kiyaslarken sadece optimizasyonun degistiginden emin ol.
5. **Mutlak sayi verme.** QEMU sonucu "su kadar mikrosaniye" diye yazma.

## Rapor formati

Bir olcum paylasirken en az sunlari yaz:

```text
Target      : QEMU 8.2.2 mps2-an505 / cortex-m33
Toolchain   : arm-none-eabi-gcc 13.2.1
Optimization: -O0 / -O2
Olcu        : kod boyutu (byte) + komut sayisi
Gecersiz    : cycle / wall-clock (bu target'ta anlamsiz)
```

## Gercek donanimda ne degisir?

Board üzerinde erişim izinleri, DWT/cycle-counter varlığı ve gerçekten
ilerlediği ayrıca doğrulanırsa `DWT->CYCCNT` ile:

- cycle bazli olcum mumkun olur
- flash wait state, cache, branch prediction etkileri gorunur
- ayni kod QEMU'dakinden farkli siralanabilir

Ayrintisi: `docs/REAL_HARDWARE_MIGRATION.md`

## Çalışan deney

[Ders 12](../lessons/12-debug-dwt/README.tr.md) readback, çalışmayan cycle counter,
SysTick yerel aralığı, I/O ile kirletilen ölçüm ve yalnızca iş yükünde O0/O2
karşılaştırmasını çalıştırır. `tests/lesson12-check.py` disassembly ve gerçek
GDB watchpoint kontrollerini üç preset üzerinde tekrarlar.
