**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — Decode an Exception Entry

Exercise: [`../../exercises/01-decode-exception-entry/`](../../exercises/01-decode-exception-entry/)

## Solution

```c
static void decode_exception_entry(const exception_frame_t *frame,
                                   uint32_t exc_return,
                                   uint32_t ipsr,
                                   entry_info_t *out)
{
    out->interrupted_mode = (exc_return & 0x8u) ? "Thread" : "Handler";
    out->frame_stack      = (exc_return & 0x4u) ? "PSP"    : "MSP";
    out->exception_number = ipsr & 0x1FFu;

    out->stacked_r0   = frame->r0;
    out->stacked_pc   = frame->pc;
    out->stacked_xpsr = frame->xpsr;
}
```

## Build & Run

```bash
cmake --build --preset debug --target 07-solution-decode
./scripts/run-qemu.sh build/debug/lessons/07-exceptions/07-solution-decode.elf
```

Actual output:

```text
[4] Exception girisi cozumu
[  INTERRUPTED_MODE] Thread
[  FRAME_STACK] PSP
[  EXCEPTION_NUMBER] 11
[  STACKED_R0] 0x11111111
[  STACKED_PC] 0x100000BA
[  STACKED_XPSR] 0x61000000
[  CHECK_DECODE_MODE] PASS
[  CHECK_DECODE_STACK] PASS
[  CHECK_DECODE_EXCNUM] PASS
[  CHECK_DECODE_R0] PASS
[  CHECK_DECODE_PC] PASS
[  CHECK_DECODE_XPSR] PASS
```

---

## Why each line is written this way

### `interrupted_mode` — `EXC_RETURN` bit 3

```c
(exc_return & 0x8u) ? "Thread" : "Handler"
```

Bit 3 (`Mode`) says **where** to return. The return destination is where the
interrupted context was — therefore also the answer to "which mode did we interrupt?"

`1` = return to Thread Mode, `0` = return to Handler Mode (nested exception).

In this measurement, `0xFFFFFFFD` → bit 3 = 1 → Thread. Correct, because `svc`
was executed from Thread Mode.

### `frame_stack` — `EXC_RETURN` bit 2

```c
(exc_return & 0x4u) ? "PSP" : "MSP"
```

Bit 2 (`SPSEL`) says **which stack received the frame**.

**This does not ask which stack the handler uses.** A handler always uses MSP —
hardware clears `SPSEL` during entry. Measurement:

```text
frame (PSP'de):     0x300002d0
handler $sp (MSP):  0x30007ff0
```

Two different regions, both correct.

In Experiment 1's MSP-origin case, `EXC_RETURN = 0xFFFFFFF9` → bit 2 = 0 → frame
on MSP. There the frame and handler share the stack; because it is full-descending,
the handler's prologue and local data push MSP further down and grow **below
the saved frame, at lower addresses**.

### `exception_number` — `IPSR`

```c
ipsr & 0x1FFu
```

`IPSR` is the low 9 bits of `xPSR` and holds the number of the **currently
executing** exception. `0` = Thread Mode.

Here `11` = SVCall. The same number is the vector-table index: `vector[11]`.

Note: the `IPSR` parameter was read using `__get_IPSR()` inside the handler.
**The `IPSR` field in stacked `xPSR` is different** — it is the exception number
of the *interrupted* context, and is `0` here (we were in Thread Mode).
Do not confuse them:

```text
__get_IPSR()  (handler icinde)  = 11   <- calisan exception
frame->xpsr & 0x1FF             = 0    <- kesilen baglamin modu
```

### `stacked_*` — frame slots

Offsets are architecturally fixed; the `exception_frame_t` struct reproduces the order exactly:

```text
+0x00 R0   +0x04 R1   +0x08 R2   +0x0C R3
+0x10 R12  +0x14 LR   +0x18 PC   +0x1C xPSR
```

`stacked_pc = 0x100000BA` → the instruction **after** `svc`. `svc 7` is at `PC - 2`
(verified with `x/2i` in the README's GDB section).

`stacked_xpsr = 0x61000000` → T bit (bit 24) `1`, `IPSR` field `0`, alignment
bit (bit 9) `0`.

---

## Why is it this simple?

Because the **wrapper** did the difficult part: saving `EXC_RETURN` and the frame
address at entry. Without that information this function cannot be written —
after C runs in the handler, `lr` and `msp` are no longer their entry values.

The solution's real lesson: **capturing the right moment is harder than reading
the right bits.**

## Scope

This solution assumes `FType = 1` (standard frame) and one security state.
An extended FP frame has the same `stacked_*` offsets but is longer; security
state transitions introduce further variants. `cm-frame` already rejects these
cases — see `docs/DEBUGGING.md`.

---

[← Lesson 07](../../README.md)
