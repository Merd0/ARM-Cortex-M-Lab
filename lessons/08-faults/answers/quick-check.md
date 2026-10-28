**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 08 — Quick Check Answers

> Write your own answer first, then open this file.

---

### 1. Why is "a HardFault occurred" not always the root cause?

Because HardFault is often an **escalation destination**. A configurable fault (MemManage, BusFault, UsageFault) escalates to HardFault if it occurs while its own handler is disabled or cannot run.

Measured in this lesson — the same `UDF` instruction:

| | UsageFault ENABLED | UsageFault DISABLED |
|---|---|---|
| `IPSR` | 6 | **3** |
| `HFSR.FORCED` | 0 | **1** |
| `UFSR.UNDEFINSTR` | 1 | 1 — **the same** |

In both cases the root cause was an undefined instruction. HardFault was only the handler that **saw** it. Lesson 07 measured the same principle with MemManage; the rule is independent of fault class.

---

### 2. What are the three subregisters of `CFSR`?

```text
CFSR (0xE000ED28)
├── MMFSR   bits [7:0]     MemManage
├── BFSR    bits [15:8]    BusFault
└── UFSR    bits [31:16]   UsageFault
```

Three parts of a single 32-bit register; they can also be accessed separately as bytes/halfwords. The positions were verified in the vendored CMSIS `core_cm33.h` (`SCB_CFSR_MEMFAULTSR_Pos = 0`, `BUSFAULTSR_Pos = 8`, `USGFAULTSR_Pos = 16`).

---

### 3. When can `MMFAR` be trusted?

**Only when `MMFSR.MMARVALID` (CFSR bit 7) = 1.**

If `MMARVALID = 0`, `MMFAR` may contain an old value or garbage — it is **not** diagnostic evidence.

Measurement (Case 1): `MMARVALID = 1`, `MMFAR = 0x30000000` = the protected address.
Counterexample (Case 3, BusFault): `MMARVALID = 0`, `MMFAR = 0x00000000` — meaningless, not used.

---

### 4. When can `BFAR` be trusted?

**Only when `BFSR.BFARVALID` (CFSR bit 15) = 1.**

Measurement (Case 3): `BFARVALID = 1`, `BFAR = 0x30100000` = the targeted unmapped address.

Note: `BFARVALID = 1` alone does not make everything trustworthy. In the Lesson 02 case, `BFARVALID = 1` **but** `STKERR` is also set — the address is valid, the frame is not (question 5).

---

### 5. Why check stacking-error bits before decoding the frame?

Because a stacking error means **the frame itself** could not be written correctly.

```text
MMFSR.MSTKERR  (bit 4)    MemManage, stacking error on entry
BFSR.STKERR    (bit 12)   BusFault,  stacking error on entry
UFSR.STKOF     (bit 20)   stack limit exceeded
```

If one of these bits is set, memory may not contain a reliable exception frame; the "R0" and "stacked PC" read from it may not represent the interrupted context.

The correct order:

1. Read fault status.
2. Decide whether stacking succeeded.
3. Use the `EXC_RETURN` saved at entry to identify the stack containing the frame.
4. **Only then** decode the frame.

This lesson's `fault_record()` does exactly this (`frame_valid`), and `cm-frame` rejects a frame when it sees a stacking error — measured in the Lesson 02 case:
`REDDEDILDI [STACKING HATASI]: CFSR = 0x00009200`.

---

### 6. Is the stacked PC always the faulting instruction?

**No.** Its meaning depends on the exception type.

| Situation | Stacked PC | In this repository |
|---|---|---|
| Precise fault | The faulting instruction **itself** | Measured: `udf` / `ldr` / `str` |
| SVC (Lesson 07) | The instruction **after** `svc` | Measured: `0x100000b8` → `0x100000ba` |
| Imprecise fault | **May not identify** the access that caused the fault | Not measured — architectural caution |

For a precise fault, the instruction did not complete, so the PC points to it. `SVC` is a completed instruction; return is to the continuation address. An imprecise error may be reported late, after the processor has moved on.

---

### 7. What does `HFSR.FORCED` tell you?

That an exception with configurable priority was **escalated** to HardFault — or that another escalation condition defined by the architecture occurred.

