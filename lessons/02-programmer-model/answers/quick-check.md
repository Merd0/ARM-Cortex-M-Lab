**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 02 — Quick Check Answers

> Consult this **after writing** your own answer. Reading a ready-made answer is
> the fastest way to think you know something.

---

### 1. You read `CONTROL = 0` and `IPSR = 0`. Which mode, privilege, and stack?

Privileged Thread Mode, MSP active.

Three pieces of information come from two registers:

| Field | Value | Meaning |
|---|---|---|
| `IPSR` | 0 | Thread Mode (a nonzero value would mean Handler Mode and give the exception number) |
| `CONTROL` bit 0 (`nPRIV`) | 0 | Privileged |
| `CONTROL` bit 1 (`SPSEL`) | 0 | MSP in use |

**The question's second part:** no, `CONTROL = 0` alone is insufficient.
Mode information is **not** in `CONTROL`, but in `IPSR`. `CONTROL = 0` can also
be read in Handler Mode — where even seeing `SPSEL = 0` adds nothing, because
Handler Mode already uses MSP and is already privileged.

Determine mode from `IPSR` first, then interpret `CONTROL`.

A second correction: `CONTROL` **has more than two fields**. `nPRIV` (bit 0)
and `SPSEL` (bit 1) are the fields **within this lesson's scope**; other fields
such as `FPCA` and `SFPA` are defined depending on architecture, FP context,
and the Security Extension (Lessons 13 and 16). Therefore, always update
`CONTROL` through read–modify–write.

---

### 2. The vector table's `Reset` entry is `0x10000205`, the symbol `0x10000204`. Where does the 1 come from?

The least-significant bit (bit 0) is not part of the address — it is the
**Thumb-state indicator**.

Cortex-M executes only the Thumb instruction set. The processor examines bit 0
of a branch target: 1 means "enter Thumb state." Function addresses are 2-byte
aligned, so bit 0 is otherwise unused; the architecture uses it as a flag.

If bit 0 = 0, the processor would attempt to enter Arm state. Cortex-M **has no**
Arm state, producing an INVSTATE (UsageFault) condition.

What this causes **at reset** depends on the conditions. Under reset fault
escalation rules, the processor generally enters lockup; "always lockup" is too
categorical — the context and priority at which the fault occurs determine the
outcome. See the Armv8-M ARM fault-escalation section for the exact rules.

Beware of a generalization: "all vector-table entries are odd" is **incorrect**.
It applies only to entries **holding handler addresses**. Two exceptions:

- **Entry `[0]` is not an address**; it is the initial value loaded into `MSP`
  at reset. In our build it is `0x30008000` — even.
- **Reserved entries** can be 0, and zero is not odd.

The `cm-vectors` output now includes this note beside `[0]`.

---

### 3. `MSP` is `0x30008000` at reset and `0x30007ff8` inside `main`. Where did those 8 bytes go?

Before calling `main`, `Reset_Handler` pushed data onto the stack: its return
address and the callee-saved registers it uses.

Direction: the Cortex-M stack is **full descending**. "Full" means SP points to
the most recently written valid data; "descending" means a push **decreases**
SP. The stack therefore grows downwards from the top of RAM.

This is why the linker script says `__stack_top = ORIGIN(RAM) + LENGTH(RAM)` —
the stack starts at the top of RAM, not its bottom.

---

### 4. (Reasoning) You wrote an address to PSP but did not change `SPSEL`. What does `SP` show?

Still the value of **MSP**.

`SP` (R13) is not physically one register but an **alias** (banked register).
`CONTROL.SPSEL` determines which physical register appears as `SP`:

```
SPSEL = 0  ->  SP, MSP'yi gösterir
SPSEL = 1  ->  SP, PSP'yi gösterir
```

`__set_PSP()` changes only PSP's contents, not which register is active.
Writing a value ≠ activating it.

Practical consequence: setting up a task stack in an RTOS takes two separate
steps — first write a valid address to PSP, **then** set SPSEL to 1. Reversing
the order produces the fault in Break It.

---

### 5. (Reasoning) If an exception occurs while Thread Mode uses PSP, which stack receives the frame?

**PSP** — the stack active when the exception occurred. On exception entry,
hardware uses the stack that the interrupted code was using.

The processor then enters Handler Mode and clears `SPSEL`, so the handler itself
runs on **MSP**. The Break It observation shows this too: at the fault,
`CONTROL = 0` because hardware cleared SPSEL.

Why this design? Kernel/handler code must not depend on a faulty task's stack.
If a task stack overflows or is corrupt, the handler can still run on a clean
MSP. This separation underpins Lesson 15's context switch and `EXC_RETURN`.

Note: writing the frame to PSP **does not guarantee successful stacking**.
If PSP is invalid, stacking itself faults (BusFault `STKERR`). This is exactly
what you saw in Break It.

---

[← Lesson 02](../README.md) · [Lesson 03 →](../../03-c-and-assembly/)
