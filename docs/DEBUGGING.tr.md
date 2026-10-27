**Dil:** [English](DEBUGGING.md) | Türkçe

# Cortex-M Debugging Rehberi

Bu dosya komut listesi degil, **teshis akisi** ogretir.
Komutlarin tam listesi icin: `scripts/gdb/cortex-m.gdb`

## Kurulum: iki terminal

Terminal 1 - QEMU'yu durdurulmus baslat:

```bash
./scripts/debug-qemu.sh build/debug/lessons/02-programmer-model/02-programmer-model.elf
```

Terminal 2 - GDB'yi bagla:

```bash
arm-none-eabi-gdb -x scripts/gdb/cortex-m.gdb \
    build/debug/lessons/02-programmer-model/02-programmer-model.elf
(gdb) cm-connect
```

`arm-none-eabi-gdb` yoksa `gdb-multiarch` ayni sekilde calisir.

## Temel komutlar

| Komut | Ne yapar |
|---|---|
| `break main` / `b main` | Fonksiyonda dur |
| `continue` / `c` | Devam et |
| `next` / `n` | Bir C satiri (fonksiyona girmez) |
| `step` / `s` | Bir C satiri (fonksiyona girer) |
| `stepi` / `si` | **Bir assembly komutu** |
| `finish` | Fonksiyondan cik |
| `info registers` | Tum core register'lar |
| `p/x $sp` | Tek register, hex |
| `x/16wx $sp` | Bellekten 16 word, hex |
| `disassemble /s` | Kaynak ile karisik disassembly |
| `watch my_var` | Degisken degisince dur |
| `rwatch` / `awatch` | Okuma / her erisim |

Cortex-M'e ozel repo komutlari (`scripts/gdb/cortex-m.gdb`):

| Komut | Kullanim |
|---|---|
| `cm-connect` | `target remote :3333` |
| `cm-regs` | R0-R12, SP/LR/PC/xPSR + MSP/PSP/CONTROL/IPSR |
| `cm-mode` | Mod, ayricalik, aktif stack |
| `cm-stack` | Aktif SP'den 16 word HAM dokum (frame cozumu DEGIL) |
| `cm-exc-return <deger>` | Bir EXC_RETURN degerini cozer |
| `cm-frame <adres> <EXC_RETURN>` | Standart 8 word frame'i cozer (asagiya bak) |
| `cm-fault` | CFSR/HFSR/MMFAR/BFAR + gecerlilik bitleri + set olan CFSR bitlerinin isimleri (`SET` satiri) |
| `cm-vectors` | VTOR ve vector table girdileri |
| `cm-mpu` | MPU_TYPE / MPU_CTRL |

Onemli on kosul: baglanmadan once

```text
set architecture armv8-m.main
```

Bu olmadan `$msp`, `$psp`, `$control`, `$psplim` gorunmez.
(`set arm force-mode thumb` yalnizca disassembly modunu etkiler; mimari
secimi DEGILDIR.)

## Teshis akislari

### "Program neden buraya geldi?"

```text
bt                 # cagri yigini
info frame         # mevcut frame
p/x $lr            # nereye donecek
```

`bt` anlamsiz gorunuyorsa stack bozulmus olabilir - asagiya bak.

### "Stack bozuldu mu?"

```text
cm-regs                     # SP mantikli bir araliktata mi?
p/x &__stack_limit
p/x &__stack_top
```

SP `__stack_limit`'in altina indiyse **stack overflow** var.
SP bu araligin tamamen disindaysa yanlis stack pointer secilmis
olabilir (CONTROL.SPSEL).

```text
x/32wx $sp                  # stack icerigi mantikli mi
```

Donus adresleri `0x1000xxxx` (FLASH) araliginda gorunmeli.
Rastgele degerler gormek bozulma isaretidir.

### "PC hangi komutta?"

```text
x/i $pc                     # su anki komut
x/10i $pc-20                # cevresi
info line *$pc              # hangi C satiri
```

### "Bu fault neden olustu?"

Sirali akis:

```text
cm-mode                       # 1) Handler'da miyiz? Hangi exception?
cm-fault                      # 2) CFSR/HFSR ne diyor? Stacking hatasi var mi?
cm-exc-return <kaydedilen LR> # 3) Frame hangi stack'te, hangi turde?
cm-frame <adres> <EXC_RETURN> # 4) Frame'i coz
x/i <stacked PC>              # 5) O komuta bak
```

### `cm-frame` nasil kullanilir

```text
cm-frame <frame_adresi> <EXC_RETURN>
```

Iki argumanin da SENIN tarafindan verilmesi bilincli bir tasarim karari:

**Frame adresi.** Handler MSP uzerinde calisir, ama kesilen baglam frame'i
PSP'ye yigmis olabilir. Ustelik handler'in kendi prolog'u MSP'yi frame'in
basindan uzaklastirir. Yani **aktif `$sp` genel olarak frame adresi
degildir** ve komut ondan tahmin etmez.