This HardFault resulted from escalation; investigate the underlying cause through `CFSR`.

`FORCED` does **not** mean "the processor forced a random crash." It means an architectural routing rule took effect.

Measured three times in this repository: Lesson 02 (`STKERR` → HardFault), Lesson 07 (`DACCVIOL` → HardFault), Lesson 08 (`UNDEFINSTR` → HardFault).

---

### 8. What happens if a configurable fault occurs while disabled?

Conceptually: the fault **does not disappear**, it **escalates**. The HardFault handler runs and `HFSR.FORCED` is set.

The hardware that writes status registers operates independently of handler selection, so `CFSR` and the address registers (when valid) are still written correctly.

> **OBSERVATION**, not universal: the `UNDEFINSTR` escalation measured in this lesson produced `HFSR = 0x40000000` and preserved `CFSR = 0x00010000`. This does not claim that every disabled fault produces the **same** register values.

---

### 9. (Reasoning) How do precise and imprecise BusFaults differ diagnostically?

**Precise (`PRECISERR`):** the processor can associate the error with a particular instruction. The stacked PC generally identifies that **instruction's** address; if `BFARVALID` is set, `BFAR` provides the **data** address the instruction attempted to access. These are different addresses that together describe one access — as measured in Case 3:

```text
stacked PC = 0x100000F8   instruction address  (ldr r0, [r0])
BFAR       = 0x30100000   data address   (where ldr tried to read)
```

**Imprecise (concept):** reporting is delayed. A typical example is a failing write that passes through a write buffer — the processor has already moved on to other instructions. Consequently:

- the stacked PC **may not identify** the access that caused the error,
- `BFAR` is generally **not valid**.

Diagnosis is therefore harder: locate the suspected access by examining the code and narrowing the timing window. Disabling write buffering during development is one technique for making some imprecise errors precise.

> A deterministic imprecise BusFault could not be established in this environment; this answer explains the general fault model, not a measurement.
>
> **Cortex-M33 note:** Generic Armv8-M/CMSIS material names the imprecise condition with BFSR bit 2 `IMPRECISERR`. This lab **does not present this bit as an implemented Cortex-M33 field**: the independent audit reported that the M33 BFSR register table marks bit 2 Reserved/RES0, while the summary table in the accessible M33 guide text still lists the term. Since the sources are inconsistent, bit 2 is labeled only "`bit2 (generic
> IMPRECISERR)`".

---

### 10. (Reasoning) Why capture status registers **before** extensive processing?

Three reasons:

1. **The handler changes context.** The C prologue changes `SP`, any `bl` overwrites `LR` (`EXC_RETURN`), and registers are reused.
2. **The handler itself can fault.** Console work, memory accesses, called functions — all can generate another fault, and sticky bits **accumulate**. Reading `CFSR` later may mix two faults.
3. **Time.** Console work takes time; other exceptions can arrive meanwhile.

This lesson's wrappers save `LR`/`MSP`/`PSP` in assembly **as their first action**; `fault_record()` captures `CFSR`/`HFSR`/`MMFAR`/`BFAR` in its first lines. Console work comes afterward.

---

### 11. (Reasoning) Why is the live handler `$sp` not automatically the interrupted frame?

Two independent reasons (both measured in Lesson 07):

1. **The frame can be on another stack.** If Thread uses PSP, the frame is on PSP while the handler uses MSP. In all four cases in this lesson, the frame was on PSP (`0x300002D8`/`0x300002F8`), while handler `$sp` = `0x30007FF0`.
2. **Even on the same stack, the handler stacks its own data.** Its prologue and local variables move MSP below the frame, toward lower addresses.

The correct method: save `EXC_RETURN` at entry, inspect bit 2, and use the entry MSP/PSP. That is why `cm-frame <adres> <EXC_RETURN>` takes two arguments (address and exception-return value).

---

### 12. (Reasoning) What is the first thing to inspect after entering HardFault?

**`HFSR.FORCED`.**

- `FORCED = 1` → this is escalation; investigate the underlying cause through `CFSR`.
- `FORCED = 0` → HardFault arose directly (for example, a vector-table read error, `VECTTBL`). Inspect the other bits of `HFSR`.

Immediately afterward, **before touching the frame**: check the stacking-error bits in `CFSR`. If they are not clear, do not try to decode the frame.

These are steps 2 and 6 of the triage list — their order matters.

---

### 13. (Reasoning) Why does writing `0` to `CFSR` not clear it?

Because `CFSR` and `HFSR` are **write-one-to-clear (W1C)** registers: writing **1** to a bit clears that bit; writing **0** does nothing.

```c
SCB->CFSR = 0;             /* HICBIR SEY TEMIZLENMEZ */
SCB->CFSR = SCB->CFSR;     /* set olan her bite 1 yaz -> hepsini temizle */
SCB->CFSR = SCB_CFSR_DACCVIOL_Msk;   /* yalnizca DACCVIOL'u temizle */
```

**Do not use the same read-back/write-back pattern for HFSR.** The Cortex-M33 HFSR bit table (as reported by the independent audit) requires writing **1** to `DEBUGEVT` (bit 31) on every HFSR write; otherwise behavior is UNPREDICTABLE. `SCB->HFSR = SCB->HFSR` writes 0 to bit 31 when `DEBUGEVT` is clear. This lesson uses an explicit mask:

```c
uint32_t hfsr = SCB->HFSR;
SCB->HFSR = SCB_HFSR_DEBUGEVT_Msk |
            (hfsr & (SCB_HFSR_FORCED_Msk | SCB_HFSR_VECTTBL_Msk));
