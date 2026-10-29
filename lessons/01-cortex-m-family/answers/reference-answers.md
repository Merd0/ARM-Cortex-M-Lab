**Language:** English | [Türkçe](reference-answers.tr.md)

# Lesson 01 — Reference Answers

> Write your own answer first. Open this file afterwards.
>
> The scenarios have **no single correct answer**. The following answers address
> "is this justification defensible?", not "this is the correct choice." If you
> chose differently with sound reasoning, your answer is correct.

---

# Part 1 — Three scenarios

## Scenario A — Secure Smart Lock

**Defensible choice:** M33 (or M23, subject to the condition below).

**Primary justification — with its assumptions.** TrustZone has a strong case if
all three hold: (a) the device holds a persistent secret, (b) it executes code
that can be updated in the field, (c) the threat model assumes "one day the
application layer will contain a vulnerability." Under these conditions,
TrustZone removes the key entirely from the Non-secure side's address space.

If the conditions change, so does the answer: if code is not field-updatable,
the secret resides in an external secure element, or certification is not required,
an MPU + a well-separated privileged layer is defensible. "TrustZone is mandatory"
cannot be stated unconditionally.

**Would the MPU suffice?** Partly. An MPU can prevent unprivileged code from
accessing a region. But privileged code configures the MPU; a vulnerability in
the privileged layer (where the bootloader, drivers, and RTOS all reside) can
allow MPU configuration to change. With TrustZone, Secure memory is **not
addressable at all** from the Non-secure side — even privileged Non-secure code
cannot see it.

**What you sacrificed:** Part cost, complexity, the burden of building two separate
images (secure + non-secure), and longer development time.

**When M23 is sufficient:** When there is no heavy crypto and DSP/FPU are not needed.
M23 (Armv8-M Baseline) carries TrustZone, but its performance and instruction set
are more limited than M33's. With a crypto accelerator as a separate peripheral,
M23 is a very reasonable choice.

**Regret scenario 2 years later:** If the product must pass an additional security
certification and the selected part lacks the Security Extension, software cannot
compensate. The silicon must change.

---

## Scenario B — Low-Power Wearable

**Defensible choice:** M0+ or M23.

**Primary justification:** The power budget dominates every other criterion.
Step counting, thresholding, and moving averages can comfortably use fixed-point
arithmetic. An FPU's contribution here is marginal; its cost persists (area,
leakage current, more gates).

**Is an FPU really necessary?** Acceleration data already arrives as integers.
Converting it to floating point is usually a **convenience**, not a necessity.
Working in Q15/Q31 fixed-point format takes somewhat more effort, but is
deterministic and inexpensive.

**What you lose by selecting M0+:**
- No MPU (optional in M0+ and absent in many inexpensive parts) → no task isolation
- Limited atomic-operation support (no LDREX/STREX) → lock-free structures become harder
- Fewer debug facilities (DWT/ITM limited or absent)
- No conveniences such as bit-banding or saturating arithmetic

**Regret scenario 2 years later:** If the product starts storing "health data"
(heart rhythm, sleep), data-protection regulations enter the picture, and the
absence of TrustZone can suddenly force migration to M23/M33. This means
revalidating much of the software.

**Middle ground:** M23 brings TrustZone and better debug at very little additional
power cost over M0+. If the product roadmap is uncertain, M23 is insurance.

---

## Scenario C — Robotics / Signal Processing Node

**Defensible choice:** M33 (configured with FPU + DSP) or M7.

**Primary justification:** The control loop is hard real-time. The critical metric
is not average speed but **worst-case latency**. Because kinematics and filtering
are floating-point-intensive, the FPU has **a strong justification** here — but
conditionally: the mathematics must require floating point, a fixed-point
implementation must be unable to meet the loop budget, and error-analysis cost
must be unacceptable. Without measuring these three, "an FPU is mandatory" is
unjustified; fixed-point (Q15/Q31) is sufficient for many control loops.

**A cached M7 is faster; why not choose it?** Because cache **harms determinism**.
The same code can take multiples of the time depending on cache hits versus misses.
The average improves, but bounding the worst case becomes harder. In a hard
real-time system, measure the ceiling, not the average. Using M7 requires cache
lockdown for critical regions or TCM — additional engineering.

**How to defend the choice through measurement:** Sample control-loop execution
time over many iterations and report the **observed maximum**, not the average.
Define headroom relative to the deadline. Also note that this is **not** a proven
WCET bound. You cannot make this measurement in QEMU — the cycle counter does not
work (`docs/TARGET_CAPABILITIES.md`). Real timing requires real hardware + DWT/trace.
FVPs are not an automatic solution either: an ordinary FVP is a **functional**
model and does not provide cycle counts; timing information comes only from models
explicitly documented as "cycle-approximate." Verify what each model offers in
its product documentation.

**Is MVE necessary?** Probably not. At the scale of sensor fusion and PID, DSP
instructions (single-cycle MAC, SIMD) are usually sufficient. MVE becomes relevant
at the scale of ML inference or intensive multichannel filtering.

**Security:** The answer changes if the robot is network-connected. A joint
controller receiving remote commands needs firmware signatures and secure boot;
at that point, TrustZone returns to the discussion.

---

# Part 2 — Quick Check answers

### 1. What is the difference between TrustZone and the MPU? Can they replace each other?

**No — but not because "one is reliability and the other is security."**
They establish different **boundaries**.

