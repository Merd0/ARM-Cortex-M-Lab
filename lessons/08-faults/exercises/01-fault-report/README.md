**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Complete the Fault Report

**Goal:** produce **decisions** from raw fault registers — the triage list in code.

## Task

Complete `decode_fault_snapshot()` in `decode.c`. The firmware produces one
deterministic fault: an unprivileged Thread directly writes protected data
(Lesson 06's MemManage `DACCVIOL`). The raw snapshot is ready.

| Field | Decision | Hint |
|---|---|---|
| `active_exception` | Which handler is running? | `s->ipsr`: 3/4/5/6 |
| `escalated` | Was it escalated? | `HFSR.FORCED` |
| `fault_class` | Which `CFSR` subfield is populated? | `MMFSR`/`BFSR`/`UFSR` |
| `primary_bit` | Primary cause bit | `VALID` bits are **not** causes |
| `address_valid` | Is the address register evidence? | `MMARVALID` / `BFARVALID` |
| `fault_address` | Only if valid | |
| `frame_safe` | Can the frame be decoded? | `s->frame_valid` |
| `stacked_pc` | Only if the frame is safe | |

**Scope:** you are not asked to decode every `CFSR` bit. This lesson's three fault
types (MemManage, BusFault, UsageFault) suffice.

## ⚠️ Pitfall

`fault_class` is **not derived from the active handler**; it comes from `CFSR`.
In an escalation case, the active handler is HardFault but `fault_class` must be
UsageFault (or MemManage). Consider this while writing your code, even though
this target does not produce escalation.

## Build & Run

```bash
cmake --build --preset debug --target 08-exercise-fault-report
./scripts/run-qemu.sh build/debug/lessons/08-faults/08-exercise-fault-report.elf
```

Actual starter output:

```text
[  D_ACTIVE_EXCEPTION] ?
[  D_ESCALATED] YES
[  D_FAULT_CLASS] ?
[  D_PRIMARY_BIT] ?
[  CHECK_D_ACTIVE] FAIL
[  CHECK_D_NOT_ESCALATED] FAIL
[  CHECK_D_CLASS] FAIL
[  CHECK_D_BIT] FAIL
[  CHECK_D_ADDRESS] FAIL
[  CHECK_D_FRAME] FAIL
```

This is the **expected starting state**, not a bug.

## Verification

All six `CHECK_D_*` lines must say `PASS`.

Text fields are compared **in full**, not just their first character.
Expected spellings are given in the exercise comment (`"MemManage"`,
`"BusFault"`, `"UsageFault"`, `"HardFault"`; cause bits use CMSIS names such as
`"DACCVIOL"`). A wrong answer starting with the same letter, such as
`"DIVBYZERO"`, does **not pass** `CHECK_D_BIT`.

## Test your function on other cases (optional)

Also link `decode.c` into `08-faults-bus` or `08-faults-escalation` and check
your decisions. The escalation case must give `active_exception = HardFault`,
`escalated = 1`, `fault_class = UsageFault`.

## If you are stuck

Reference solution: [`../../solutions/01-fault-report/`](../../solutions/01-fault-report/)

---

[← Lesson 08](../../README.md)
