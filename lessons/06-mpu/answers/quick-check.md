**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 06 — Quick Check Answers

> Write your own answer before opening this file.

---

### 1. What does the MPU enforce? What does it not enforce?

**It enforces:** whether a memory access is permitted for the accessing
context's privilege level. Each access (data read/write, instruction fetch)
is checked against the region table; without permission, `MemManage` is raised.

Alongside permissions, it defines **memory type** (`RLAR.AttrIndx` → `MAIR`):
Normal or Device, cacheability, shareability.

**It does not enforce:**

- *Logical* correctness of code. Writing incorrect data into a permitted
  region does not concern the MPU.
- Security-state separation (Secure/Non-secure) — TrustZone's job.
- SoC-level peripheral access control — the PPC's job (question 10).
- Protection of privileged software itself. Privileged code configures
  the MPU; a bug there can remove protection.

---

### 2. How do privilege and the MPU relate?

**Privilege supplies the level; the MPU restricts memory according to it.**
Neither suffices alone:

| Privilege alone (Lesson 05) | MPU alone |
|---|---|
| “This code runs unprivileged” | Meaningless — region permissions use `NP` to depend **on privilege** |
| But it can access all RAM | Without privilege, there is no answer to “who” |

The difference between Lessons 05 and 06 is concrete: same `g_service_counter`,
same unprivileged Thread. Direct writes **worked** in Lesson 05 (a contract);
in Lesson 06 they cause `MemManage` (a hardware rule). Only the MPU policy changed.

---

### 3. Why can unprivileged Thread access its own stack?

Region 1 was defined precisely for that purpose:

```text
RBAR = 0x30000023
       AP  = 01  -> RO=0 (read/write), NP=1 (ayricaliksiz erisime acik)
       XN  = 1   -> calistirma YOK
       BASE      -> 0x30000020
RLAR = 0x30000321
       LIMIT     -> son byte 0x3000033F (DAHIL)
       EN  = 1
```

`NP = 1` means “allow unprivileged access too.” Thread's stack and data area
lie in this range; the measured value was `STACK_WRITE_MAGIC = 0xA5A5C0DE`.

`XN = 1` provides separate protection: data injected into the stack cannot
be executed as code (question 7).

---

### 4. Why can the same Thread not access `g_service_counter`?

Because **no region was defined to cover it**.

This seems reversed: protection comes from **omitting** a region, not
*adding* one. The Armv8-M rule is: with the MPU enabled, an unprivileged
access without a matching region is **denied**. Privileged accesses have
the `PRIVDEFENA` fallback; unprivileged accesses do not.

Measured addresses:

```text
COUNTER_ADDR  = 0x30000000
UNPRIV_BASE   = 0x30000020   <- Region 1 starts here
```

The counter lies **below** the unprivileged region, with no other region covering it.

---

### 5. Why can `SVC_Handler` access the same variable?

A three-step chain:

1. `SVC` is an exception → the processor enters **Handler Mode**.
2. Handler Mode is **always privileged** (Lesson 05).
3. With `MPU_CTRL.PRIVDEFENA = 1`, a privileged access without a matching
   region uses the **default memory map**, permitting the access.

The handler's privilege comes from context, not code; its access permission
comes from `PRIVDEFENA`, not an explicit region policy.

Measured values: `SVC_INCREMENT_1 = 1`, `SVC_INCREMENT_2 = 2` — same variable,
same firmware, different context.

---

### 6. What does `PRIVDEFENA` provide? What would break with `0`?

`MPU_CTRL.PRIVDEFENA = 1`: privileged accesses without a matching region
use the architecture's **default memory map**.

With `0`, privileged code would also follow “no match means deny.” Everything
in this lesson would break:

- `main()` could not access its own MSP stack (around `0x30007FF0`) — no
  region covers it.
- The handler could not access `g_service_counter`.
- UART would be inaccessible for console output.

Selecting `PRIVDEFENA = 0` therefore requires regions for **every privileged
access too**. Some high-security designs deliberately do this to constrain
privileged code, but the policy grows much larger — 8 regions quickly run out.

Measured: `MPU_CTRL = 0x00000005` = `ENABLE | PRIVDEFENA`.

---

### 7. What does `XN` prevent? Why use `XN = 1` for the Thread block?

`XN` (execute never) prevents **instruction fetch** from that region.
Violating it sets `MMFSR.IACCVIOL`, a different bit from the `DACCVIOL`
measured in this lesson.

The Thread block contains stack and data, with no legitimate reason to
execute code there. `XN = 1` is classic hardening: data injected through
stack or buffer overflow **cannot execute**. The same idea as desktop W^X / DEP.

Symmetrically, Region 0 (FLASH) has `XN = 0` and `RO = 1`: executable but
not writable. Together they establish “writable memory is not executable;
executable memory is not writable.”

---

### 8. How does Armv8-M's `RBAR`/`RLAR` model differ from Armv7-M?

| | Armv7-M (PMSAv7) | Armv8-M (PMSAv8) |
|---|---|---|
| Region definition | `base` + **encoded size field** (power of 2) | `RBAR.BASE` + `RLAR.LIMIT` |
| Alignment | Region must align to **its own size** | 32-byte granularity suffices |
| Subregions | `SRD` (subregion disable) field | **None** |
| Memory type | `TEX`/`C`/`B`/`S` fields within the region | `RLAR.AttrIndx` → `MAIR` |
| Size | Must be a power of 2 | Any multiple of 32 bytes |