**EXC_RETURN.** Frame TURUNU (standart / FP-genisletilmis, hangi stack,
hangi guvenlik durumu) yalnizca o soyler. Degeri handler'a **girer girmez**
`$lr`'den al; handler icinde bir C cagrisi yapildiktan sonra canli `$lr`
artik EXC_RETURN olmayabilir.

Tipik akis:

```text
(gdb) b MyFault_Handler
(gdb) continue
(gdb) set $er = $lr              # handler girisinde: EXC_RETURN
(gdb) cm-exc-return $er          # bit2 -> hangi stack?
(gdb) cm-frame $psp $er          # bit2 = 1 ise
(gdb) cm-frame $msp $er          # bit2 = 0 ise (handler henuz stack kullanmadiysa)
```

### `cm-frame` sınırları

Bu komut desteklenmeyen frame türlerini ve geçersiz değerleri eler.
Kontrollerin geçmesi, verilen adreste donanımın daha önce bir exception
frame yazdığını doğrulamaz; yalnızca adres bilgisi bunun için yeterli değildir.

Reddettigi durumlar:

Red mesajlari kategorilidir; hepsi tek bir genel hataya indirgenmez:

| Etiket | Anlami |
|---|---|
| `[BICIMSIZ EXC_RETURN]` | Deger mimari kodlamaya uymuyor |
| `[DESTEKLENMIYOR]` | Mimari olarak gecerli, ama bu komut o bicimi cozmuyor |
| `[STACKING HATASI]` | Donanim frame'i yigamamis olabilir |
| `[GECERSIZ FRAME ADRESI]` | Hizasiz veya RAM penceresi disinda |
| `[GECERSIZ FRAME ICERIGI]` | Okunan degerler frame olamaz |

