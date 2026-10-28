**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 05 — Quick Check Answers

> Write your own answer before opening this file.

---

### 1. What does `CONTROL.nPRIV` control? Does it cover Handler Mode?

`CONTROL.nPRIV` (bit 0) defines the privilege level of **Thread Mode**:
`0` = privileged, `1` = unprivileged.

**It does not cover Handler Mode.** Handler Mode is always privileged. Reading
`CONTROL` inside a handler may show `nPRIV = 1` — this **does not** mean the
handler is unprivileged; it describes the interrupted Thread context.

Measured here: `CONTROL = 0x1` was read inside `svc_dispatch`, and `cm-mode` printed:

```text
Mode      : Handler Mode (exception 11)
Privilege : Privileged (Handler Mode her zaman ayricaliklidir)
            CONTROL.nPRIV = 1 -> KESILEN Thread baglami ayricaliksizdi
```

Correct order: **determine the mode from `IPSR` first, then interpret `CONTROL`.**

---

### 2. Can an unprivileged Thread clear `nPRIV` to gain privilege?

**No.** Writes to `CONTROL` from unprivileged Thread Mode are ignored.

Measurement:

```text
CONTROL_UNPRIV          0x00000003    (nPRIV = 1)
--- __set_CONTROL(CONTROL & ~1) calistirildi ---
CONTROL_AFTER_ATTEMPT   0x00000003    (degismedi)
```

No fault, error, or warning — the write silently has no effect. Gaining
privilege requires **a privileged execution path**: an exception (such as SVC)
enters a handler, which is privileged and may choose to clear `nPRIV` for
the returning context.

This lesson's `SVC #4` service does exactly that — but notice that the
**handler** executes the line. The same line was ignored when executed from
Thread. What matters is **where it executes**, not the code itself.

---

### 3. Why can `SVC_Handler` perform privileged work?

Not because its code is special. **Because it runs in Handler Mode.**

Taking an exception puts the processor in Handler Mode, which is architecturally
privileged. Privilege therefore comes from the **execution context**, not the
identity of the code.

An important practical consequence: any function called inside `SVC_Handler`
also runs privileged. Handler behavior must therefore be carefully constrained —
this is why a syscall dispatcher rejects unrecognized requests (Break It).

Vector table entry 11 pointing to `SVC_Handler` means “call this function for
the SVCall exception.” The exception grants privilege, not the table.

---

### 4. Are “Unprivileged” and “Non-secure” the same thing?

**No.** Two separate mechanisms, two independent axes.

| | Privilege | Security state |
|---|---|---|
| Control | `CONTROL.nPRIV` + mode | TrustZone / SAU |
| Values | Privileged / Unprivileged | Secure / Non-secure |
| Availability | Every Cortex-M | Only with the Security Extension |

All four combinations are possible: Secure-privileged, Secure-unprivileged,
Non-secure-privileged, Non-secure-unprivileged. The Non-secure side also has
its own privileged code (for example, a Non-secure RTOS kernel).

This lesson runs in one security state (Secure) and changes only the privilege
axis. Details are in Lesson 16.

---

### 5. If SVC occurs while Thread uses PSP, where is the frame written?

**To PSP** — the stack active at the exception. Hardware uses the stack of
the interrupted context.

Measurement: at handler entry, `$psp = 0x30000308`, and the wrapper computed
`frame = 0x30000308`. The same address.

`EXC_RETURN` bit 2 tells us this: `0xFFFFFFFD` → bit 2 = 1 → frame on PSP.
`svc_handler.S` tests precisely this bit (`tst lr, #4`).

Caution: “written to PSP” and “the write succeeded” are different. If PSP
is invalid or below `PSPLIM`, stacking itself faults (Break It in Lessons
02 and 04).

---

### 6. Which stack does Handler Mode use? Who selects it?

**MSP.** **Hardware** selects it, not software.

On exception entry, the processor clears `CONTROL.SPSEL`, so the handler
runs on MSP. Measurement confirms this:

