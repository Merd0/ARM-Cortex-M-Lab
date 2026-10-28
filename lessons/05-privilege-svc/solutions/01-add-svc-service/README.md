**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — Add an SVC Service

Exercise: [`../../exercises/01-add-svc-service/`](../../exercises/01-add-svc-service/)

## Solution

```c
case SVC_GET_VERSION:
    result = 0x00010002u;
    break;
```

One line of work. What matters is **where** that line belongs, and **why**.

## Build & Run

```bash
cmake --build --preset debug --target 05-solution-svc2
./scripts/run-qemu.sh build/debug/lessons/05-privilege-svc/05-solution-svc2.elf
```

Actual output (relevant lines):

```text
[5] SVC uzerinden ayricalikli servis
[  SVC1_RESULT] 1
[  SVC1_RESULT_AGAIN] 2
[  CHECK_SVC_SERVICE] PASS
[  CONTROL_IN_THREAD_AFTER_SVC] 0x00000003
[  CHECK_SVC_DID_NOT_PROMOTE] PASS
[6] SVC #2 (get_service_version)
[  SVC2_VERSION] 0x00010002
[  CHECK_SVC2_VERSION] PASS
```

## The complete chain

### 1. How was the `SVC` instruction generated?

The `SVC` immediate is a **compile-time constant**, not a variable.
Each service therefore needs inline assembly:

```c
#define SVC_CALL(num)                                                   \
    ({                                                                  \
        register uint32_t _r0 __asm__("r0");                            \
        __asm__ volatile ("svc %[imm]"                                  \
                          : "=r" (_r0)                                  \
                          : [imm] "I" (num)                             \
                          : "memory");                                  \
        _r0;                                                            \
    })
```

The `"I"` constraint tells the compiler “this must be an immediate constant.”
We obtain the result from `r0` using `register ...
__asm__("r0")`, because exception return restores `R0` from the frame.

Generated code (measured):

```text
10000194:  df01   svc  1
100001a2:  df04   svc  4
```

16 bits, `0xDF` + `imm8`.

### 2. How was the immediate decoded?

```c
const uint8_t *svc_instr = (const uint8_t *)(frame->pc - 2u);
uint32_t svc_number = (uint32_t)svc_instr[0];
```

Stacked PC points to the instruction **after** `SVC`. Since `SVC` is 16 bits,
the instruction is at `pc - 2`, and its low byte is `imm8`.

Verified in GDB:

```text
(gdb) x/2i (*(unsigned int*)($psp+24))-2
   0x10000194 <thread_entry+104>:  svc  1
   0x10000196 <thread_entry+106>:  str  r0, [r4, #28]
```

This calculation applies **only to 16-bit `SVC`**; there is no general rule
that “the faulting instruction is at `pc - 2`” (Lesson 08).

### 3. Where do the argument and result live?

The exception frame's first word is `R0`. Hardware writes `R0` there on
exception entry and restores it from there on exception return.

```c
frame->r0 = result;      /* Thread bunu R0'da gorecek */
```

Thus, the service result reaches the AAPCS return-value register (`r0`)
through the frame. Calling C code can read `SVC_CALL(...)` like an ordinary
function return value.

Arguments would follow the same path: Thread places them in `R0`/`R1`;
the dispatcher reads `frame->r0` / `frame->r1`.

### 4. Which stack contains the interrupted frame?

PSP — because Thread was using PSP. `svc_handler.S` determines this from
`EXC_RETURN` bit 2:

```asm
    tst     lr, #4
    ite     eq
    mrseq   r0, msp
    mrsne   r0, psp
```

Measurement: at handler entry, `$lr = 0xfffffffd` (bit 2 = 1), and
`frame == $psp == 0x30000308`.

### 5. Why is the handler privileged?

Not because its code is special — **because it runs in Handler Mode.**
Handler Mode is architecturally always privileged. `cm-mode` output:

```text
Mode      : Handler Mode (exception 11)
Privilege : Privileged (Handler Mode her zaman ayricaliklidir)
            CONTROL.nPRIV = 1 -> KESILEN Thread baglami ayricaliksizdi
```

Reading `CONTROL = 0x1` does not make the handler unprivileged;
it is the interrupted Thread's state.

### 6. Why could the unprivileged Thread not elevate itself?

Writes to `CONTROL` from unprivileged Thread Mode are ignored. Measurement:

```text
CONTROL_UNPRIV           0x00000003
CONTROL_AFTER_ATTEMPT    0x00000003    (degismedi)
```

Not even a fault occurs — the write silently has no effect.

### 7. How does exception return resume Thread?

`svc_dispatch` returns like an ordinary C function. However, because
`svc_handler.S` branched to it with **`b`**, not `bl`, `lr` still holds
`EXC_RETURN` (`0xFFFFFFFD`). When the function's epilogue writes this value
to `pc`, the processor interprets it as exception return: the frame is
restored from PSP, and execution returns to Thread Mode.

Measurement — immediately after return:

```text
Mode      : Thread Mode
Privilege : Unprivileged (CONTROL.nPRIV = 1)
r0 = 0x1
```

`SVC` did not automatically elevate Thread, and the service result arrived in `R0`.

## Boundaries

This service boundary provides **architectural privilege flow, not memory
isolation**. `g_service_counter` is still ordinary RAM; unprivileged code
can write it without `SVC`. Lesson 06 will block direct access using the MPU.

---

[← Lesson 05](../../README.md)