`LIMIT` is **inclusive** (the region's final byte), not “end + 1.”

**Why online examples cannot be used:** Armv7-M examples populate `SIZE`,
use `SRD`, and assume size alignment. None of these fields exists in Armv8-M.
Copying them either fails to compile or programs an entirely different
region. The right habit: consult the Armv8-M ARM or vendored `armv8m_mpu.h`.

---

### 9. Why does MemManage matter? Does disabling `MEMFAULTENA` remove a violation?

**No.** An MPU violation still causes a fault; `MEMFAULTENA` determines only
**which handler** receives it.

`MEMFAULTENA = 1` (this lesson): the violation goes directly to the MemManage
handler. Measured: `IPSR = 4`, `HFSR = 0` — no escalation, and `MMFSR`/`MMFAR`
can be read directly.

`MEMFAULTENA = 0`: MemManage is disabled, so the fault **escalates to HardFault**
and sets `HFSR.FORCED`. We saw this as `HFSR = 0x40000000` in Lesson 02.

Why does this matter? Diagnostic quality changes. HardFault says “something
happened”; MemManage says “this type of access to this address was denied.”
An RTOS can also catch MemManage and terminate **only the faulty task**;
HardFault generally represents a system-wide crash.

> The `MEMFAULTENA = 0` behavior in this answer was **not measured** in this
> lesson — it is left to you as Experiment 3. This states the architectural
> rule; you make the measurement.

---

### 10. (Reasoning) Why was Lesson 05's UART restriction not an MPU example?

Because **the MPU had never been enabled**. Lesson 05 firmware never touched
`MPU->CTRL`; measurement confirms this (`MPU_CTRL = 0x00000000` in Lesson 06,
section `[1]`).

The restriction came from the SoC's **Peripheral Protection Controller**.
In QEMU v8.2.2's `hw/misc/tz-ppc.c`, `tz_ppc_check()` blocks a “usermode”
access if that port's `cfg_ap` bit is zero; with `cfg_sec_resp = 0`, a read
returns `0`.

The distinguishing sign is the **difference in behavior**:

| | PPC (Lesson 05) | MPU (Lesson 06) |
|---|---|---|
| Violation outcome | Silent: `UART_BAUDDIV_UNPRIV = 0` | Exception: `IPSR = 4` |
| Fault address | None | `MMFAR = 0x30000000` |
| Location | Outside the processor | Inside the processor |

Practical rule: for unexpected access failure, ask **which block** rejected
it. An exception points to the MPU; silent incorrect data may indicate a
system-level controller.

---

### 11. (Reasoning) Can the MPU replace TrustZone?

No — they act on **different axes**.

| | MPU | TrustZone |
|---|---|---|
| Axis | Privilege (privileged / unprivileged) | Security state (Secure / Non-secure) |
| Configured by | Privileged software | Secure software (SAU) |
| Trusts | Correctness of the privileged layer | Correctness of Secure software |
| Protects against | Unprivileged code | **Including privileged Non-secure code** |

**Threat model where the MPU suffices:** the attacker can execute only
unprivileged code — a sandboxed plugin, a task parsing network input, or a
user application. As long as the privileged layer is trusted, the MPU is
a real security boundary.

**Threat model where the MPU does not suffice:** the attacker can reach
privileged code (bootloader, driver, RTOS kernel — all privileged). Privileged
code configures the MPU; an attacker reaching it can rewrite the policy.
It also does not suffice if the device is physically accessible or
certification requires a boundary independent of Non-secure software.

They are used together: TrustZone establishes the Secure/Non-secure boundary,
and the MPU isolates tasks within each side. Armv8-M already has separate
`MPU` and `MPU_NS` banks.

---

### 12. (Reasoning) Does enabling the MPU automatically make software secure?

No. The MPU is **a tool**, not a policy. Protection is only as correct as
the policy you write.

Mistakes that can silently leave everything open:

- An overly broad `LIMIT` accidentally includes protected data in a permitted region.
- Omitted `XN` permits execution from a data region.
- `RW` instead of `RO` makes an intended read-only region writable.
- Region priority: Armv8-M regions **must not overlap**; overlap is undefined behavior.
- Too few regions — this target has 8, quickly consumed by peripherals and
  per-task stacks in a real system.
- 32-byte granularity: you cannot protect “one variable”; its neighbors
  enter the same region. The `~0x1Fu` mask in this lesson demonstrates exactly this.

The MPU also **cannot see** logic errors, incorrect data written into permitted
regions, timing errors, or side-channel attacks.

The accurate statement is: *“A correctly configured MPU provides a hardware
boundary against a particular threat model.”* Not *“the MPU is on, so we are secure.”*

---

## References

- Armv8-M ARM (Arm DDI 0553) — PMSAv8, `MPU_RBAR`/`MPU_RLAR`/`MPU_CTRL`,
  `MMFSR`, `MMFAR`, `SHCSR.MEMFAULTENA`
- Cortex-M33 Devices Generic User Guide (Arm 100235)
- CMSIS-Core Armv8-M MPU API — `external/CMSIS/Core/Include/m-profile/armv8m_mpu.h`
- QEMU v8.2.2 `hw/misc/tz-ppc.c` — Lesson 05 PPC observation

---

[← Lesson 06](../README.md) · [Lesson 07 →](../../07-exceptions/)
