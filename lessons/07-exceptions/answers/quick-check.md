**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 07 — Quick Check Answers

> Write your own answer before opening this file.

---

### 1. Are exceptions and external interrupts the same?

No — but they use **the same mechanism**.

`Exception` is the superset, containing two groups:

- **System exceptions**: architecturally defined and numbered — Reset (1), NMI (2),
  HardFault (3), MemManage (4), BusFault (5), UsageFault (6), SecureFault (7),
  SVCall (11), DebugMonitor (12), PendSV (14), SysTick (15).
- **External interrupts**: device-specific. IRQ *n* has exception number `16 + n`
  and uses vector-table entry `[16 + n]`.

**What they share:** all exceptions except Reset follow the same entry/exit path —
pending → acceptance → vector lookup → hardware stacking → Handler Mode →
`EXC_RETURN` → restoration. Everything measured through SVCall here also applies
to a device IRQ.

**Differences:** who defines them (architecture vs. chip manufacturer), what
triggers them (instruction/fault vs. hardware signal), and which register family
configures them (`SHPR*` vs. `NVIC` registers).

"Interrupt" can be used broadly in everyday speech; architectural explanations
must preserve the distinction.

---

### 2. Why is vector-table entry 0 not a handler address?

Because there is **no valid stack yet** at reset. Hardware needs an `MSP` value
before executing the first instruction and reads it from `vector[0]`.

The order is `vector[0]` → `MSP`, then `vector[1]` → `PC`.

Measured in this build:

```text
[0] initial MSP  0x30008000     <- cift sayi
[1] Reset        0x10000535     <- tek sayi (Thumb biti)
[11] SVCall      0x10000041     <- tek sayi
```

`vector[0]` is **not a handler address**; it holds the initial `MSP` value.
`0x30008000` is even because it is a **stack-pointer value**: word/8-byte aligned,
with no applicable Thumb-bit concept. (It is a memory address, just not a branch target.)

Reset itself is also special: `Reset_Handler` starts in **privileged Thread Mode**,
not Handler Mode like an ordinary handler. At its first instruction this build
measured `IPSR = 0`, `LR = 0xffffffff`, `MSP = 0x30008000` (matching `vector[0]`).
There is no stacked frame, no `EXC_RETURN`, and no context to return to.

Thus "all values in the vector table are odd" is **wrong**. Two exceptions:
`vector[0]` and reserved entries — the latter may be `0`.

---

### 3. Which registers are automatically written to the basic frame? Why exactly those?

Eight words, starting at the lowest address:

```text
+0x00 R0   +0x04 R1   +0x08 R2   +0x0C R3
+0x10 R12  +0x14 LR   +0x18 PC   +0x1C xPSR
```

**Why these?** The eight words comprise two distinct groups:

- **`R0-R3` and `R12`** — AAPCS (Lesson 03) **volatile (caller-clobbered)** working
  registers, freely overwriteable by the callee in an ordinary call. Hardware
  saving them lets the handler be **an ordinary C function**: the compiler already
  preserves `r4-r11` in its prologue (callee-saved), while freely using `r0-r3`/`r12`.
- **`LR`, `PC`, `xPSR`** — stacked not because of an ABI category, but to restore
  **interrupted control flow and architectural state**: return location, flags,
  and execution state.

The frame is **not** "the AAPCS caller-saved set." A correct summary is the
volatile working registers immediately needed to resume ordinary code, plus
control-flow/state information needed to continue interrupted execution.

This layout makes the exception mechanism work compatibly with the ABI.

In this lesson's measurement, marker values loaded before `SVC` appeared unchanged
in the frame (`0x11111111` at `+0x00`, `0xCCCCCCCC` at `+0x10`).

---

### 4. If `SVC` occurs while Thread uses PSP, which stack receives the frame?

**PSP** — the stack **active at the time of the exception**.

Measurement: before `SVC`, `PSP = 0x300002F0`; at handler entry,
`PSP = 0x300002D0`. Exactly 32 bytes (8 words) lower.

`EXC_RETURN` bit 2 confirms this: `0xFFFFFFFD` → bit 2 = 1 → frame on PSP.

In Experiment 1's MSP-origin case, `EXC_RETURN = 0xFFFFFFF9` (bit 2 = 0), and
the frame was written to MSP. The same rule: **the active stack**.

---

### 5. Which stack does the handler use? Does software or hardware choose?

**MSP**, selected by **hardware**.

On exception entry, the processor clears `CONTROL.SPSEL`. Measurement:

```text
Thread'de:   CONTROL = 0x2   (SPSEL = 1, PSP aktif)
Handler'da:  $sp == $msp == 0x30007ff0
```

No software instruction cleared `SPSEL`.

**Design reason:** handler code should not depend on the stack of a faulty Thread
context. Even if the Thread stack overflows or is corrupted, the handler runs
on a clean MSP.

The difference from question 4: **the frame can be on PSP while the handler uses
MSP.** They are separate questions.

(Assuming one security state. With TrustZone, this is the MSP of the *current
security state* — Lesson 16.)