| Kontrol | Kategori | Red sebebi |
|---|---|---|
| EXC_RETURN[31:7] | BICIMSIZ | RES1'dir, hepsi 1 olmali. **Yalnizca `0xFFFFFF` onekine bakmak YETMEZ** |
| EXC_RETURN[1] | BICIMSIZ | Ayrilmistir, sifir olmak zorunda (`0xFFFFFFFF` bu yuzden gecersiz) |
| `Mode = 0` + `SPSEL = 1` | BICIMSIZ | Handler Mode Main stack kullanir; boyle bir kodlama yok (`0xFFFFFFF5`) |
| `FType = 0` | DESTEKLENMIYOR | FP-genisletilmis frame (FPU ders 13) |
| `DCRS = 0` | DESTEKLENMIYOR | Ek durum baglami (callee register'lari yigilmis) |
| `ES != S` | DESTEKLENMIYOR | Guvenlik durumlari arasi gecis (ders 16) |
| CFSR stacking biti | STACKING HATASI | STKOF / STKERR / MSTKERR |
| Adres hizasi | GECERSIZ FRAME ADRESI | 4'un kati degil |
| Adres araligi | GECERSIZ FRAME ADRESI | Platform RAM penceresi disinda |
| Yigilmis xPSR | GECERSIZ FRAME ICERIGI | T biti (bit24) sifir - Cortex-M'de imkansiz |
| Yigilmis PC | GECERSIZ FRAME ICERIGI | 0, tek sayi, veya kod penceresi disinda |

Pratikte desteklenen EXC_RETURN degerleri yalnizca sunlardir:

`cm-frame`, mimari olarak gecerli EXC_RETURN bicimlerinin **bilincli olarak
secilmis bir ALT KUMESINI** destekler: standart frame (`FType=1`), varsayilan
callee yigmasi (`DCRS=1`) ve tek guvenlik durumu (`ES == S`). Geriye alti
deger kalir:

| Deger | ES | S | Mode | SPSEL | Anlami |
|---|---|---|---|---|---|
| `0xFFFFFFF1` | 1 | 1 | 0 | 0 | Secure, Handler Mode'a donus, frame MSP'de |
| `0xFFFFFFF9` | 1 | 1 | 1 | 0 | Secure, Thread Mode'a donus, frame MSP'de |
| `0xFFFFFFFD` | 1 | 1 | 1 | 1 | Secure, Thread Mode'a donus, frame PSP'de |
| `0xFFFFFFB0` | 0 | 0 | 0 | 0 | Non-secure, Handler Mode'a donus, frame MSP'de |
| `0xFFFFFFB8` | 0 | 0 | 1 | 0 | Non-secure, Thread Mode'a donus, frame MSP'de |
| `0xFFFFFFBC` | 0 | 0 | 1 | 1 | Non-secure, Thread Mode'a donus, frame PSP'de |

Alti degerin hepsinde `[31:7]` RES1 (hepsi bir), `[1] = 0`, `FType = 1`,
`DCRS = 1`, ve `Mode = 0` olan satirlarda `SPSEL = 0`.

**Sik yapilan hata:** `0xFFFFFFB1` / `0xFFFFFFB9` / `0xFFFFFFBD`
desteklenmez. Bu degerlerde `ES = 1` ama `S = 0`'dir — yani exception
Secure durumdan alinmis ama frame Non-secure stack'tedir. Bu mimari olarak
gecerli bir guvenlik durumu gecisidir; `cm-frame` onu
`[DESTEKLENMIYOR]` olarak reddeder (ders 16).

Kaynak: Armv8-M Exception Model User Guide (Arm 107706), EXC_RETURN bit
alanlari tablosu — `[31:7] RES1`, `[6] S`, `[5] DCRS`, `[4] FType`,
`[3] Mode`, `[2] SPSEL`, `[1]` ayrilmis, `[0] ES`.

Platform pencereleri `$cm_ram_start` / `$cm_ram_end` ve `$cm_code_start` /
`$cm_code_end` degiskenlerinden ayarlanabilir.

CFSR okuma rehberi:

| Alan | Bit araligi | Anlami |
|---|---|---|
| MMFSR | CFSR[7:0] | MemManage - genelde MPU ihlali |
| BFSR | CFSR[15:8] | BusFault - gecersiz bellek erisimi |
| UFSR | CFSR[31:16] | UsageFault - gecersiz komut/durum |

Sik gorulen bitler:

| Bit | Isim | Anlami |
|---|---|---|
| CFSR[1] | MMFSR.DACCVIOL | Veri erisimi MPU'ya takildi |
| CFSR[7] | MMFSR.MMARVALID | MMFAR gecerli adres iceriyor |
| CFSR[9] | BFSR.PRECISERR | Hatanin adresi kesin |
| CFSR[12] | BFSR.STKERR | **Exception stacking sirasinda hata** |
| CFSR[15] | BFSR.BFARVALID | BFAR gecerli adres iceriyor |
| CFSR[16] | UFSR.UNDEFINSTR | Tanimsiz komut |
| CFSR[17] | UFSR.INVSTATE | Gecersiz state (cogunlukla Thumb biti 0) |
| HFSR[30] | FORCED | Baska bir fault **escalate** oldu |

`HFSR.FORCED = 1` gordugunde: "asil hata HardFault degil". Gercek sebep
CFSR'dedir; ilgili fault handler kapali oldugu icin HardFault'a yukselmistir.

### Ozel durum: stacking'in kendisi basarisizsa

`BFSR.STKERR`, `UFSR.STKOF` veya `MMFSR.MSTKERR` set ise donanim exception
frame'ini **yigamamis** olabilir; bellekte guvenilir bir frame yoktur.
`cm-frame` bu bitleri gördüğünde güvenilir olmayan belleği register
değerleri olarak yorumlamamak için çözümlemeyi **reddeder**.

Bu repoda iki ornek var:

| Ders | Bit | Senaryo |
|---|---|---|
| 02 "Break It" | `BFSR.STKERR` | PSP bilerek gecersiz, sonra SPSEL = 1 |
| 04 "Break It" | `UFSR.STKOF` | `PSPLIM` + derin ozyineleme |

Dikkat: bu bitler **yapiskandir (sticky)**. Eski bir hatadan kalmis
olabilirler; emin degilsen 1 yazarak temizleyip tekrar dene.

### "Degisken neden degisiyor?"

```text
watch counter
continue
bt                 # degistiren kod kim
```

`-O2` ile derlenmis kodda degisken register'a alinmis olabilir ve
watchpoint calismayabilir. Once `-O0`/`Debug` preset ile dene.

### "Optimizasyon degiskenimi yedi"

```text
<optimized out>
```

Derleyici değişkeni optimizasyon sırasında kaldırdığı için GDB onu gösteremiyor.
`Debug` preset (`-Og -g3`) kullan veya degiskeni `volatile` yap.

## Reset'ten itibaren adim adim gitmek

```text
cm-connect
cm-vectors                 # vector table'da ne var
p/x $pc                    # Reset_Handler'da miyiz
si                         # tek tek komut
break main
c
```

Startup'i bu sekilde izlemek ders 11'in ana egzersizidir.

## QEMU tarafindaki ipuclari

```bash
# Gecersiz bellek erisimlerini logla
qemu-system-arm -M mps2-an505 -kernel fw.elf -nographic \
                -d guest_errors,unimp

# Icra edilen komutlari logla (cok buyuk dosya)
qemu-system-arm -M mps2-an505 -kernel fw.elf -nographic \
                -d in_asm -D trace.log
```

---

## Fault triage

Sistematik fault teshis listesi ders 08'de:
[`lessons/08-faults/README.md`](../lessons/08-faults/README.tr.md) → "Cortex-M Fault Triage".
Kisa hali: IPSR → HFSR.FORCED → CFSR alt alani → sebep bitleri → VALID bitleri →
yigma hatasi → EXC_RETURN → frame → disassembly.
