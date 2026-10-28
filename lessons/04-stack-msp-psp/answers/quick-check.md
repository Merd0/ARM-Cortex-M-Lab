**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 04 — Quick Check Answers

> Write your own answer before opening this.

---

### 1. What does full-descending stack mean? What does each word say?

Two pieces of information combined in one term:

| Word | What it says |
|---|---|
| **full** | `SP` points to the last **occupied** slot written, not the next empty one |
| **descending** | The stack grows downwards from high addresses; `PUSH` decreases `SP` |

Alternatives exist (such as empty-ascending), and some architectures use them.
On Cortex-M, the architecture fixes the choice: full-descending.

On an empty stack, `SP` points **one past** the usable area — it points to no
valid data at that moment. "SP always points to meaningful data" is **false**;
it holds only when the stack is not empty.

The area below `SP` is also allocated, but its contents are undefined: remnants
of previously pushed and popped data.

---

### 2. How far does `PUSH {r4, r5}` move `SP`? Which register is at the lower address?

`SP` **decreases by 8 bytes** (two registers × 4). Observable result:

```text
PUSH {r4, r5}:   new SP = old SP - 8
                 [new SP + 0] = r4      <- lower-numbered register, lower address
                 [new SP + 4] = r5

POP  {r4, r5}:   r4 = [SP + 0]
                 r5 = [SP + 4]
                 new SP = old SP + 8
```

Rule: **lowest-numbered register at the lowest address**. Writing the list in
another order (`{r5, r4}`) changes nothing; the assembler sorts it anyway.

The architecture defines this as an *effective-address* calculation. "SP goes
down first, then data is written" is a rough description of the net effect,
not a micro-operation guarantee. The final state above is what you can rely on.

This determines the initial address to put into `SP` when setting up a stack.
For `uint32_t stack[128]`, the correct top is `&stack[128]` — **one past the
array**. The first `PUSH` subtracts 4 and writes to `&stack[127]`; it stays
inside the array.

With `&stack[127]`, the first `PUSH` would write to `&stack[126]` — **inside**
the array, not above it. There would be no overflow, but two losses. First,
capacity: the last element (`stack[127]`) would never be used. Second, more
importantly, **alignment**: if `&stack[128]` is 8-byte aligned, `&stack[127]`
is not, giving `SP % 8 == 4` at the call boundary — an AAPCS violation.

The truly dangerous error is writing `&stack[0]` (the bottom): the first
`PUSH` writes **below** the array and overwrites neighboring variables.

---

### 3. Why does writing `PSP` not make it the active stack?

Because these answer two separate questions:

| Question | Where the answer is held |
|---|---|
| What are PSP's **contents**? | The `PSP` register |
| Which register does `SP` **refer to**? | `CONTROL.SPSEL` (bit 1) |

`__set_PSP(x)` changes the first, not the second. `CONTROL.SPSEL` makes the
active selection.

We measured this in the lesson: in stage 2, `$psp = 0x30000200`, but `$sp`
still equals `$msp` (`0x30007fd0`), and local-variable addresses did not change.

The practical consequence is a **mandatory** order:

1. Write a valid address to `PSP` first.
2. Then set `SPSEL` to 1.

Reversing them makes `SP` point to zero-valued (or garbage) `PSP` as soon as
`SPSEL = 1`, and the next `PUSH` attempts an invalid write. This was exactly
the Break It scenario in Lesson 02.

---

### 4. Which stack does Thread Mode use after reset? How do you know without writing code?

**MSP.**

Two ways to know without code:

1. **The reset value of `CONTROL`.** The architecture specifies `CONTROL = 0`
   after reset. `SPSEL` is bit 1, so 0 → MSP.
2. **The vector table's first entry.** At reset, hardware reads entry 0 (the
   word at offset `0x00000000`) and loads it into **MSP**. PSP has no such
   mechanism, because PSP is not used during reset.

We measured the second in Lesson 02: `vector[0] = 0x30008000`, and at reset
`MSP = 0x30008000`.

This is logical: setting up PSP is software's responsibility, and no software
has executed yet at reset. The system must start with a stack, and that is MSP.

---

### 5. An exception occurs while Thread Mode uses PSP: which SP does Handler use, and where does hardware write?

**Two different answers; confusing them is common.**

| Question | Answer |
|---|---|
| Where is the hardware-pushed state (exception frame) written? | **PSP** — the interrupted code's stack |
| On which SP does the handler run? | **MSP** — because entry clears `SPSEL` |

The processor first writes the interrupted context to the **currently active**
stack (PSP here), then enters Handler Mode and clears `SPSEL`. The frame remains
on PSP while the handler runs on MSP.

We measured `Stack: MSP (0x30007fd0)` and `$control = 0` at the fault, although
`SPSEL = 1` just before it. Software did not clear `SPSEL`; **hardware** did.

The reason: handler code must not depend on a faulty task's stack. If the task
stack overflows or is corrupt, the handler can still run on a clean MSP.

Two cautions:

**1. Attempting a write and completing it are different.** If PSP is invalid
or crosses `PSPLIM`, stacking itself faults (`BFSR.STKERR`, `UFSR.STKOF`,
`MMFSR.MSTKERR`). We measured both situations in this lesson. This is why
`cm-frame <adres> <EXC_RETURN>` **rejects** decoding when these bits appear:
memory may contain no valid frame. Acceptance by the command does not prove
the frame is real either; it is a limited decoder that filters invalid cases.

**2. This explanation assumes one security state.** With TrustZone, stack
selection also depends on the security-state transition (Lesson 16).

---

### 6. (Reasoning) Must every C function use the stack?

**No.** We measured two contrasting examples together.

Uses the stack (`frame_using_function`):