---

### 6. Why is `LR` not a normal return address on handler entry?

Because entry did not happen through `BL`. When invoking the handler, hardware
writes a special value called **`EXC_RETURN`** into `LR` instead of a return address.

This value is not an address; its upper 25 bits are fixed (`0xFFFFFF80` pattern),
and the low bits describe **how** return will happen:

```text
bit3 Mode   1 = Thread'e donus,  0 = Handler'a donus
bit2 SPSEL  1 = frame PSP'de,    0 = frame MSP'de
bit4 FType  1 = standart frame,  0 = genisletilmis (FP)
```

Measured: PSP-origin `0xFFFFFFFD`, MSP-origin `0xFFFFFFF9`.

Practical consequence: a `bl` inside the handler overwrites `LR`, losing
`EXC_RETURN`. The wrapper's **first action** here is to save it — the other side
of precisely the issue that required the Lesson 04 `cm-frame` repair.

---

### 7. How can `BX LR` initiate exception return?

The processor checks whether the value written to `PC` has an `EXC_RETURN`
encoding. If so, it **does not interpret it** as a branch target; it performs
exception return.

Measured in one step here. Before `bx lr`:

```text
$lr = 0xfffffffd
Mode: Handler Mode (exception 11), Stack: MSP
```

After a single `si`:

```text
Mode: Thread Mode, Stack: PSP (0x300002f0)
r0  = 0x11111112   <- handler'in frame'e yazdigi yeni deger
r1  = 0x22222222   <- degismeden geri yuklendi
r12 = 0xcccccccc   <- degismeden geri yuklendi
```

One instruction did all of this: `IPSR` 11 → 0, active stack MSP → PSP,
eight words restored, `PC` jumped to its stacked value.

Note: exception return is initiated not by `BX` itself, but by **writing an
`EXC_RETURN` value to `PC`**. `POP {..., pc}` also works — the Lesson 05 dispatcher
used that.

---

### 8. What is the difference between `Pending` and `Active`?

```text
Inactive --(olay/istek)--> Pending --(kabul)--> Active --(donus)--> Inactive
```

- **Pending:** the exception was requested but is not yet running.
- **Active:** its handler is running (or is interrupted by a nested exception).

An exception can be pending and active simultaneously: if requested again while
its handler is running.

**Pending does not always mean "will run immediately."** Masking, an active
exception with equal or higher priority, or architectural eligibility conditions
can make it wait.

We measured active state here through `SHCSR`:

```text
MemManage etkinken:  SHCSR = 0x00010001  (bit0 MEMFAULTACT = 1)
MemManage kapaliyken: SHCSR = 0x00000004  (bit2 HARDFAULTACT = 1)
```

Two different handlers active, from the same violation.

---

### 9. What does a lower priority number mean?

**Higher urgency.** Priority rises as the number decreases.

Fixed, nonprogrammable priorities:

```text
Reset      -3   (en yuksek)
NMI        -2
HardFault  -1
```

The remaining system exceptions and all external IRQs are configurable and
default to `0` — lower priority than HardFault.

Therefore, another exception of the same priority cannot preempt a running fault
handler, but NMI can.

This may seem counterintuitive ("priority 1" is most important in everyday
language). Think of architectural priority as an **ordering number**: 0 comes first.

---

### 10. Why are 8 priority bits not a universal Cortex-M33 fact?

Because **the number of implemented priority bits is an implementation option**,
chosen by the chip manufacturer when synthesizing the core. The architecture
defines only an upper bound.

Measured on this target:

```text
NVIC_IPR0_READBACK       0x000000FF
NVIC_PRIO_BITS_MEASURED  8
NVIC_PRIO_BITS_COMPILED  8
NVIC_PRIO_BITS_MATCH     YES
```

Measurement method: write `0xFF` to a priority register and read it back.
Unimplemented **low** bits read as zero.

On real MCUs, 2, 3, or 4 bits are very common. Code assuming 8 bits can silently
assign the wrong priority on another device — where you write `0x20`, hardware sees `0x00`.

> **TARGET CONFIG** vs. **ARCHITECTURE**: "8 bits" is this QEMU target's configuration,
> not an architectural constant. This is why this repository bases `__NVIC_PRIO_BITS`
> on measurement, and `capability-probe` compares it with the compile-time value
> on every run.

---

### 11. (Reasoning) Why is `cm-frame $sp ...` not automatically correct inside a handler?

Two independent reasons, both measured in this lesson.

**Reason 1: the frame may be on another stack.** If Thread uses PSP, the frame
is stacked there, but the handler runs on MSP. In that case `$sp` (= MSP) is
unrelated to the region holding the frame:

```text
$sp / $msp    = 0x30007ff0
frame ($psp)  = 0x300002d0
```

**Reason 2: even on the same stack, the handler stacks its own data.** Its
prologue and locals move `SP` away from the frame start. Measurement:

```text
SVC_Handler ilk komut:   $sp = 0x30007ff0
svc_dispatch icinde:     $sp = 0x30007fe0    <- 16 byte asagi
```

