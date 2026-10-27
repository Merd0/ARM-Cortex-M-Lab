**Dil:** [English](README.md) | Türkçe

# Solution 01 — Repair an ABI-Broken Assembly Function

Exercise: [`../../exercises/01-callee-save/`](../../exercises/01-callee-save/README.tr.md)

Kod: [`scale_and_sum.S`](scale_and_sum.S)

## Gerçek çıktı

`debug`:

```text
[  CHECK_HARNESS_POSITIVE_CONTROL] PASS
[  RESULT] 36
[  MASK] 0x00000000
[  CORRUPTED] (yok)
[  SP_AT_BL] 0x30007FA8
[  CHECK_RESULT] PASS
[  CHECK_CALLEE_SAVED] PASS
[  CHECK_SP_ALIGNED_AT_CALL] PASS
===== END =====
```

`release` ve `minsize`'da `SP_AT_BL = 0x30007F98`; kararlar aynı.

## Başlangıç ile çözüm yan yana

```asm
/* BASLANGIC */                         /* COZUM */
push  {r4, lr}                          push  {r3, r4, r5, lr}
mov   r4, r0                            mov   r4, r0
mov   r5, r1                            mov   r5, r1
sub   sp, sp, #4
<SP kaydi>                              <SP kaydi>
mov   r0, r4                            mov   r0, r4
bl    helper_scale                      bl    helper_scale
add   sp, sp, #4
add   r0, r0, r5                        add   r0, r0, r5
pop   {r4, pc}                          pop   {r3, r4, r5, pc}
```

## Onarım 1 — r5 korunmalı

`b` çağrı boyunca **r5**'te tutuluyor. r5 **callee-saved**: sahibi
`scale_and_sum`'ı çağıran. Kullanıyorsak girişte saklamak ve çıkışta geri
yüklemek zorundayız.

Başlangıç yalnızca `{r4, lr}` saklıyordu; harness r5'in bozulduğunu buldu
(`MASK 0x00000002`, `CORRUPTED r5`).

Neden `b`'yi r1'de bırakmadık? Çünkü r1 **caller-clobbered**: `helper_scale`
onu serbestçe bozabilir. Çağrı boyunca yaşaması gereken bir değer ya
callee-saved bir register'da ya da stack'te durmalı.

## Onarım 2 — `BL` anında SP 8'e hizalı olmalı

Girişte SP 8'e hizalı (harness'in `BLX` anında ölçülüyor). Başlangıçta:

```text
push {r4, lr}   8 byte   -> hizali
sub  sp, #4     4 byte   -> BL aninda SP mod 8 = 4   (SP_AT_BL = 0x30007FAC)
```

Çözüm iki şey yapıyor:

- Gereksiz `sub sp, #4` / `add sp, #4` çifti kaldırıldı — bu fonksiyonun yerel
  bir çalışma alanına ihtiyacı yok.
- Korunacak register'lar r4, r5 ve LR — **üç** register, 12 byte. 8'in katı
  değil. Bu yüzden listeye bir **dolgu** register'ı eklendi: r3. Toplam dört
  register, 16 byte.

r3 neden? Caller-clobbered, yani saklanıp geri yüklenmesi bir şey bozmaz; ve
`push`/`pop` listesinde yalnızca yer tutuyor. GCC de aynı kalıbı kullanıyor:
`-Og`'da `tail_call` için `push {r3, lr}` üretti. Harness de öyle:
`push {r3-r11, lr}`.

Alternatif: `push {r4, r5, r6, lr}` de 16 byte'tır ve doğrudur; bu durumda r6
gereksiz yere saklanır ama zararı yoktur.

## LR ve dönüş

- **LR neden saklanıyor?** `scale_and_sum` yaprak değil: `bl helper_scale`
  LR'ye `scale_and_sum` içindeki, `BL`'den sonraki komutun adresini (Thumb
  bit 0 = 1 ile) yazar. Çağıranın dönüş adresi saklanmasaydı, fonksiyon
  çağırana dönmek yerine kendi içindeki bu noktaya "dönmeye" çalışırdı.
- **Neden `pop {…, pc}`?** Saklanan LR değeri doğrudan PC'ye yüklenir: geri
  yükleme ve dönüş tek komutta. `pop {r3, r4, r5, lr}` + `bx lr` de doğrudur;
  yalnızca bir komut daha uzundur. Her iki durumda da LR'nin bit 0'ı (Thumb)
  korunur.

## "Doğru sonuç" neden hiçbir zaman kanıt değildi

Başlangıç `RESULT 36` verdi — doğru. Ama:

- r5 bozukluğu, `scale_and_sum`'ı çağıran fonksiyonun r5'te tuttuğu bir değeri
  **daha sonra** kaybetmesine yol açardı.
- Hizasız SP, `helper_scale`'in (veya onun çağırdığı bir şeyin) 8 byte hizalı
  stack varsayımıyla ürettiği kodu yanlış bir temel üzerinde çalıştırırdı.

İkisi de `scale_and_sum`'ın kendi sonucunda görünmez. Sözleşme, sonuçtan ayrı
denetlenmek zorunda — harness'in varlık sebebi bu.

---

[← Ders 09](../../README.tr.md)
