**Dil:** [English](quick-check.md) | Türkçe

# Lesson 09 — Quick Check Cevapları

> Önce kendi cevabını yaz, sonra aç.

---

### 1. `r0`–`r3` ne için kullanılır, neden korunmayabilir?

**ABI:** İlk argümanları taşırlar (a1–a4), sonuç `r0` (64 bit ise `r0:r1`)
ile döner, ve çağrılar arasında scratch olarak kullanılabilirler. AAPCS32 onları
**caller-clobbered** sayar: çağrılan fonksiyon onları serbestçe bozabilir. Bir
değer çağrıdan sonra lazımsa, çağıran onu başka bir yerde (callee-saved bir
register'da veya stack'te) tutmak zorundadır.

Ayrım önemli: **caller-clobbered**, callee'nin onları değiştirmeye
**yetkili** olduğu ve çağıranın onlara **güvenemeyeceği** anlamına gelir —
her callee'nin onları gerçekten değiştirdiği anlamına gelmez.

Ölçüm (GDB durak C ve D, üç preset): `helper_scale` **yalnızca `r0`'ı**
değiştirdi; `r1`–`r3` `BL`'den önce ve sonra aynıydı. Bu bir gözlemdir; ona
güvenmek yine de ABI ihlali olur.

---

### 2. Caller-saved ve callee-saved: kim neyden sorumlu?

| | Caller | Callee |
|---|---|---|
| **caller-clobbered** (r0–r3, r12) | Lazımsa kendisi korur | Serbestçe kullanır |
| **callee-saved** (r4–r8, r10, r11; v6 ise r9) | Hiçbir şey yapmaz | Kullanırsa saklar ve geri yükler |

Sebep bir iş bölümü: kısa ömürlü değerler için çağıran hiçbir şey saklamak
zorunda kalmaz; uzun ömürlü değerler için callee yalnızca **gerçekten
kullandığı** register'ları saklar.

---

### 3. (Reasoning) Callee sonucu doğru olsa bile neden r4'ü geri yüklemek zorunda?

Çünkü r4'ün **sahibi çağırandır**. Çağıran, AAPCS'e güvenerek bir değeri r4'te
çağrı boyunca tutuyor olabilir. Callee'nin sonucu bu sözleşmenin yalnızca bir
parçasıdır; r4'ü bozmak, çağıranın **başka bir değişkenini** bozar ve bu hata
çağrının kendisinde değil, **daha sonra** ortaya çıkar.

Break It bunu ölçtü: `bad_callee` doğru sonuç verdi (`CHECK_RESULT PASS`), harness
r4'ün bozulduğunu buldu (`CHECK_CALLEE_SAVED FAIL`, `CORRUPTED r4`).

Doğrudan C'den çağrılsaydı etkinin görünür olup olmaması, çağıranın o anda r4'ü
kullanıp kullanmadığına — yani register tahsisine — bağlı olurdu. Doğru sonuç,
doğru sözleşme değildir.

---

### 4. `r12` (IP) ne için kullanılabilir?

**ABI:** Intra-procedure-call scratch register'ı; caller-clobbered. AAPCS32
*Use of IP by the linker*: linker'ın eklediği bir **veneer** (örneğin menzil
dışı bir dallanma için ara kod parçası) r12'yi bozabilir; uyumlu bir program
bunun herhangi bir uygun dallanmada olabileceğini varsaymalıdır.

Bu derste r12 iki yerde kullanıldı: probe'lar onu `g_probe` işaretçisi olarak
kullandı (argüman taşımadığı için), harness onu `blx` hedefi olarak kullandı.
Veneer'lar ders 10'da.

---

### 5. Başka bir fonksiyonu çağıran fonksiyonda LR'ye ne olur?

`BL`, dönüş adresini LR'ye yazar — yani iç çağrı, fonksiyonun **kendi**
dönüş adresini ezer. Bu yüzden yaprak olmayan bir fonksiyon **iç çağrıdan
önce** LR'yi saklamalıdır.

`abi_chain`'de: `push {r4, lr}` … `bl helper_scale` … `pop {r4, pc}`. GDB durak
C'de stack'te `[SP+4] = 0x100004eb` — saklanan LR. `pop {…, pc}` bu değeri
doğrudan PC'ye yükleyerek döner.

İstisna: tail call (soru 10) — orada LR hiç saklanmaz, çünkü fonksiyon kendi
dönüşünü çağrılan fonksiyona bırakır.

---

### 6. (Reasoning) "Public interface'te SP mod 8 = 0" ne demek?

**ABI:** AAPCS32, dışarıdan çağrılabilen bir fonksiyon sınırında (bir `BL`
anında ve fonksiyon girişinde) SP'nin 8'in katı olmasını ister. Her zaman
geçerli kural ise yalnızca SP mod 4 = 0.

