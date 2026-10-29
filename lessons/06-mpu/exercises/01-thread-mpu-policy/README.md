**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Build the Thread MPU Policy

**Goal:** Turn Lesson 05's software contract into a hardware rule.

## Task

Complete `mpu_configure()` in `main.c`. `MAIR` configuration and
`ARM_MPU_Disable()` are provided; the remaining four steps are yours.

| Step | What |
|---|---|
| 1 | **Region 0 — FLASH:** unprivileged read + execute, no write |
| 2 | **Region 1 — Thread block:** unprivileged read + write, no execution |
| 3 | Enable the MemManage exception |
| 4 | Enable the MPU; let privileged accesses use the default map |

**Do not define** a region for `g_service_counter`. Protection comes from
omitting a region: an unprivileged access matching no region is denied.

Provided address constants: `FLASH_BASE_ADDR`, `FLASH_LIMIT_ADDR`, `UNPRIV_BASE`,
`UNPRIV_LIMIT`, `ATTR_IDX_NORMAL`.

## Available CMSIS API

Official header vendored in the repository:
`external/CMSIS/Core/Include/m-profile/armv8m_mpu.h`

```c
ARM_MPU_SetRegion(rnr, rbar, rlar)
ARM_MPU_RBAR(BASE, SH, RO, NP, XN)
ARM_MPU_RLAR(LIMIT, AttrIndx)        /* LIMIT = bölgenin SON byte'ı */
ARM_MPU_SH_NON
ARM_MPU_Enable(control)
MPU_CTRL_PRIVDEFENA_Msk
SCB_SHCSR_MEMFAULTENA_Msk
```

`ARM_MPU_RBAR` parameters: `RO = 1` means read-only, `NP = 1` permits
unprivileged access, `XN = 1` means non-executable.

## ⚠️ Be careful with Armv7-M examples

The Armv8-M MPU uses **base + limit**. Armv7-M's “region size” field
**does not exist here**, nor must a region align to its own size. Most
online MPU examples are for Armv7-M and cannot be applied directly.

`LIMIT` is **inclusive**: the address of the region's last byte.
Granularity is 32 bytes.

## Build & Run

```bash
cmake --build --preset debug --target 06-exercise-mpu
./scripts/run-qemu.sh build/debug/lessons/06-mpu/06-exercise-mpu.elf
```

Initial output (actual):

```text
[  CHECK_MPU_ENABLED] FAIL
[  CHECK_PRIVDEFENA] FAIL
[  CHECK_MEMFAULT_ENABLED] FAIL
[  CHECK_FLASH_REGION_RO_EXEC] FAIL
[  CHECK_STACK_REGION_RW_XN] FAIL
[  CHECK_THREAD_UNPRIV] PASS
[  CHECK_UNPRIV_REGION_WRITABLE] PASS
[  CHECK_SVC_PROTECTED_ACCESS] PASS
===== END =====
```

With the MPU disabled, firmware runs normally and SVC works — but
**there is no protection**. This is the expected initial state, not a bug.

## Verification

All eight `CHECK_*` lines must be `PASS`.

`CHECK_FLASH_REGION_RO_EXEC` and `CHECK_STACK_REGION_RW_XN` read back the
registers and check `AP`, `XN`, and `EN`, verifying that the policy reached
hardware.

### Verify the violation too

After implementing the policy, check protection with this experiment:

```bash
cmake --build --preset debug --target 06-mpu-broken
./scripts/run-qemu.sh build/debug/lessons/06-mpu/06-mpu-broken.elf
```

This target attempts a direct write using the reference policy and receives
`MemManage`. In GDB, use `cm-fault` to verify that `MMFAR` equals `COUNTER_ADDR`.

## If you are stuck

Reference solution: [`../../solutions/01-thread-mpu-policy/`](../../solutions/01-thread-mpu-policy/)

---

[← Lesson 06](../../README.md)