```text
In Thread:   CONTROL = 0x3   (SPSEL = 1, PSP)
In Handler:  CONTROL = 0x1   (SPSEL = 0, MSP)
             $sp == $msp == 0x30007ff0
```

No software line cleared `SPSEL`.

Design reason: handler code should not depend on a faulty task's stack.
Even if that stack overflows or is corrupted, the handler continues on a
clean MSP.

(Assuming a single security state. With TrustZone, this is the MSP of the
*current security state*.)

---

### 7. Does `SVC` automatically make the returning Thread privileged?

**No.** The behavior measured on this target is described below.

Immediately after returning from `SVC #1`:

```text
Mode      : Thread Mode
Privilege : Unprivileged (CONTROL.nPRIV = 1)
$control = 0x3
```

`SVC` is only **the gateway**: it provides entry to a privileged context.
**Handler policy** determines what happens on return.

This lesson demonstrates two policies together:

- `SVC #1` (increment): does its work without changing privilege → Thread
  returns unprivileged.
- `SVC #4` (restore): the handler clears `nPRIV` → Thread returns privileged.

A real RTOS usually does not do the latter; tasks remain unprivileged.
Here it is deliberate, to expose the mechanism.

---

### 8. (Reasoning) Why can the `SVC` immediate be read at `PC - 2`?

Two facts combine:

1. **The stacked PC points to the instruction after `SVC`.** Hardware stacks
   the address where the interrupted context **will resume**.
2. **The Thumb T1 encoding of `SVC` is 16 bits:** `0b11011111:imm8`, or
   `0xDFxx`. The instruction is 2 bytes long, with `imm8` in the low byte.

Thus, `pc - 2` is the start of the instruction, and the byte there is `imm8`.

Measured in this build:

```text
10000194:  df01   svc  1        <- pc - 2
10000196:                       <- stacked pc
```

**Does this apply to every exception? No.** This calculation applies only
to the 16-bit `SVC` instruction. Generalizing it fails in two ways:

- **32-bit instructions:** if the offending instruction occupies 4 bytes,
  `pc - 2` lands in its middle.
- **The meaning of stacked PC changes for faults.** For a precise fault,
  stacked PC may identify the faulting instruction; for an imprecise fault,
  the instruction responsible may already be far behind. Lesson 08 expands this.

The right habit: inspect the generated encoding with `objdump`, rather than assuming.

---

### 9. (Reasoning) What is missing for a truly isolated syscall boundary?

**Memory isolation.** This lesson established privilege *flow*, not protection.

`g_service_counter` is an ordinary RAM variable. Unprivileged Thread can
increment it **directly**, bypassing `SVC`, without anything happening.

How would you check? Add this line in the unprivileged section and run:

```c
g_service_counter = 1000u;      /* SVC'siz, dogrudan */
```

Then call `SVC #1` and inspect its result. If you see `1001`, the contract
has been bypassed. (Remove the line afterward.)

What is missing:

| Item | Current state |
|---|---|
| Kernel data | Writable by unprivileged code |
| Thread stack ↔ kernel stack | Same address space; no hardware separation |
| Code region | Readable by unprivileged code |
| One task writing to another | Not prevented |

The only actual restriction measured was peripheral access
(`UART_BAUDDIV_UNPRIV = 0`) — from the SoC's **Peripheral Protection Controller**,
not the MPU. A different block and mechanism.

**Lesson 06 — MPU** completes this: `g_service_counter` is closed to
unprivileged code, and a direct write causes a **MemManage fault**. Then
“you must use SVC” becomes a **hardware rule**, not just a contract.

---

## References

- Armv8-M ARM (Arm DDI 0553) — `CONTROL`, Thread/Handler Mode, exception
  entry, `SVC` T1 encoding
- Armv8-M Exception Model User Guide (Arm 107706) — SVCall = exception 11,
  `EXC_RETURN`
- Cortex-M33 Devices Generic User Guide (Arm 100235)
- AAPCS32 — return value in `r0`

---

[← Lesson 05](../README.md) · [Lesson 06 →](../../06-mpu/)