Üç ayrı kural:

| Kural | Katman |
|---|---|
| Çağrı sınırında SP mod 8 = 0 | **ABI** |
| `LDRD`/`STRD` gibi erişimlerin hizalama gereksinimi | **ARCHITECTURE** |
| Exception girişinde frame hizalaması (xPSR bit 9) | **ARCHITECTURE** |

Hizasız bir SP **tek başına** bir `LDRD` fault'u üretmez; bozulan şey ABI
varsayımıdır. Ve kural çağrı sınırı içindir: `asm_clobber_good` `-Og`'da
yalnızca `push {r4}` yapıyor (4 byte), yani içinde SP mod 8 = 4 — yasal, çünkü
hiçbir şey çağırmıyor.

Bu derste ölçülen çağrı sınırları: `abi_chain` `BL` anında `0x30007FD8`,
harness `BLX` anında `0x30007FB8` — ikisi de mod 8 = 0. Exercise başlangıcı
`0x30007FAC` — mod 8 = 4 → `CHECK_SP_ALIGNED_AT_CALL FAIL`.

---

### 7. (Reasoning) `f(u32 a, u32 b, u32 c, u64 d)` için `d` nereye gider?

**ABI (AAPCS32 kuralları):**

1. `a`, `b`, `c` → r0, r1, r2. NCRN = r3.
2. `d` 8 byte hizalama ister → **C.3**: NCRN bir sonraki çift register'a,
   r4'e yuvarlanır.
3. r4 argüman register'ı değil → `d` register'lara sığmaz (C.4 başarısız,
   C.5 NCRN < r4 koşulu sağlanmaz).
4. **C.6–C.8:** NCRN = r4, NSAA 8 byte'a yuvarlanır, `d` stack'e gider.

**OBSERVATION (üç preset):** `[SP+0] = 0xDDEEFF00` (düşük word),
`[SP+4] = 0x99AABBCC` (yüksek word), SP mod 8 = 0.

**r3 hakkında:** argüman taşımıyor; içeriği **tanımsız**. Ölçüm bunu açıkça
gösterdi: `debug` ve `minsize`'da r3 tesadüfen `0x99AABBCC` (d'nin yüksek
word'ü — GCC onu stack'e yazarken geçici olarak r3'te tutmuş), `release`'de
`0x00000000`. Bu yüzden ders r3'e bakarak **hiçbir karar vermiyor**; kontrol
yalnızca `d`'nin doğru yerde olduğunu pozitif olarak doğruluyor.

---

### 8. `volatile` neyi garanti eder?

**Eder:** programın nesneye yaptığı her erişimin korunmasını ve nesneye bütün
olarak erişilmesini (AAPCS32 *Volatile Data Types*). Derleyici değeri bir
register'da önbellekleyip tekrar okumaktan kaçınamaz.

Ölçüm: `vol_store_twice` üç preset'te de **iki** `str`; `dead_store` (volatile
değil) `-O2`/`-Os`'ta **tek** `str`. `poll_vol` her iterasyonda yeniden okuyor;
`poll_plain` `-O2`'de bir kez okuyup sonsuz döngüye giriyor.

**Etmez:**

- **Atomiklik** — `g_vol++` hâlâ ayrı bir load ve store'dur.
- **Karşılıklı dışlama** — iki bağlam aynı anda yazabilir.
- **Volatile olmayan erişimlerin sıralanması** — bir bellek bariyeri
  (`DMB`/`DSB`) değildir.
- Thread güvenliği, cache tutarlılığı, bellek koruması.

AAPCS32 ayrıca, **adresi hiç alınmayan** otomatik bir değişkenin `volatile`
niteliğinin (fonksiyon `setjmp()` çağırmıyorsa) derleyici tarafından yok
sayılabileceğini söyler. Bu bir izindir; bu derste ölçülmedi ve fikstürler ona
dayanmıyor.

---

### 9. Yaprak fonksiyon nedir? Stack kullanabilir mi?

**Yaprak fonksiyon = hiçbir fonksiyon çağırmayan fonksiyon.** Stack kullanıp
kullanmaması ayrı bir sorudur.

`leaf_with_stack` hiçbir şey çağırmıyor — üç preset'te de `bl` yok — ama stack
ayırıyor:

```text
-Og:  sub sp, #16 ... add sp, #16 ; bx lr
-O2:  push {r4} ; sub sp, #20 ... add sp, #20 ; pop {r4} ; bx lr
-Os:  sub sp, #16 ... add sp, #16 ; bx lr
```

LR saklanmıyor, çünkü onu ezecek bir `BL` yok. Dizi bellekte, çünkü adresi boş
bir inline asm'e veriliyor (`"r"(buf)` + `"memory"`) — bir çağrı değil.

---

