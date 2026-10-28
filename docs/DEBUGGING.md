**Language:** English | [Türkçe](DEBUGGING.tr.md)

# Cortex-M Debugging Guide

This file teaches a **diagnostic workflow**, not a command list.
For the full command list: `scripts/gdb/cortex-m.gdb`

## Setup: two terminals

Terminal 1 - start QEMU halted:

```bash
./scripts/debug-qemu.sh build/debug/lessons/02-programmer-model/02-programmer-model.elf
```

Terminal 2 - connect GDB:

```bash
arm-none-eabi-gdb -x scripts/gdb/cortex-m.gdb \
    build/debug/lessons/02-programmer-model/02-programmer-model.elf
(gdb) cm-connect
```

If `arm-none-eabi-gdb` is unavailable, `gdb-multiarch` works the same way.

## Basic commands

| Command | What it does |
|---|---|
| `break main` / `b main` | Stop in a function |
| `continue` / `c` | Continue |
| `next` / `n` | One C line (without entering functions) |
| `step` / `s` | One C line (entering functions) |
| `stepi` / `si` | **One assembly instruction** |
| `finish` | Return from the function |
| `info registers` | All core registers |
| `p/x $sp` | One register, hexadecimal |
| `x/16wx $sp` | 16 memory words, hexadecimal |
| `disassemble /s` | Disassembly mixed with source |
| `watch my_var` | Stop when the variable changes |
| `rwatch` / `awatch` | Read / any access |

Repository-specific Cortex-M commands (`scripts/gdb/cortex-m.gdb`):

| Command | Usage |
|---|---|
| `cm-connect` | `target remote :3333` |
| `cm-regs` | R0-R12, SP/LR/PC/xPSR + MSP/PSP/CONTROL/IPSR |
| `cm-mode` | Mode, privilege, active stack |
| `cm-stack` | RAW dump of 16 words from active SP (NOT frame decoding) |
| `cm-exc-return <deger>` | Decode an EXC_RETURN value |
| `cm-frame <adres> <EXC_RETURN>` | Decode a standard 8-word frame (see below) |
| `cm-fault` | CFSR/HFSR/MMFAR/BFAR + validity bits + names of set CFSR bits (`SET` line) |
| `cm-vectors` | VTOR and vector-table entries |
| `cm-mpu` | MPU_TYPE / MPU_CTRL |

Important prerequisite, before connecting:

```text
set architecture armv8-m.main
```

Without this, `$msp`, `$psp`, `$control`, and `$psplim` are not visible.
(`set arm force-mode thumb` affects only disassembly mode; it is NOT architecture selection.)

## Diagnostic workflows

### "Why did the program get here?"

```text
bt                 # cagri yigini
info frame         # mevcut frame
p/x $lr            # nereye donecek
```

If `bt` looks meaningless, the stack may be corrupt - see below.

### "Is the stack corrupt?"

```text
cm-regs                     # SP mantikli bir araliktata mi?
p/x &__stack_limit
p/x &__stack_top
```

If SP has fallen below `__stack_limit`, there is a **stack overflow**.
If SP is entirely outside this range, the wrong stack pointer may have been
selected (CONTROL.SPSEL).

```text
x/32wx $sp                  # stack icerigi mantikli mi
```

Return addresses should appear in the `0x1000xxxx` (FLASH) range.
Random-looking values are a sign of corruption.

### "Which instruction is PC pointing to?"

```text
x/i $pc                     # su anki komut
x/10i $pc-20                # cevresi
info line *$pc              # hangi C satiri
```

### "Why did this fault occur?"

Sequential workflow:

```text
cm-mode                       # 1) Handler'da miyiz? Hangi exception?
cm-fault                      # 2) CFSR/HFSR ne diyor? Stacking hatasi var mi?
cm-exc-return <kaydedilen LR> # 3) Frame hangi stack'te, hangi turde?
cm-frame <adres> <EXC_RETURN> # 4) Frame'i coz
x/i <stacked PC>              # 5) O komuta bak
```

### How to use `cm-frame`

```text
cm-frame <frame_adresi> <EXC_RETURN>
```

Requiring YOU to supply both arguments is a deliberate design choice:

