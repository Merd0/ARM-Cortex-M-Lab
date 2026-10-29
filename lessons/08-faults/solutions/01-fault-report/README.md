**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — Complete the Fault Report

Exercise: [`../../exercises/01-fault-report/`](../../exercises/01-fault-report/)

Code: [`decode.c`](decode.c)

## Actual output

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

## Decision order — the same as the triage list

### 1. `active_exception` ← `IPSR`

Which handler is running now. This is **the handler that sees the event, not
the name of the event itself**.

### 2. `escalated` ← `HFSR.FORCED`

If `1`, the active handler does not identify the root cause; inspect the
escalated fault's status information.

### 3. `fault_class` ← `CFSR` subfields

```c
if      (s->cfsr & SCB_CFSR_MEMFAULTSR_Msk) d->fault_class = "MemManage";
else if (s->cfsr & SCB_CFSR_BUSFAULTSR_Msk) d->fault_class = "BusFault";
else if (s->cfsr & SCB_CFSR_USGFAULTSR_Msk) d->fault_class = "UsageFault";
```

**Not derived from the active handler.** In the escalation case, HardFault is
active, but `CFSR` still carries `UNDEFINSTR` — yielding UsageFault for
`fault_class`. This encodes the principle measured in Lessons 07 and 08.

### 4. `primary_bit` ← cause bit within the subfield

`MMARVALID` and `BFARVALID` are **not** causes; they indicate only address-register
validity. Cause bits are therefore checked first (`DACCVIOL`, `PRECISERR`,
`UNDEFINSTR`...).

BFSR bit 2 (generic Armv8-M/CMSIS name `IMPRECISERR`) is **deliberately** absent
from the cause list: this lab does not present it as an implemented Cortex-M33
field (inconsistent sources — see the BFSR bit 2 note in the lesson README).

### How verification works

Firmware checks the resulting decisions with **complete string comparison**
(a small local `str_eq()` that requires no libc). Checking only the first
character would accept `"DIVBYZERO"` as `"DACCVIOL"`; the repair measured that
`"MemManageX"`, `"Memory"`, and `"DIVBYZERO"` **FAIL** the corresponding checks.

### 5. `address_valid` / `fault_address` ← `VALID` bits

```c
if (s->cfsr & SCB_CFSR_MMARVALID_Msk) {
    d->address_valid = 1;
    d->fault_address = s->mmfar_raw;
}
```

Without the `VALID` bit, the address field stays `0` — raw contents are **not
evidence**. UsageFault has no address register at all.

### 6. `frame_safe` / `stacked_pc` ← `frame_valid`

`frame_valid` is calculated in `fault_record()` from these two conditions:

```c
int stacking_failed = MSTKERR || STKERR || STKOF;
int basic_frame     = (exc_return & 0x10) != 0;   /* FType = 1 */
frame_valid = !stacking_failed && basic_frame;
```

If the frame is unsafe, `stacked_pc` is not populated.

## Scope

This solution distinguishes the **most common** cause bits of three fault
classes, not every `CFSR` bit. Multiple cause bits can coexist (Lesson 02:
`PRECISERR` + `STKERR`); here the first match becomes "primary." A real fault
analyzer lists all of them, like `cm-fault`'s `SET` line. That is the job of
`projects/fault-analyzer/`.

---

[← Lesson 08](../../README.md)
