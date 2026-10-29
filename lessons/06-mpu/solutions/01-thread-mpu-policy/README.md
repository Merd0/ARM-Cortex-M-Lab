**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — Build the Thread MPU Policy

Exercise: [`../../exercises/01-thread-mpu-policy/`](../../exercises/01-thread-mpu-policy/)

## Solution

```c
static void mpu_configure(void)
{
    ARM_MPU_Disable();

    ARM_MPU_SetMemAttr(ATTR_IDX_NORMAL,
        ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1u,1u,1u,1u),
                     ARM_MPU_ATTR_MEMORY_(1u,1u,1u,1u)));

    /* Region 0: FLASH — ayrıcalıksız oku + çalıştır, yazma yok */
    ARM_MPU_SetRegion(0u,
        ARM_MPU_RBAR(FLASH_BASE_ADDR, ARM_MPU_SH_NON, 1u, 1u, 0u),
        ARM_MPU_RLAR(FLASH_LIMIT_ADDR, ATTR_IDX_NORMAL));

    /* Region 1: Thread bloğu — ayrıcalıksız oku + yaz, çalıştırma yok */
    ARM_MPU_SetRegion(1u,
        ARM_MPU_RBAR(UNPRIV_BASE, ARM_MPU_SH_NON, 0u, 1u, 1u),
        ARM_MPU_RLAR(UNPRIV_LIMIT, ATTR_IDX_NORMAL));

    /* g_service_counter için BİLEREK bölge yok */

    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
}
```

## Build & Run

```bash
cmake --build --preset debug --target 06-solution-mpu
./scripts/run-qemu.sh build/debug/lessons/06-mpu/06-solution-mpu.elf
```

Actual output (check lines):

```text
[  CHECK_MPU_INITIALLY_DISABLED] PASS
[  CHECK_MPU_ENABLED] PASS
[  CHECK_PRIVDEFENA] PASS
[  CHECK_MEMFAULT_ENABLED] PASS
[  CHECK_FLASH_REGION_RO_EXEC] PASS
[  CHECK_STACK_REGION_RW_XN] PASS
[  CHECK_THREAD_UNPRIV] PASS
[  CHECK_UNPRIV_REGION_WRITABLE] PASS
[  CHECK_SVC_PROTECTED_ACCESS] PASS
===== END =====
```

---

## Why is each piece necessary?

### Why is the FLASH region required?

With the MPU enabled, unprivileged accesses follow “no match means deny” —
including **instruction fetch**. Without a FLASH region, unprivileged Thread
cannot fetch even its first instruction, immediately causing `IACCVIOL`.

`RO = 1`: protects code itself against writes.
`XN = 0`: permits instruction fetch — why this region exists.

Region 0 also covers `.rodata` (the same FLASH range), letting unprivileged
code read constant strings and tables.

### Why is the task stack region required?

The same rule applies to data. Without a region, Thread cannot perform its
first `push` to its own stack — in the prologue of the `psp_start()` target,
right at the beginning.

`RO = 0`, `NP = 1`: read/write, open to unprivileged access.

Region 1 covers both the stack and Thread's data area (`unpriv_obs_t`).
They occupy one 32-byte-aligned block: regions use 32-byte granularity via
`RBAR.BASE`/`RLAR.LIMIT`, so individually covering scattered objects would
waste regions.

### Why is the stack `XN`?

There is no legitimate reason to execute code there. `XN = 1` prevents
**executing** data injected through stack or buffer overflow — classic W^X
hardening. A violation appears as `MMFSR.IACCVIOL`, not `DACCVIOL`.

Together with Region 0, this establishes: writable memory is not executable,
and executable memory is not writable.

### Why was access to `g_service_counter` not granted?

**Protection comes from omitting a region.** With the Armv8-M MPU enabled,
an unprivileged access without a matching region is denied. The counter
is outside all regions, so unprivileged Thread cannot access it.

Measured addresses show this:

```text
COUNTER_ADDR = 0x30000000
UNPRIV_BASE  = 0x30000020    <- Region 1 starts here
```