**Frame address.** Handler runs on MSP, but the interrupted context may have
stacked its frame on PSP. The handler's own prologue can also move MSP away from
the start of the frame. Thus **active `$sp` is generally not the frame address**,
and the command does not guess from it.

**EXC_RETURN.** Only this value identifies the frame TYPE (standard / FP-extended,
which stack, which security state). Capture it from `$lr` **immediately on handler
entry**; after a C call inside the handler, live `$lr` may no longer be EXC_RETURN.

Typical flow:

```text
(gdb) b MyFault_Handler
(gdb) continue
(gdb) set $er = $lr              # handler girisinde: EXC_RETURN
(gdb) cm-exc-return $er          # bit2 -> hangi stack?
(gdb) cm-frame $psp $er          # bit2 = 1 ise
(gdb) cm-frame $msp $er          # bit2 = 0 ise (handler henuz stack kullanmadiysa)
```

### Limits of `cm-frame`

This command rejects unsupported frame types and invalid values. Passing its
checks does not establish that hardware previously wrote an exception frame at
the supplied address; an address alone cannot establish that.

Rejected cases:

Rejection messages have categories; they are not collapsed into a single generic error:

| Label | Meaning |
|---|---|
| `[BICIMSIZ EXC_RETURN]` | Value does not match the architectural encoding |
| `[DESTEKLENMIYOR]` | Architecturally valid, but this command does not decode that format |
| `[STACKING HATASI]` | Hardware may have failed to stack the frame |
| `[GECERSIZ FRAME ADRESI]` | Misaligned or outside the RAM window |
| `[GECERSIZ FRAME ICERIGI]` | The values read cannot form a frame |

| Check | Category | Rejection reason |
|---|---|---|
| EXC_RETURN[31:7] | BICIMSIZ | RES1, all must be 1. **Checking only the `0xFFFFFF` prefix is NOT ENOUGH** |
| EXC_RETURN[1] | BICIMSIZ | Reserved, must be zero (this makes `0xFFFFFFFF` invalid) |
| `Mode = 0` + `SPSEL = 1` | BICIMSIZ | Handler Mode uses the Main stack; no such encoding exists (`0xFFFFFFF5`) |
| `FType = 0` | DESTEKLENMIYOR | FP-extended frame (FPU, Lesson 13) |
| `DCRS = 0` | DESTEKLENMIYOR | Additional state context (callee registers stacked) |
| `ES != S` | DESTEKLENMIYOR | Transition between security states (Lesson 16) |
| CFSR stacking bit | STACKING HATASI | STKOF / STKERR / MSTKERR |
| Address alignment | GECERSIZ FRAME ADRESI | Not a multiple of 4 |
| Address range | GECERSIZ FRAME ADRESI | Outside the platform RAM window |
| Stacked xPSR | GECERSIZ FRAME ICERIGI | T bit (bit24) zero - impossible on Cortex-M |
| Stacked PC | GECERSIZ FRAME ICERIGI | 0, odd, or outside the code window |

In practice, only the following EXC_RETURN values are supported:

`cm-frame` supports a **deliberately selected SUBSET** of architecturally valid
EXC_RETURN formats: standard frame (`FType=1`), default callee stacking
(`DCRS=1`), and one security state (`ES == S`). Six values remain:

| Value | ES | S | Mode | SPSEL | Meaning |
|---|---|---|---|---|---|
| `0xFFFFFFF1` | 1 | 1 | 0 | 0 | Secure, return to Handler Mode, frame on MSP |
| `0xFFFFFFF9` | 1 | 1 | 1 | 0 | Secure, return to Thread Mode, frame on MSP |
| `0xFFFFFFFD` | 1 | 1 | 1 | 1 | Secure, return to Thread Mode, frame on PSP |
| `0xFFFFFFB0` | 0 | 0 | 0 | 0 | Non-secure, return to Handler Mode, frame on MSP |
| `0xFFFFFFB8` | 0 | 0 | 1 | 0 | Non-secure, return to Thread Mode, frame on MSP |
| `0xFFFFFFBC` | 0 | 0 | 1 | 1 | Non-secure, return to Thread Mode, frame on PSP |

In all six values, `[31:7]` is RES1 (all ones), `[1] = 0`, `FType = 1`,
`DCRS = 1`, and rows with `Mode = 0` have `SPSEL = 0`.

