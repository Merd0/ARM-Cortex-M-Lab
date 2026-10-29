**Dil:** [English](README.md) | Türkçe

# Solution 01 — Activate PSP Correctly

Exercise: [`../../exercises/01-activate-psp/`](../../exercises/01-activate-psp/README.tr.md)

> Bu dosya, bağımsız denetim sonrası **yeniden yazıldı**. Önceki sürüm artık
> var olmayan bir çözümü anlatıyordu (sıradan C içinde `activate_psp()` ile
> stack değiştirme) ve iki teknik hata içeriyordu. İkisi de aşağıda düzeltildi.

---

## Çözüm nerede

Asıl çözüm **C'de değil, assembly'de**:

```text
solutions/01-activate-psp/psp_start.S
```

```asm
psp_start:                      @ r0 = psp_top, r1 = entry  — DONMEZ
    msr     psp, r0             /* 1) PSP = yeni stack tepesi */

    mrs     r2, control
    orr     r2, r2, #2          /* 2) SPSEL = 1               */
    msr     control, r2
    isb                         /* 3) senkronize et           */

    ldr     lr, =psp_entry_trap /* entry donerse tuzak        */
    orr     lr, lr, #1
    bx      r1                  /* taze C baglamina dallan    */
```

`main.c` tarafı değişmedi: stack bloğunu ve guard'ları kurar, sonra
`psp_start(TASK_STACK_LIMIT, thread_entry)` çağırır ve **geri dönmez**.

Akış:

```text
MSP uzerinde C (main)
        |
        v  psp_start()  — derleyicinin frame varsayimi olmayan bir sinir
   PSP secilir (msr psp + CONTROL.SPSEL + isb)
        |
        v  bx r1
TAZE C girisi (thread_entry) — kendi prolog'unu YENI SP uzerinde kurar
        |
        v
MSP'de birakilan frame'e BIR DAHA DONULMEZ
```

---

## Neden assembly sınırı zorunlu?

Derleyici, bir C fonksiyonu için MSP üzerinde bir frame ayırır ve yerel
değişkenlere `[sp, #offset]` ile erişen komutlar üretir. `CONTROL.SPSEL`
değiştiğinde `SP` başka bir bölgeyi gösterir — **aynı offset'ler artık başka
bir adrese denk gelir.**

`CONTROL` yazmak:

- mevcut frame'i yeni stack'e **taşımaz**,
- yeni stack'te bir frame **oluşturmaz**,
- derleyiciye bir şey **bildirmez**.

Bu teorik bir kaygı değil, ölçülmüş bir hata: bu dersin ilk sürümü geçişi
sıradan C içinde yapıyordu ve bağımsız denetim `-O2`/`-Os` altında
`task_stack`'in **bir üstüne** yazıldığını gösterdi (`CHECK_REGION FAIL`).

Neyin **çözmediğini** de bilmek gerekir:

| Denenen | Neden yetmez |
|---|---|
| `volatile` | Erişimin silinmesini engeller; taban register'ı değiştirmez — offset'ler yine `sp`'ye göre |
| `noinline` | Fonksiyonun ayrı kalması, frame'inin MSP'de olduğu gerçeğini değiştirmez |
| Fazladan `ISB` | Senkronizasyon sorununu çözer; buradaki sorun **frame geçerliliği** |
| Frame'i kopyalamak | Pointer'lar ve saklanmış adresler eski konuma işaret etmeye devam eder |

Tek doğru yol: geçişi `SP`'ye bağlı yerel durumu olmayan bir kodda yapmak ve
**sıfırdan** bir C fonksiyonuna dallanmak.

`psp_start` bu koşulu sağlıyor — disassembly'sinde tek bir `[sp, #offset]`
erişimi yok:

```text
10000054 <psp_start>:
10000054:  f380 8809   msr   PSP, r0
10000058:  f3ef 8214   mrs   r2, CONTROL
1000005c:  f042 0202   orr.w r2, r2, #2
10000060:  f382 8814   msr   CONTROL, r2
10000064:  f3bf 8f6f   isb   sy
10000068:  f8df e008   ldr.w lr, [pc, #8]
1000006c:  f04e 0e01   orr.w lr, lr, #1
10000070:  4708        bx    r1
```

Hedef ise kendi prolog'unu yeni `SP` üzerinde kuruyor:

```text
10000170 <thread_entry>:
10000170:  b500   push  {lr}
10000172:  b083   sub   sp, #12
```

`push {lr}` (4 byte) + `sub sp, #12` = 16 byte; çağrı sınırında 8 byte
hizalama korunuyor.

---

## Üç adım, üç sebep

### 1. `msr psp, r0` — neden `&block[128]`?

Stack full-descending: `PUSH` `SP`'yi azaltır ve **yeni** `SP` adresine yazar.
Yani verdiğin adresin kendisine hiç yazılmaz.

`&task_stack_block[128]` kullanılabilir alanın bir ötesidir; bu adrese
erişilmez, yalnızca başlangıç değeri olarak durur. Sonuç: 128 word'ün tamamı
kullanılabilir ve sınır aşılmaz.