We did not define a separate “forbidden region” — the Armv8-M MPU has no
“deny access” region type. **Permitted** regions are listed; the rest is closed.

### Why can the privileged Handler access it?

Because `MPU_CTRL.PRIVDEFENA = 1`. If a privileged access matches no region,
the architecture's **default memory map** applies and permits access.

Unprivileged accesses have **no** fallback. This lesson's policy relies on
precisely that asymmetry.

### What exactly does `PRIVDEFENA` do?

```text
PRIVDEFENA = 1   privileged + no match  -> default map (permitted)
PRIVDEFENA = 0   privileged + no match  -> DENY
in either case      unprivileged + no match -> DENY
```

With `0`, you must define regions for every address used by privileged code
too: MSP stack, kernel data, UART, NVIC... Here, `main()` would lose access
to its own stack when enabling the MPU. Some high-security designs do this
deliberately, but 8 regions quickly become insufficient.

### Why is `MAIR` required?

`RLAR.AttrIndx` is mandatory: every region **must** select one of the 8
attribute entries in `MAIR0`/`MAIR1`. A region cannot be defined without
an attribute.

The attribute tells the processor **what kind of memory** this is — Normal
or Device, cacheable or shareable. Thus, the MPU answers both “who may access”
and “how this memory behaves.”

One attribute was used here (Normal, WB, non-transient, RA/WA), with
`MAIR0 = 0x000000FF` measured. Cache behavior is outside this lesson.

### How are `RBAR`/`RLAR` interpreted?

Measured raw values and their decoding:

```text
Region 0
RBAR = 0x10000006   XN=0  AP=11 (RO=1,NP=1)  SH=00  BASE=0x10000000
RLAR = 0x1007FFE1   EN=1  AttrIndx=0  PXN=0  LIMIT=0x1007FFE0
                    -> INCLUSIVE last byte: 0x1007FFFF

Region 1
RBAR = 0x30000023   XN=1  AP=01 (RO=0,NP=1)  SH=00  BASE=0x30000020
RLAR = 0x30000321   EN=1  AttrIndx=0  PXN=0  LIMIT=0x30000320
                    -> INCLUSIVE last byte: 0x3000033F
```

Bit layout:

```text
RBAR: [31:5] BASE | [4:3] SH | [2:1] AP | [0] XN
RLAR: [31:5] LIMIT | [4] PXN | [3:1] AttrIndx | [0] EN
```

`LIMIT` is **inclusive**: the last byte's address. In the CMSIS header's
terms, it is “one extended” — the low 5 bits are treated as 1. Thus,
writing `0x1007FFE0` covers addresses through `0x1007FFFF`.

### Why cannot Armv7-M “region size” examples be used?

| | Armv7-M (PMSAv7) | Armv8-M (PMSAv8) |
|---|---|---|
| Region | `base` + encoded **size** (power of 2) | `BASE` + `LIMIT` |
| Alignment | Region must align to its own size | 32 bytes suffice |
| Subregion | `SRD` field exists | None |
| Memory type | `TEX`/`C`/`B`/`S` within the region | `AttrIndx` → `MAIR` |

Armv7-M examples' `SIZE` and `SRD` fields **do not exist** in Armv8-M.
Copying them either fails to compile or programs an entirely different
region. Use the Armv8-M ARM or the vendored
`external/CMSIS/Core/Include/m-profile/armv8m_mpu.h`.

---

## Verify that protection works

With the correct policy, a violation causes `MemManage`:

```text
IPSR  = 4               MemManage (HardFault degil)
CFSR  = 0x00000082      MMFSR.DACCVIOL = 1, MMARVALID = 1
HFSR  = 0x00000000      escalation yok
MMFAR = 0x30000000      = &g_service_counter
stacked PC = 0x10000078 = "str r1, [r0, #0]"
```

---

## Boundary

This policy contains only two regions. Real systems need peripherals,
shared buffers, and **a separate stack region for each task** — with only
8 regions on this target. Policy design is a budgeting problem.

---

[← Lesson 06](../../README.md)