For an MSP-origin exception, the frame was at `0x30007FC0`. Because the stack is
full-descending, the handler's prologue and locals moved MSP **further downward**;
handler stack use grew **below the saved frame, at lower addresses** (dispatcher
local data at `0x30007FB0`). There is no fixed relationship between `$sp` and the frame.

This is why `cm-frame` **requires the frame address as an argument** and does not
guess from `$sp`. The independent audit found that the old version decoded
unrelated memory as an "exception frame" for exactly this reason and printed
fabricated values such as `PC = 0`.

Correct method: save `LR` **immediately on handler entry**, examine `EXC_RETURN`
bit 2, and use entry MSP or PSP. This lesson's wrapper does precisely that.

---

### 12. (Reasoning) Why does an MPU violation become another exception when MemManage is disabled?

Because a fault **does not disappear**. If its assigned handler is unavailable,
the architecture **escalates it to HardFault**.

The same violation was measured twice here:

| | MemManage ENABLED | MemManage DISABLED |
|---|---|---|
| `IPSR` | 4 | **3** |
| `MMFSR` | `0x82` | `0x82` — **same** |
| `MMFAR` | `0x30000000` | `0x30000000` — **same** |
| `HFSR` | `0x00000000` | `0x40000000` |
| `HFSR.FORCED` | 0 | **1** |
| Stacked PC | `0x10000110` | `0x10000110` — **same** |

**Nothing happens to `MMFSR`.** Hardware writing the status registers works
independently of handler selection. Even an escalated HardFault therefore retains
the actual cause in `CFSR` and `MMFAR` — diagnose by inspecting `HFSR.FORCED` first,
then `CFSR`.

`HFSR.FORCED = 1` means exactly: *"this HardFault did not arise on its own;
another fault was escalated."*

There are other escalation causes, such as a fault occurring while an equal- or
higher-priority handler is running. We saw `FORCED = 1` for another reason in
Lesson 02's `STKERR` case. Details are in Lesson 08.

---

### 13. (Reasoning) Must the handler save `r4-r11`? Why does hardware not stack them?

**Hardware does not stack them, but the handler needs no special action either** —
the compiler already does the work.

`r4-r11` are **callee-saved** under AAPCS (Lesson 03). For a C handler, the compiler
pushes them in the prologue if it will use them and pops them in the epilogue.
The interrupted context's `r4-r11` values are therefore preserved.

Not stacking them in hardware is a deliberate design decision: **exception
latency**. Stacking more context can increase entry/exit work and latency — but
total latency depends on the implementation and other architectural/microarchitectural
factors; "16 words = exactly twice the latency" does not follow. The architecture
does not duplicate what the ABI already guarantees.

When must you handle it manually? **When writing an assembly handler.** This
lesson's wrapper freely uses `r0-r3` and `r12` (hardware saved them), but never
touches `r4-r11`. If it did, it would have to push/pop them itself.

The Lesson 15 context switch deliberately fills this gap: PendSV **manually**
stacks `r4-r11`, because the eight hardware-stacked words are insufficient when
switching between tasks.

---

## Exercise 02 — `EXC_RETURN` classification answers

| Value | Return mode | Frame location | Frame type | Supported by `cm-frame`? |
|---|---|---|---|---|
| `0xFFFFFFF9` | Thread | MSP | basic (FType=1) | ✅ Yes |
| `0xFFFFFFFD` | Thread | PSP | basic (FType=1) | ✅ Yes |
| `0xFFFFFFED` | Thread | PSP | **extended** (FType=0) | ❌ `[DESTEKLENMIYOR]` |
| `0xFFFFFFF1` | **Handler** | MSP | basic | ✅ Yes |
| `0xFFFFFFF5` | — | — | — | ❌ `[BICIMSIZ EXC_RETURN]` |

The last line matters: `0xFFFFFFF5` would mean bit 3 = 0 (return to Handler)
**and** bit 2 = 1 (frame on PSP). Since Handler Mode always uses the Main stack,
there is **no valid encoding** of this kind; `cm-frame` rejects it as malformed,
rather than reporting "return to Thread."

`0xFFFFFFED`, however, is **architecturally valid** — only this lab's decoder does
not support extended FP frames. "Invalid" and "unsupported" are different.

Check for yourself:

```gdb
(gdb) cm-exc-return 0xFFFFFFED
```

---

## References

- Armv8-M ARM (Arm DDI 0553) — exception model, `EXC_RETURN`, escalation
- Armv8-M Exception Model User Guide (Arm 107706) — exception numbers, `EXC_RETURN` bit fields
- Cortex-M33 Devices Generic User Guide (Arm 100235) — `SHCSR`, priority registers
- AAPCS32 — volatile / callee-saved register distinction
- [`docs/DEBUGGING.md`](../../../docs/DEBUGGING.md) — `cm-frame` /
  `cm-exc-return` usage and limits

---

[← Lesson 07](../README.md) · [Lesson 08 →](../../08-faults/)