- `&block[127]` olsaydı: taşma olmazdı ama iki şey kaybederdin — kapasite
  (son word hiç kullanılmaz) ve **hizalama** (`&block[128]` 8 byte hizalıysa
  `&block[127]` değildir; çağrı sınırında `SP % 8 == 4` olur, AAPCS ihlali).
- `&block[0]` olsaydı: ilk `PUSH` dizinin **altına** yazar, komşu değişkenleri
  ezerdi.

### 2. `CONTROL` — neden oku-değiştir-yaz?

Doğrudan `2` yazmak bit 0'ı (`nPRIV`) sessizce sıfırlardı. Bu derste `nPRIV`
zaten 0 olduğu için fark etmezdi, ama ders 05'te unprivileged Thread Mode'a
geçtiğimizde aynı satır zararlı olurdu. Ayrıca `CONTROL`'ün yalnızca bu iki
alanı yoktur — mimari/FP/güvenlik durumuna göre başka alanlar da bulunur
(`FPCA`, `SFPA`). Genel kural: **dokunmadığın bitleri koru.**

> **Düzeltme.** Bu dosyanın önceki sürümü, ayrıcalıksız kodun `CONTROL`'e
> `2` yazarak kendini yeniden ayrıcalıklı yapabileceğini ima ediyordu.
> Bu **yanlıştır**. Unprivileged Thread Mode kendi `nPRIV` bitini temizleyerek
> ayrıcalık kazanamaz; yetki yükseltme yalnızca **ayrıcalıklı bir exception
> yolundan** (örneğin SVC handler'ı) geçer. Konu ders 05'te.

### 3. `isb` — senkronizasyon kaynağı tam olarak nerede?

Mimari ilke: `CONTROL` yazımı yürütme bağlamını değiştirir; değişikliğin
sonraki komutlarca görülmesinin garanti olması için bir komut senkronizasyon
bariyeri gerekir.

Ama **kimin çalıştırdığı** önemli:

| Yol | `ISB`'yi kim çalıştırır |
|---|---|
| Burada: `msr control, r2` elle yazıldı | **Biz** — `isb` satırı bize ait |
| C'den `__set_CONTROL()` (CMSIS) | **Sarmalayıcının kendisi** — içinde `__ISB()` var |
| C'den `__set_CONTROL()` (`core/cm33_teaching.h`) | **Sarmalayıcının kendisi** — o da `__ISB()` yapar |

> **Düzeltme.** Önceki sürüm "`__ISB()`'yi kaldırırsan senkronizasyon kalkar"
> anlamına gelen bir deney öneriyordu. Bu yanlıştı: `__set_CONTROL()` zaten
> içinde `ISB` çalıştırıyor, dolayısıyla C tarafında fazladan `__ISB()` yazmak
> gereksizdir ve kaldırmak senkronizasyonu ortadan kaldırmaz. Mimari kural
> geçerliliğini koruyor; değişen tek şey, bariyerin nerede olduğunu doğru
> göstermek.

Doğrulamak için kaynağa bak:

```bash
grep -A3 "__set_CONTROL(uint32_t" external/CMSIS/Core/Include/m-profile/cmsis_gcc_m.h
```

---

## Build & Run

```bash
cmake --build --preset debug --target 04-solution-psp
./scripts/run-qemu.sh build/debug/lessons/04-stack-msp-psp/04-solution-psp.elf
```

## Gerçek çıktı

Onarılmış uygulamadan **yeniden ölçüldü** (arm-none-eabi-gcc 13.2.1,
QEMU 8.2.2, mps2-an505, `debug` preset):

```text
===== LESSON 04 SOLUTION 01 =====
[TASK_STACK_BASE] 0x30000000
[TASK_STACK_LIMIT] 0x30000200
[TOP_MOD_8] 0
[MSP] 0x30007FF0
[PSP] 0x300001F0
[CONTROL] 0x00000002
[LOCAL_ADDR] 0x300001F4
[GUARD_VALUE] 0xDEADBEEF
[CHECK_SPSEL] PASS
[CHECK_REGION] PASS
[CHECK_STACK_GUARD] PASS
===== END =====
```

Üç kontrol de `release` ve `minsize` preset'lerinde de `PASS` verir —
eski çözüm bu iki optimizasyon ayarında başarısız oluyordu.

> Adresler build'e özeldir. Kodun herhangi bir yeri değişirse kayarlar.

`CHECK_STACK_GUARD` hakkında bir sınır: guard word'leri yalnızca **izlenen o
iki konumun** üzerine yazılmadığını gösterir. Alt sınırdan taşma olmadığını,
başka bir yere kaçak yazma olmadığını veya `SP`'nin hiç aralık dışına
çıkmadığını **göstermez**. Bu durumlar GDB ölçümleri ve disassembly ile
incelenir (dersin "Observe in GDB" bölümü).

---

[← Ders 04](../../README.tr.md)
