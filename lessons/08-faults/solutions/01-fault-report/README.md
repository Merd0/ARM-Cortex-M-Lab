# Solution 01 — Complete the Fault Report

Exercise: [`../../exercises/01-fault-report/`](../../exercises/01-fault-report/)

Kod: [`decode.c`](decode.c)

## Gerçek çıktı

```text
--- DECISION (decode_fault_snapshot) ---
[  D_ACTIVE_EXCEPTION] MemManage
[  D_ESCALATED] NO
[  D_FAULT_CLASS] MemManage
[  D_PRIMARY_BIT] DACCVIOL
[  D_ADDRESS_VALID] YES
[  D_FAULT_ADDRESS] 0x30000000
[  D_FRAME_SAFE] YES
[  D_STACKED_PC] 0x100000FC
[  CHECK_D_ACTIVE] PASS
[  CHECK_D_NOT_ESCALATED] PASS
[  CHECK_D_CLASS] PASS
[  CHECK_D_BIT] PASS
[  CHECK_D_ADDRESS] PASS
[  CHECK_D_FRAME] PASS
```

## Karar sırası — triage listesiyle aynı

### 1. `active_exception` ← `IPSR`

Şu an hangi handler çalışıyor. Bu **olayın adı değil, onu gören handler'ın
adıdır.**

### 2. `escalated` ← `HFSR.FORCED`

`1` ise aktif handler kök sebep değildir; kanıt bir kat aşağıda.

### 3. `fault_class` ← `CFSR` alt alanları

```c
if      (s->cfsr & SCB_CFSR_MEMFAULTSR_Msk) d->fault_class = "MemManage";
else if (s->cfsr & SCB_CFSR_BUSFAULTSR_Msk) d->fault_class = "BusFault";
else if (s->cfsr & SCB_CFSR_USGFAULTSR_Msk) d->fault_class = "UsageFault";
```

**Aktif handler'dan türetilmiyor.** Escalation vakasında aktif handler
HardFault'tur ama `CFSR` hâlâ `UNDEFINSTR`'i taşır — `fault_class` UsageFault
çıkar. Ders 07 ve ders 08'in ölçtüğü ilkenin kod hali bu.

### 4. `primary_bit` ← alt alan içindeki sebep biti

`MMARVALID` ve `BFARVALID` sebep **değildir**; yalnızca adres register'ının
geçerliliğini söylerler. Bu yüzden önce sebep bitleri kontrol ediliyor
(`DACCVIOL`, `PRECISERR`, `UNDEFINSTR`...).

### 5. `address_valid` / `fault_address` ← `VALID` bitleri

```c
if (s->cfsr & SCB_CFSR_MMARVALID_Msk) {
    d->address_valid = 1;
    d->fault_address = s->mmfar_raw;
}
```

`VALID` biti yoksa adres alanı `0` bırakılıyor — ham içerik **kanıt değildir**.
UsageFault'un adres register'ı hiç yoktur.

### 6. `frame_safe` / `stacked_pc` ← `frame_valid`

`frame_valid`, `fault_record()` içinde şu iki koşuldan hesaplanıyor:

```c
int stacking_failed = MSTKERR || STKERR || STKOF;
int basic_frame     = (exc_return & 0x10) != 0;   /* FType = 1 */
frame_valid = !stacking_failed && basic_frame;
```

Frame güvenli değilse `stacked_pc` doldurulmuyor.

## Kapsam

Bu çözüm üç fault sınıfının **en yaygın** sebep bitlerini ayırt ediyor; her
`CFSR` bitini değil. Birden fazla sebep biti aynı anda set olabilir (ders 02:
`PRECISERR` + `STKERR`); burada ilk eşleşen "primary" seçiliyor. Gerçek bir
fault analizörü hepsini listeler — `cm-fault`'un `SET` satırı gibi. Bu,
`projects/fault-analyzer/`'ın işi.

---

[← Ders 08](../../README.md)
