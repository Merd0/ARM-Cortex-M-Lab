**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — Activate PSP Correctly

Exercise: [`../../exercises/01-activate-psp/`](../../exercises/01-activate-psp/)

> This file was **rewritten** after independent audit. Its previous version
> described a solution that no longer exists (switching stacks with
> `activate_psp()` inside ordinary C) and contained two technical errors.
> Both are corrected below.

---

## Where the solution lives

The actual solution is **assembly, not C**:

```text
solutions/01-activate-psp/psp_start.S
```

```asm
psp_start:                      @ r0 = psp_top, r1 = entry  — DONMEZ
    msr     psp, r0             /* 1) PSP = yeni stack tepesi */

    mrs     r2, control
    orr     r2, r2, #2          /* 2) SPSEL = 1               */
    msr     control, r2
    isb                         /* 3) senkronize et           */

    ldr     lr, =psp_entry_trap /* entry donerse tuzak        */
    orr     lr, lr, #1
    bx      r1                  /* taze C baglamina dallan    */
```

The `main.c` side is unchanged: it initializes the stack block and guards,
then calls `psp_start(TASK_STACK_LIMIT, thread_entry)` and **never returns**.

Flow:

```text
C on MSP (main)
        |
        v  psp_start()  — a boundary without compiler frame assumptions
   PSP selected (msr psp + CONTROL.SPSEL + isb)
        |
        v  bx r1
FRESH C entry (thread_entry) — establishes its own prologue on the NEW SP
        |
        v
NEVER RETURN to the abandoned MSP frame
```

---

## Why is an assembly boundary mandatory?

The compiler allocates a C function's frame on MSP and emits local accesses
through `[sp, #offset]`. Changing `CONTROL.SPSEL` points `SP` at another region —
**the same offsets now refer to different addresses**.

Writing `CONTROL`:

- **does not move** the existing frame to the new stack,
- **does not create** a frame on the new stack,
- **does not notify** the compiler.

This is a measured bug, not a theoretical concern: the first version
transitioned inside ordinary C, and independent audit showed writes **one past**
`task_stack` under `-O2`/`-Os` (`CHECK_REGION FAIL`).

Know what **does not solve** it:

| Attempt | Why it is insufficient |
|---|---|
| `volatile` | Prevents removal of an access; does not change the base register — offsets still use `sp` |
| `noinline` | Keeping the function separate does not change its frame being on MSP |
| Extra `ISB` | Solves synchronization; the problem here is **frame validity** |
| Copying the frame | Pointers and saved addresses still refer to the old location |

The correct approach is to transition in code with no local state depending
on `SP`, then branch to a C function **from scratch**.

`psp_start` satisfies this — its disassembly contains no `[sp, #offset]` access:

```text
10000054 <psp_start>:
10000054:  f380 8809   msr   PSP, r0
10000058:  f3ef 8214   mrs   r2, CONTROL
1000005c:  f042 0202   orr.w r2, r2, #2
10000060:  f382 8814   msr   CONTROL, r2
10000064:  f3bf 8f6f   isb   sy
10000068:  f8df e008   ldr.w lr, [pc, #8]
1000006c:  f04e 0e01   orr.w lr, lr, #1
10000070:  4708        bx    r1
```

The target establishes its own prologue on the new `SP`:

```text
10000170 <thread_entry>:
10000170:  b500   push  {lr}
10000172:  b083   sub   sp, #12
```

`push {lr}` (4 bytes) + `sub sp, #12` = 16 bytes; 8-byte alignment remains
intact at the call boundary.

---

## Three steps, three reasons

### 1. `msr psp, r0` — why `&block[128]`?

The stack is full-descending: `PUSH` decreases `SP` and writes at the **new**
`SP`. The supplied initial address itself is therefore never written.

`&task_stack_block[128]` is one past the usable area; that address is not
accessed, only used as the initial value. All 128 words remain usable without
crossing the boundary.

- With `&block[127]`: no overflow, but lost capacity (the last word is never
  used) and **alignment** (`&block[128]` being 8-byte aligned means
  `&block[127]` is not; `SP % 8 == 4` at the call boundary violates AAPCS).
- With `&block[0]`: the first `PUSH` writes **below** the array and overwrites
  neighboring variables.

### 2. `CONTROL` — why read–modify–write?

Writing `2` directly would silently clear bit 0 (`nPRIV`). Here `nPRIV` is
already 0, so it would make no difference, but the same line would be harmful
when we enter unprivileged Thread Mode in Lesson 05. Also, `CONTROL` has more
than these two fields — other fields depend on architecture/FP/security state
(`FPCA`, `SFPA`). General rule: **preserve bits you are not changing**.

> **Correction.** This file previously implied unprivileged code could regain
> privilege by writing `2` to `CONTROL`. That is **wrong**. Unprivileged Thread
> Mode cannot gain privilege by clearing its own `nPRIV`; elevation requires
> **a privileged exception path**, such as an SVC handler. See Lesson 05.

### 3. `isb` — where exactly is synchronization provided?

Architectural principle: writing `CONTROL` changes execution context; an
instruction synchronization barrier is required to guarantee subsequent
instructions see that change.

But **who executes it** matters:

| Path | Who executes `ISB` |
|---|---|
| Here: manually written `msr control, r2` | **We do** — the `isb` line is ours |
| C calling `__set_CONTROL()` (CMSIS) | **The wrapper itself** — it contains `__ISB()` |
| C calling `__set_CONTROL()` (`core/cm33_teaching.h`) | **The wrapper itself** — it also performs `__ISB()` |

> **Correction.** The previous version proposed an experiment implying that
> "removing `__ISB()` removes synchronization." That was wrong:
> `__set_CONTROL()` already executes `ISB` internally, so an extra C-side
> `__ISB()` is unnecessary and removing it does not remove synchronization.
> The architectural rule remains; the change is correctly identifying where
> the barrier is implemented.

Inspect the source to verify:

```bash
grep -A3 "__set_CONTROL(uint32_t" external/CMSIS/Core/Include/m-profile/cmsis_gcc_m.h
```

---

## Build & Run

```bash
cmake --build --preset debug --target 04-solution-psp
./scripts/run-qemu.sh build/debug/lessons/04-stack-msp-psp/04-solution-psp.elf
```

## Actual output

**Remeasured** from the repaired implementation (arm-none-eabi-gcc 13.2.1,
QEMU 8.2.2, mps2-an505, `debug` preset):

```text
===== LESSON 04 SOLUTION 01 =====
[TASK_STACK_BASE] 0x30000000
[TASK_STACK_LIMIT] 0x30000200
[TOP_MOD_8] 0
[MSP] 0x30007FF0
[PSP] 0x300001F0
[CONTROL] 0x00000002
[LOCAL_ADDR] 0x300001F4
[GUARD_VALUE] 0xDEADBEEF
[CHECK_SPSEL] PASS
[CHECK_REGION] PASS
[CHECK_STACK_GUARD] PASS
===== END =====
```

All three checks also report `PASS` in the `release` and `minsize` presets —
the old solution failed under these two optimization settings.

> Addresses are build-specific and shift if any code changes.

Limit of `CHECK_STACK_GUARD`: guard words establish only that **those two
monitored locations** were not overwritten. They **do not establish** no
overflow below the lower bound, no stray write elsewhere, or `SP` never leaving
its range. Examine those with GDB measurements and disassembly (the lesson's
"Observe in GDB" section).

---

[← Lesson 04](../../README.md)
