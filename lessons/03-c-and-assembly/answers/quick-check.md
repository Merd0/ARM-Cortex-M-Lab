**Language:** English | [Türkçe](quick-check.tr.md)

# Lesson 03 — Quick Check Answers

> Write your own answer before opening this.

---

### 1. What is the difference between `BL` and `B`? What happens with `B lr` instead of `BX lr`?

`B` only branches. `BL` branches **and** writes the return address to `lr` —
the address of the next instruction, with the Thumb bit set. In one phrase:
`BL` leaves a return path; `B` does not.

`B lr` does not branch to the address in the register — `B` takes an immediate
offset encoded in the instruction, not a register operand. Branching to an
address in a register requires `BX` (or `BLX`).

In the experiment performed in this repository, the GNU assembler **does not
reject** `b lr`. It reads `lr` as a **label name**, not a register, and produces
a branch to an undefined symbol:

```text
$ arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -c tb.S -o tb.o     # no error
$ arm-none-eabi-objdump -d tb.o
   0:   f7ff bffe       b.w     0 <lr>
$ arm-none-eabi-nm tb.o
         U lr                                                    # undefined!
```

The link-stage result is `undefined reference to 'lr'`. The error therefore
appears in the linker, not the assembler, and initially looks puzzling. This is
a small example of why "the assembler accepted it, so it is correct" is dangerous.

The distinction also appears in compiler output: `B` is generally used for loops
and `if` branches within a function, `BL` for calls. When you see `BL` in
disassembly, read it as "a function is called here, and `lr` changed."

---

### 2. Why does `lr` matter? `add_numbers` does not push it, but `accumulate` does — why?

`lr` is the function's **only** return path. If lost, there is nowhere to return.

- `add_numbers` is a **leaf function**: it calls no other function. Nobody changes
  `lr` while it runs, so it need not save it.
- `accumulate` executes `bl helper`. **By definition**, `BL` overwrites `lr` with
  a new value. If `accumulate` had not pushed its own `lr` first, the return address of `accumulate`
  would be lost after `helper` returned.

The distinction is immediately visible in disassembly:

```text
add_numbers:   adds r0, r0, r1 / bx lr           <- push yok
accumulate:    push {r4, lr} / ... / pop {r4, pc} <- push var
```

`pop {r4, pc}` also contains a small elegance: rather than restoring `lr` and
then executing `bx lr`, it writes the saved return address directly into `pc`.
One instruction performs both restoration and return.

---

### 3. Which registers hold two `int` arguments? Where does a fifth argument go?

The first four use `r0`, `r1`, `r2`, `r3`, in order. Thus two `int` arguments use
`r0` and `r1`.

The fifth and subsequent arguments go on the **stack**. AAPCS32 defines this as
an allocation algorithm: arguments are assigned to registers in order; once
registers are exhausted, the remainder is passed in memory on the stack.

Practical consequence: arguments beyond four are not free. Each extra argument
means at least one `str` (write) and one `ldr` (read) in the callee. In a function
called from a hot loop, this can become measurable. It is one reason passing a
struct pointer is common instead of passing many individual arguments.

Note that this count is in **words**. Even `char` occupies a full register;
`int64_t`, by contrast, uses two registers and introduces alignment rules.
Details are in Lesson 09.

---

### 4. Why can the return value be read from `r0`? Hardware or agreement?

**Agreement.** Nothing in hardware makes `r0` a "return-value register";
`r0` is no different from other general-purpose registers.

This is a contract (AAPCS32), and it works because both parties know the same
contract: the callee puts the result in `r0`, the caller reads it from `r0`.
Since hardware does not enforce it, code that violates the contract still
**builds without trouble** — it simply behaves incorrectly.

The practical reason for choosing `r0`: it already holds the first argument.
Many functions process that argument to produce their result, so the result
is often already in `r0`, without an extra `mov`. The `adds r0, r0, r1` in
`add_numbers` demonstrates exactly this: argument and result occupy the same place.

---

### 5. (Reasoning) Why can handwritten assembly not ignore AAPCS?

"My function, my choice of registers" breaks down in two places.

**First: you are not the one calling your function.** The compiler generated
the calling C code according to AAPCS. It placed arguments in `r0-r3` and will
read the result from `r0`. If your function reads elsewhere, you get no error
message — just garbage.

**Second: your function does not live alone.** The linker places your object
file alongside compiler-generated ones. Following the same contract is the
prerequisite for those objects to work together. ABI is precisely the answer
to "how do separately compiled pieces cooperate?"

More subtly, the contract is **bidirectional**. You may use `r0-r3` freely
(the caller already considers them lost), but must return `r4-r8`, `r10`,
`r11`, and `SP` unchanged. The role of `r9` is **platform-specific**: it may
be a static base (SB), thread register (TR), or an additional preserved register
(v6) — the platform standard defines which. This is why `r9` is kept separate
from both lists. Break It violated the second direction.

A small repair note: the original `accumulate` saved `a` in `r4` but left `b`
in `r1`. It **worked** because the existing `helper()` did not touch `r1` —
but that was a hidden local contract, not an AAPCS guarantee. Adding one line
that clobbered `r1` in `helper` would silently produce a wrong result. Both
values are now in preserved registers.

AAPCS is not a style preference but an **interoperability requirement**.
The only exception is entirely self-contained assembly neither called by C
nor calling C — something almost never found in real projects.

---

### 6. (Reasoning) Which is more dangerous: HardFault or a silent wrong result?

A defensible answer: **the silent wrong result**, because of diagnosability.

A HardFault is noisy. It tells you exactly where it happened, explains the cause
through `CFSR`/`HFSR`, and gives a location through `PC`. It appears during
development, in tests, and can be caught by a watchdog even in the field.
A bad day, but a **visible** one.

A silent wrong result does none of this:

- It may pass tests, because no symptom appears if the corrupted register is not
  live on a particular code path.
- It may appear or disappear when the optimization level or unrelated code changes
  — the "sometimes it happens" class of bugs.
- Its symptom appears **in a different function** from the error. In Break It,
  `helper` made the mistake, `accumulate` produced the wrong result, and `main`
  printed the complaint. Three places.

The counterargument is legitimate too: in a safety-critical system, HardFault
may mean an uncontrolled stop and therefore a hazard, while a slightly incorrect
result may be tolerated. Which is worse depends on the system's **safe state**.

The practical answer is therefore to make silent errors noisy. Assertions, the
MPU, stack-limit checks, static analysis, and compiler warnings all serve that
purpose — exposing faults early and **loudly**.

---

## References

- AAPCS32 (ARM-software/abi-aa) — argument allocation, registers that must be preserved
- Armv8-M ARM (Arm DDI 0553) — `B`, `BL`, `BX` instruction definitions
- [`../README.md`](../README.md) — the lesson itself

---

[← Lesson 03](../README.md) · [Lesson 04 →](../../04-stack-msp-psp/)