### 10. (Reasoning) `b helper` ile biten bir fonksiyon — ABI bozuk mu?

**Hayır, olmak zorunda değil.** Bu bir **tail call** olabilir: fonksiyonun son
işi başka bir fonksiyonu çağırıp sonucunu aynen döndürmekse, derleyici
`bl` + dönüş yerine doğrudan `b` üretebilir. LR'ye dokunulmaz; çağrılan
fonksiyon doğrudan **orijinal çağırana** döner.

Ölçüm: `tail_call` `-O2`/`-Os`'ta `adds r0, #1 ; b.w helper_ext` — LR
saklanmıyor, push yok. `-Og`'da ise `push {r3, lr} ; bl helper_ext ; pop {r3, pc}`.

Bu bir **COMPILER** optimizasyonudur (`-foptimize-sibling-calls`), ABI kuralı
değil ve garanti de değil. Ama sözleşmeyi bozmaz: dönüş değeri r0'da doğru
yere varır, callee-saved'ler korunur.

---

### 11. (Reasoning) `-O2` bir değeri çağrı boyunca r3'te tuttu — ABI kuralı mı?

**Hayır.** r3 caller-clobbered; AAPCS çağıranın onun korunmasına güvenmesine
**izin vermez**.

Bunu mümkün kılan bir **derleyici bilgisidir**: `-fipa-ra` (`-O2`/`-Os`'ta açık)
ile GCC, aynı derleme birimindeki bir callee'nin gerçekte hangi register'ları
bozduğunu bilir ve bozmadıklarını çağrı boyunca kullanabilir. Bu, callee'nin
**içini görmeye** dayanan bir optimizasyondur.

Ayrı derlenmiş bir fonksiyon veya el yazımı assembly için bu bilgi yoktur —
bu yüzden `helper_scale` bilerek ayrı bir TU'da ve harness bir C çağırana
güvenmiyor.

---

### 12. `(x + 1) > x` neden `int` için `return 1`'e katlanıyor?

İşaretli tamsayı taşması C'de **tanımsız davranıştır** (ISO C11 §6.5 ¶5).
Derleyici taşmanın olmayacağını varsayabilir; o zaman `x + 1 > x` her zaman
doğrudur.

İşaretsiz aritmetik ise modüler olarak **tanımlıdır**: `x = UINT_MAX` için
`x + 1 = 0`, sonuç yanlış. Derleyici katlayamaz.

Ölçüm:

```text
signed_gt:    movs r0, #1 ; bx lr                 (-Og, -O2, -Os)
unsigned_gt:  adds r0, #1 ; it ne ; movne r0, #1   (-Og, -O2, -Os)
```

Katlama `-Og`'da bile gerçekleşti. Ders 03'ün düzeltmesiyle aynı ilke:
assembly'deki `ADDS`'ın sarması, taşan işaretli C'nin davranışı **değildir**.
`signed_gt` bu yüzden asla çalıştırılmıyor; yalnızca disassembly'de inceleniyor.

---

### 13. (Reasoning) r4'e yazan inline asm neden clobber bildirmeli?

Inline asm'in **iki sözleşmesi** vardır:

1. **Komutun anlamı** (işlemci): `mov r4, %0` r4'e yazar. Bu her iki durumda
   da aynı ve doğrudur.
2. **Kısıt/clobber sözleşmesi** (derleyici): GCC asm metnini **okumaz**. Hangi
   register'ların değiştiğini yalnızca clobber listesinden öğrenir.

Ölçüm (`asm_clobber_bad` üç preset'te, `asm_clobber_good` `-Og`/`-O2`'de):

```text
asm_clobber_bad:   mov r4, r0 ; bx lr                        <- r4 saklanmadi
asm_clobber_good:  push {r4} ; mov r4, r0 ; pop {r4} ; bx lr <- saklandi
```

`-Os`'ta `asm_clobber_good`, `push {r4, lr}` … `pop {r4, pc}` kullanır;
orada da r4 korunur (README, Deney 1).

Clobber'sız sürüm, çağıranına karşı bir **AAPCS ihlali** — `bad_callee` ile
aynı hata, bu kez derleyicinin elinden. Görünür olup olmaması yine register
tahsisine bağlıdır. Bu yüzden `asm_clobber_bad` asla çalıştırılmıyor.

---

## Referanslar

- AAPCS32 (release 2025Q4) — *Core registers*, *Stack constraints at a public
  interface*, *Use of IP by the linker*, *Result Return*, *Parameter Passing*
  (C.3–C.8), *Volatile Data Types*
- ISO C11 §6.5 ¶5, §6.7.3
- GCC manual — *Options That Control Optimization*, *Extended Asm*

---

[← Ders 09](../README.tr.md) · [Ders 10 →](../../10-linker-elf-memory/README.tr.md)