| | MPU | TrustZone (Security Extension) |
|---|---|---|
| What it separates | Privileged ↔ unprivileged access | Secure ↔ Non-secure **state** |
| What it protects against | Unprivileged software, whether faulty or hostile | The **entire** Non-secure side, including privileged code |
| Who configures it | Privileged software | Secure software (SAU) |
| What must be trusted | Correctness of the privileged layer | Correctness of Secure software |

An MPU **can be a real security boundary**: as long as the privileged layer is
trusted, it confines unprivileged code — even hostile code — to its own region.
For sandboxed plugins, a task parsing network data, or a user application, this
is security protection in the full sense.

TrustZone draws a different boundary: even **privileged** Non-secure code cannot
see Secure memory. This protection assumes **Secure software is trusted**; a bug
there or incorrect SAU configuration removes the protection.

Neither therefore **eliminates** the assumption of a "trusted software layer";
they place that layer differently. They are used together: Armv8-M already has
separate MPU_S and MPU_NS configurations.

---

### 2. An FPU exists, but firmware never uses floating point. What is the cost?

Three separate costs:

1. **Silicon cost** — area and leakage current are paid even when unused.
2. **Exception-latency risk — conditionally.** **Permitting** FP access with
   `CPACR` does not mean an extended frame is written on every exception.
   Permission and **active FP context** are separate; `CONTROL.FPCA` is decisive.
   Armv8-M also provides *lazy* FP state preservation (`FPCCR.LSPEN`): space is
   reserved, but registers are written only when actually needed. Cost therefore
   depends on actual FP use and preservation configuration; "you enabled the FPU,
   so every frame became 26 words" is too categorical.
3. **ABI and budget risk.** Executing an FP instruction without hardware FP access
   permission does **not automatically fall back to software emulation**; a fault
   (UsageFault, NOCP) occurs. Software FP routines are a **compile-and-link**
   decision: `-mfloat-abi=soft` makes the compiler turn FP operations into libgcc
   calls. The real risk is not "silent emulation," but **slow yet correct**
   software FP exceeding the budget — or link/runtime errors from mixing objects
   compiled with different FP ABIs.

This is exactly why this repository uses `-mfloat-abi=soft`: keeping the exception
frame at 8 words simplifies Lessons 07/08. The FPU will be enabled **deliberately**
in Lesson 13.

---

### 3. `MVFR1.MVE = 0` — simulator limitation or processor property?

A processor property. More precisely, **an architectural boundary**.

MVE (Helium) is a vector extension introduced with Armv8.1-M. Cortex-M33 implements
Armv8-M Mainline. No Cortex-M33 implementation therefore has MVE — it is not a
configuration option that can be enabled.

**How to distinguish them:** Consult two documents.
- *Armv8-M Architecture Reference Manual* → does this architecture contain the extension?
- *Cortex-M33 Technical Reference Manual / Integration and Implementation Manual*
  → if so, is it optional or mandatory in this core?

Compare: reading `MPU_REGIONS = 0` would indicate a **configuration** decision;
another M33 might report 8 or 16. `MVE = 0` is an architectural fact.

---

### 4. (Reasoning) Hard real-time loop: M7 or M33?

Scenario C covers this in detail. In summary:

M33 is the better choice **if** worst-case latency is more critical than average
performance. M7's cache improves the average but increases variance; variance
drives the decision in a hard real-time system.

Defense: measure the **maximum** duration over many loop iterations, not the
average. "M7 is 40% faster" means nothing without measuring the maximum.

**An observed maximum is not a proven WCET (worst-case execution time) bound.**
Measurement covers only the inputs and states you exercised; you may never have
triggered the worst path. Real WCET analysis requires static-analysis tools,
hardware models, and usually a certification process. The correct statement is
"the observed maximum was X and the budget was Y," not "WCET is X."

---

### 5. (Reasoning) "TrustZone is expensive; we can manage with the MPU" — what is correct and what is dangerous?

**The correct part:** An MPU provides real isolation and is sufficient in many
products. Cost concerns are legitimate. If the threat model is "protection from
our own bugs," the MPU is the right tool and TrustZone adds complexity.

**The dangerous part:** Confusing two different threat models. Privileged software
configures the MPU. If an attacker reaches privileged code (bootloader, a driver,
RTOS kernel — all privileged), they can reconfigure the MPU and remove protection.
The MPU thus assumes **the privileged layer is trusted**.

TrustZone removes this assumption: even privileged code on the non-secure side
cannot see secure memory.

**The right question:** "What is our threat model, and at what privilege level
can an attacker execute code?"

- If an attacker can execute only **unprivileged** code (sandboxed plugin, parser
  task, user application), the MPU is a real security boundary and may suffice.
- If an attacker can reach **privileged** code, the device is physically accessible,
  or certification requires a boundary independent of Non-secure software,
  the MPU alone is insufficient.

Both "the MPU is not security" and "the MPU solves everything" are wrong.
What matters is **where** the boundary is drawn and which layer is trusted.

---

## References

- Arm Cortex-M33 Technical Reference Manual — optional extensions, MPU/SAU
- Arm Cortex-M33 Integration and Implementation Manual — `FPU`, `DSP`,
  `SECEXT`, `MPU_S`, `MPU_NS` configuration parameters
- Armv8-M Architecture Reference Manual (Arm DDI 0553)
- [`docs/TARGET_CAPABILITIES.md`](../../../docs/TARGET_CAPABILITIES.md)

---

[← Lesson 01](../README.md)