```text
10000070:  b086    sub   sp, #24
10000074:  9305    str   r3, [sp, #20]
```

Never touches the stack (`active_stack_name`):

```text
10000054:  f3ef 8314   mrs   r3, CONTROL
10000058:  f013 0f02   tst.w r3, #2
1000005e:  4802        ldr   r0, [pc, #8]
10000060:  4770        bx    lr
```

The second contains neither `push` nor `sub sp`. It operates entirely in registers.

Three situations require a function to touch the stack:

1. **Calling another function** — `BL` overwrites `lr`, which must be saved.
2. **Using callee-saved registers** — `r4-r8`, `r10`, `r11` must be returned
   as received (Lesson 03).
3. **Local state does not fit in registers** — the excess spills to the stack.

If none applies, the stack is untouched.

Important qualification: this holds for **this compiler, source, and optimization
level**. A variable spilled under `-Og` may stay entirely in registers under
`-O2`; another compiler may emit entirely different code. Answer "does this
function use the stack?" from **generated machine code, not source**.
The right reflex is `objdump`.

Terminology: a **leaf function** *calls no other function* — it does not mean
"uses no stack." A leaf can allocate locals using `sub sp, #N`;
`frame_using_function` does exactly that.

---

### 7. (Reasoning) Why can 8-byte alignment matter?

AAPCS32 requires `SP` to be 8-byte aligned at a public interface boundary —
an ordinary function call. The reasons have several layers:

**0. First distinguish two rules.** This is an **ABI** (calling-contract) rule,
not an instruction-alignment rule. Cortex-M33 `LDRD`/`STRD` require **word
(4-byte)** alignment, not 8 bytes. Confusing the two is common — the first
version of this lesson did so too.

**1. Data types and structures.** Types requiring 8-byte alignment, such as
`double` / `int64_t`, reside on the stack. If `SP` is not 8-aligned at the call
boundary, the compiler's layout assumptions break.

**2. ABI interoperability.** The compiler generates code **assuming** the
callee's `SP` is 8-aligned, calculating offsets accordingly. Violating that
assumption in handwritten assembly lies to compiler-generated code, causing
silent misbehavior like the callee-saved violation in Lesson 03.

**3. Exception stacking.** On exception entry, Cortex-M inserts a padding word
if necessary to align the frame, recording this with a bit in `xPSR`.
Hardware, not just your code, cares about alignment.

What happens at a call boundary with `SP % 8 == 4`? **Expecting an immediate
instruction fault is wrong.** `LDRD`/`STRD` require word alignment, and
`SP % 8 == 4` is still word-aligned, so this alone does not cause an
`LDRD`/`STRD` fault. The violated property is an **ABI assumption**: the callee
calculates local layout assuming an 8-aligned `SP`. Possible outcomes include
misalignment of a type requiring 8 alignment, unexpected behavior of compiler-
generated offsets, or **nothing at all**. The last is most dangerous: tests
pass, and the problem appears only with another compiler version or code path.

Practical rule: have `SP % 8 == 0` on entry to a `bl` call. The easiest way is
to keep total prologue bytes a multiple of 8, explaining the apparently
unnecessary extra register often pushed in compiler prologues. `accumulate`
(Lesson 03) and the `psp_start` target `thread_entry` use this method here.

This does **not** mean **each individual `PUSH`** must contain an even number
of registers. The rule concerns net `SP` at the call boundary; pushing one
register temporarily and balancing it later is allowed.

---

### 8. (Reasoning) What breaks if you change `CONTROL.SPSEL` midway through C?

The compiler allocated that function's frame **on MSP** and emitted accesses
to locals using `[sp, #offset]`. When `SPSEL` changes, `SP` refers to another
region — **the same offsets refer to entirely different addresses**.

Writing `CONTROL`:

- **does not move** the existing frame to the new stack,
- **does not create** a frame on the new stack,
- **does not notify** the compiler.

The remaining function therefore writes **above** the new stack's top or to
arbitrary locations. Exactly this happened in the first version: under `-O2`
and `-Os`, the word just beyond `task_stack` was overwritten and `CHECK_REGION`
reported FAIL.

**Why not `volatile`?** `volatile` prevents accesses from being optimized away;
it does not change *which base register* is used to calculate them. Offsets
remain relative to `sp`.

**Why not an extra `ISB`?** `ISB` solves a **synchronization** issue: ensuring
subsequent instructions see the context change. This is a **frame-validity**
problem, not synchronization. The `__set_CONTROL()` wrapper already executes
`ISB` internally — synchronization existed while the bug still occurred.

**Why not `noinline`?** Keeping the function separate does not change the fact
that its frame remains on MSP.

The correct solution is to transition at a point without compiler frame
assumptions (a small assembly fragment), then branch to a **fresh C function**.
That function creates its own prologue on the new `SP`.

One more detail: updating `CONTROL` with read–modify–write preserves bit 0
(`nPRIV`). This does **not** mean unprivileged code can regain privilege by
writing `2` to `CONTROL`. Unprivileged Thread Mode cannot clear its own `nPRIV`;
elevation requires a privileged exception path, such as an SVC handler.
That is Lesson 05's topic.

---

## References

- Armv8-M Architecture Reference Manual (Arm DDI 0553) — stack-pointer
  banking, `CONTROL.SPSEL`, exception-entry behavior, `PSPLIM`/`MSPLIM`
- Cortex-M33 Devices Generic User Guide (Arm 100235) — `CONTROL`, MSP/PSP
- AAPCS32 (ARM-software/abi-aa) — 8-byte stack alignment
- CMSIS-Core (M) — `__set_PSP`, `__set_CONTROL` (executes `ISB` internally), `__ISB`

---

[← Lesson 04](../README.md) · [Lesson 05 →](../../05-privilege-svc/)
