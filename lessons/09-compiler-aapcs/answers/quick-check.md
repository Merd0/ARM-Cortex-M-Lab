**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 09 — Quick Check Answers

> Write your own answer before opening this file.

---

### 1. What are `r0`–`r3` used for, and why might they not be preserved?

**ABI:** They carry initial arguments (a1–a4), return results through `r0`
(or `r0:r1` for 64 bits), and may serve as scratch between calls. AAPCS32
makes them **caller-clobbered**: the called function may freely overwrite
them. If a value is needed after a call, the caller must keep it elsewhere
(in a callee-saved register or on the stack).

The distinction matters: **caller-clobbered** means the callee is **allowed**
to change them and the caller **cannot rely on** them — not that every callee
actually changes them.

Measurement (GDB stops C and D, three presets): `helper_scale` changed
**only `r0`**; `r1`–`r3` were identical before and after `BL`. This is an
observation; relying on it would still violate the ABI.

---

### 2. Caller-saved and callee-saved: who is responsible for what?

| | Caller | Callee |
|---|---|---|
| **caller-clobbered** (r0–r3, r12) | Preserve if needed | Use freely |
| **callee-saved** (r4–r8, r10, r11; r9 if v6) | Do nothing | Save and restore if used |

The reason is a division of labor: the caller need not save anything for
short-lived values; for long-lived values, the callee saves only the registers
it **actually uses**.

---

### 3. (Reasoning) Why must a callee restore r4 even when its result is correct?

Because **r4 belongs to the caller**. Trusting AAPCS, the caller may keep a
value in r4 across the call. The callee's result is only part of this contract;
clobbering r4 corrupts **another variable** of the caller, and the failure
appears **later**, not in the call itself.

Break It measured this: `bad_callee` returned the right result
(`CHECK_RESULT PASS`), while the harness detected r4 corruption
(`CHECK_CALLEE_SAVED FAIL`, `CORRUPTED r4`).

If called directly from C, whether the effect became visible would depend
on whether the caller was using r4 at that moment — register allocation.
A correct result is not a correct contract.

---

### 4. What can `r12` (IP) be used for?

**ABI:** Intra-procedure-call scratch register; caller-clobbered. AAPCS32,
*Use of IP by the linker*: a linker-inserted **veneer** (for example, an
intermediate code fragment for an out-of-range branch) may clobber r12;
a conforming program must assume this can happen at any eligible branch.

This lesson used r12 in two places: probes used it as a `g_probe` pointer
(because it carries no argument), and the harness used it as the `blx`
target. Veneers are covered in Lesson 10.

---

### 5. What happens to LR in a function that calls another function?

`BL` writes the return address to LR — the inner call overwrites the
function's **own** return address. A non-leaf function must therefore save
LR **before the inner call**.

In `abi_chain`: `push {r4, lr}` … `bl helper_scale` … `pop {r4, pc}`.
At GDB stop C, `[SP+4] = 0x100004eb` on the stack — saved LR.
`pop {…, pc}` returns by loading that value directly into PC.

Exception: a tail call (question 10) — LR is never saved there, because the
function delegates its return to the called function.

---

### 6. (Reasoning) What does “SP mod 8 = 0 at a public interface” mean?

**ABI:** AAPCS32 requires SP to be a multiple of 8 at an externally callable
function boundary (at `BL` and function entry). The rule that applies at
all times is only SP mod 4 = 0.

Three separate rules:

| Rule | Layer |
|---|---|
| SP mod 8 = 0 at a call boundary | **ABI** |
| Alignment required by accesses such as `LDRD`/`STRD` | **ARCHITECTURE** |
| Frame alignment on exception entry (xPSR bit 9) | **ARCHITECTURE** |

A misaligned SP **alone** does not cause an `LDRD` fault; what breaks is the
ABI assumption. And the rule applies at call boundaries: at `-Og`,
`asm_clobber_good` only executes `push {r4}` (4 bytes), leaving SP mod 8 = 4
inside — legal because it calls nothing.

Call boundaries measured here: `abi_chain` at `BL`, `0x30007FD8`; the harness
at `BLX`, `0x30007FB8` — both mod 8 = 0. Exercise starter: `0x30007FAC` —
mod 8 = 4 → `CHECK_SP_ALIGNED_AT_CALL FAIL`.

---

### 7. (Reasoning) For `f(u32 a, u32 b, u32 c, u64 d)`, where does `d` go?

**ABI (AAPCS32 rules):**

1. `a`, `b`, `c` → r0, r1, r2. NCRN = r3.
2. `d` requires 8-byte alignment → **C.3**: NCRN rounds up to the next
   even-numbered register, r4.