```

Measurement (QEMU, escalation case): written value `0xC0000000`, HFSR `0x40000000` → `0x00000000`. QEMU does **not validate** this write rule (it implements HFSR as plain W1C); this was not tested on real hardware. The Cortex-M3/M4 guides' instruction to "write 0" for `DEBUGEVT` does not apply to this core.

The design rationale: avoid software inadvertently clearing a bit that hardware set for a new fault during a read-modify-write sequence. Software clears only the bits it **observed**.

Practical consequence: when running multiple fault experiments in the same firmware session, failing to clear with W1C between experiments leaves the previous experiment's bits in `CFSR`, mixing two faults. Each target in this lesson starts from reset and generates a single fault, so this problem does not arise here.

---

## Experiment 2 answers — which evidence is trustworthy?

| Evidence | 1. Clean `DACCVIOL` | 2. Escalated `UNDEFINSTR` | 3. `UsageFault` | 4. Stacking error (Lesson 02) |
|---|---|---|---|---|
| Active handler | ✅ MemManage | ✅ HardFault — but **not the root cause** | ✅ UsageFault | ✅ HardFault — not the root cause |
| `HFSR.FORCED` | ✅ 0 | ✅ 1 → inspect `CFSR` | ✅ 0 | ✅ 1 → inspect `CFSR` |
| `CFSR` cause bit | ✅ `DACCVIOL` | ✅ `UNDEFINSTR` | ✅ `UNDEFINSTR` | ✅ `PRECISERR` + `STKERR` |
| Address register | ✅ `MMFAR` (VALID) | — none | — none | ✅ `BFAR` (VALID) |
| Frame | ✅ valid | ✅ valid | ✅ valid | ❌ **`STKERR` → do not trust** |
| Stacked PC | ✅ `str` | ✅ `udf` | ✅ `udf` | ❌ not read |

The subtle point in the last column: **the address** is trustworthy because `BFARVALID = 1`, but **the frame** is not because of `STKERR`. Address validity and frame validity must therefore be assessed separately.

---

## References

- Armv8-M ARM (Arm DDI 0553) — `CFSR`, `HFSR`, `MMFAR`, `BFAR`, `SHCSR`, W1C, escalation, precise/imprecise
- Armv8-M Exception Model User Guide (Arm 107706)
- Cortex-M33 Devices Generic User Guide (Arm 100235) — 4.2.11 CFSR, 4.2.12 HFSR (W1C / sticky; `DEBUGEVT` write rule), 2.5.1 fault types
- CMSIS-Core `core_cm33.h` — bit positions (generic Armv8-M definitions; including BFSR bit 2)

---

[← Lesson 08](../README.md) · [Lesson 09 →](../../09-compiler-aapcs/)