**Common mistake:** `0xFFFFFFB1` / `0xFFFFFFB9` / `0xFFFFFFBD` are unsupported.
In these values, `ES = 1` but `S = 0` — the exception was taken from Secure
state, but the frame is on the Non-secure stack. This is an architecturally valid
security-state transition; `cm-frame` rejects it as `[DESTEKLENMIYOR]` (Lesson 16).

Source: Armv8-M Exception Model User Guide (Arm 107706), EXC_RETURN bit-field
table — `[31:7] RES1`, `[6] S`, `[5] DCRS`, `[4] FType`, `[3] Mode`,
`[2] SPSEL`, `[1]` reserved, `[0] ES`.

Platform windows can be configured through `$cm_ram_start` / `$cm_ram_end`
and `$cm_code_start` / `$cm_code_end`.

CFSR reading guide:

| Field | Bit range | Meaning |
|---|---|---|
| MMFSR | CFSR[7:0] | MemManage - usually an MPU violation |
| BFSR | CFSR[15:8] | BusFault - invalid memory access |
| UFSR | CFSR[31:16] | UsageFault - invalid instruction/state |

Common bits:

| Bit | Name | Meaning |
|---|---|---|
| CFSR[1] | MMFSR.DACCVIOL | Data access blocked by the MPU |
| CFSR[7] | MMFSR.MMARVALID | MMFAR contains a valid address |
| CFSR[9] | BFSR.PRECISERR | Fault address is precise |
| CFSR[12] | BFSR.STKERR | **Error during exception stacking** |
| CFSR[15] | BFSR.BFARVALID | BFAR contains a valid address |
| CFSR[16] | UFSR.UNDEFINSTR | Undefined instruction |
| CFSR[17] | UFSR.INVSTATE | Invalid state (often Thumb bit 0) |
| HFSR[30] | FORCED | Another fault **escalated** |

When you see `HFSR.FORCED = 1`: "the original fault is not HardFault."
The actual cause is in CFSR; it escalated to HardFault because the corresponding
fault handler was disabled.

### Special case: stacking itself fails

If `BFSR.STKERR`, `UFSR.STKOF`, or `MMFSR.MSTKERR` is set, hardware may have
**failed to stack** the exception frame; there is no trustworthy frame in memory.
`cm-frame` **refuses** to decode when it sees these bits, rather than interpreting
untrustworthy memory as register values.

This repository contains two examples:

| Lesson | Bit | Scenario |
|---|---|---|
| 02 "Break It" | `BFSR.STKERR` | Deliberately invalid PSP, then SPSEL = 1 |
| 04 "Break It" | `UFSR.STKOF` | `PSPLIM` + deep recursion |

Note: these bits are **sticky**. They may remain from an earlier fault;
if unsure, clear them by writing 1 and try again.

### "Why is the variable changing?"

```text
watch counter
continue
bt                 # degistiren kod kim
```

In code compiled with `-O2`, the variable may be held in a register and a
watchpoint may not work. First try `-O0`/the `Debug` preset.

### "Optimization ate my variable"

```text
<optimized out>
```

GDB cannot display it because the compiler eliminated the variable during
optimization. Use the `Debug` preset (`-Og -g3`) or make the variable `volatile`.

## Stepping from reset

```text
cm-connect
cm-vectors                 # vector table'da ne var
p/x $pc                    # Reset_Handler'da miyiz
si                         # tek tek komut
break main
c
```

Tracing startup this way is the main exercise of Lesson 11.

## QEMU-side tips

```bash
# Gecersiz bellek erisimlerini logla
qemu-system-arm -M mps2-an505 -kernel fw.elf -nographic \
                -d guest_errors,unimp

# Icra edilen komutlari logla (cok buyuk dosya)
qemu-system-arm -M mps2-an505 -kernel fw.elf -nographic \
                -d in_asm -D trace.log
```

---

## Fault triage

The systematic fault-diagnosis checklist is in Lesson 08:
[`lessons/08-faults/README.md`](../lessons/08-faults/README.md) → "Cortex-M Fault Triage".
Short form: IPSR → HFSR.FORCED → CFSR subfield → cause bits → VALID bits →
stacking error → EXC_RETURN → frame → disassembly.