3. r4 is not an argument register → `d` does not fit in registers (C.4
   fails; C.5's NCRN < r4 condition is not met).
4. **C.6–C.8:** NCRN = r4, NSAA rounds up to 8 bytes, and `d` goes on the stack.

**OBSERVATION (three presets):** `[SP+0] = 0xDDEEFF00` (low word),
`[SP+4] = 0x99AABBCC` (high word), SP mod 8 = 0.

**About r3:** it carries no argument; its contents are **unspecified**.
Measurement showed this clearly: in `debug` and `minsize`, r3 happened to
contain `0x99AABBCC` (d's high word — GCC temporarily held it in r3 while
writing it to the stack); in `release`, `0x00000000`. The lesson therefore
makes **no decision** based on r3; the check positively verifies only that
`d` is in the correct place.

---

### 8. What does `volatile` guarantee?

**It guarantees:** preserving each program access to the object, and accessing
the object as a whole (AAPCS32 *Volatile Data Types*). The compiler cannot
cache the value in a register to avoid reading it again.

Measurement: `vol_store_twice` has **two** `str` in all three presets;
`dead_store` (non-volatile) has **one** `str` at `-O2`/`-Os`. `poll_vol`
rereads on every iteration; `poll_plain` at `-O2` reads once and enters an
infinite loop.

**It does not guarantee:**

- **Atomicity** — `g_vol++` still consists of separate load and store.
- **Mutual exclusion** — two contexts can write simultaneously.
- **Ordering of non-volatile accesses** — it is not a memory barrier (`DMB`/`DSB`).
- Thread safety, cache coherency, memory protection.

AAPCS32 also permits the compiler to ignore the `volatile` qualification of
an automatic variable **whose address is never taken** (if the function does
not call `setjmp()`). This is permission; it was not measured here, and the
fixtures do not rely on it.

---

### 9. What is a leaf function? Can it use the stack?

**A leaf function calls no other function.** Whether it uses the stack is
a separate question.

`leaf_with_stack` calls nothing — no `bl` in any of the three presets —
but allocates stack space:

```text
-Og:  sub sp, #16 ... add sp, #16 ; bx lr
-O2:  push {r4} ; sub sp, #20 ... add sp, #20 ; pop {r4} ; bx lr
-Os:  sub sp, #16 ... add sp, #16 ; bx lr
```

LR is not saved because there is no `BL` to overwrite it. The array is in
memory because its address is passed to an empty inline asm
(`"r"(buf)` + `"memory"`) — not a call.

---

### 10. (Reasoning) A function ends with `b helper` — is the ABI violated?

**Not necessarily.** This may be a **tail call**: if the function's last task
is calling another function and returning its result unchanged, the compiler
may generate `b` directly instead of `bl` + return. LR is untouched; the
called function returns directly to the **original caller**.

Measurement: at `-O2`/`-Os`, `tail_call` is `adds r0, #1 ; b.w helper_ext` —
no saved LR, no push. At `-Og`, it is
`push {r3, lr} ; bl helper_ext ; pop {r3, pc}`.

This is a **COMPILER** optimization (`-foptimize-sibling-calls`), not an ABI
rule or a guarantee. It does not violate the contract: the return value in
r0 reaches the correct destination, and callee-saved registers are preserved.

---

### 11. (Reasoning) `-O2` kept a value in r3 across a call — an ABI rule?

**No.** r3 is caller-clobbered; AAPCS **does not permit** the caller to rely
on it being preserved.

**Compiler knowledge** makes this possible: with `-fipa-ra` (enabled at
`-O2`/`-Os`), GCC knows which registers a callee in the same translation
unit actually clobbers, and can use the others across the call. This
optimization depends on **seeing inside** the callee.

That knowledge is unavailable for a separately compiled function or
handwritten assembly — hence `helper_scale` deliberately lives in a
separate TU, and the harness does not depend on a C caller.

---

### 12. Why does `(x + 1) > x` fold to `return 1` for `int`?

Signed integer overflow is **undefined behavior** in C (ISO C11 §6.5 ¶5).
The compiler may assume overflow does not occur; then `x + 1 > x` is always true.

Unsigned arithmetic, however, is **defined** modularly: for `x = UINT_MAX`,
`x + 1 = 0`, and the result is false. The compiler cannot fold it.

Measurement:

```text
signed_gt:    movs r0, #1 ; bx lr                 (-Og, -O2, -Os)
unsigned_gt:  adds r0, #1 ; it ne ; movne r0, #1   (-Og, -O2, -Os)
```

Folding occurred even at `-Og`. The same principle as Lesson 03's correction:
wrapping `ADDS` in assembly **is not** the behavior of overflowing signed C.
That is why `signed_gt` is never executed, only inspected in disassembly.

---

### 13. (Reasoning) Why must inline asm writing r4 declare a clobber?

Inline asm has **two contracts**:

1. **Instruction semantics** (processor): `mov r4, %0` writes r4. This is
   identical and correct in both cases.
2. **Constraint/clobber contract** (compiler): GCC **does not read** the asm
   text. It learns which registers change only from the clobber list.

Measurement (`asm_clobber_bad` in three presets, `asm_clobber_good` at `-Og`/`-O2`):

```text
asm_clobber_bad:   mov r4, r0 ; bx lr                        <- r4 not saved
asm_clobber_good:  push {r4} ; mov r4, r0 ; pop {r4} ; bx lr <- saved
```

At `-Os`, `asm_clobber_good` uses `push {r4, lr}` … `pop {r4, pc}`;
r4 is preserved there too (README, Experiment 1).

The version without a clobber is an **AAPCS violation** against its caller —
the same mistake as `bad_callee`, this time produced by the compiler. Whether
it becomes visible again depends on register allocation. That is why
`asm_clobber_bad` is never executed.

---

## References

- AAPCS32 (release 2025Q4) — *Core registers*, *Stack constraints at a public
  interface*, *Use of IP by the linker*, *Result Return*, *Parameter Passing*
  (C.3–C.8), *Volatile Data Types*
- ISO C11 §6.5 ¶5, §6.7.3
- GCC manual — *Options That Control Optimization*, *Extended Asm*

---

[← Lesson 09](../README.md) · [Lesson 10 →](../../10-linker-elf-memory/)
