**Language:** English | [Türkçe](REFERENCES.tr.md)

# References

Technical claims in this repository are based primarily on the following
**official** sources. Tutorials/blogs are not used as sources.

## Arm architecture documentation

| Document | Purpose |
|---|---|
| **Armv8-M Architecture Reference Manual** (Arm DDI 0553) | Definitive architectural source: registers, exception model, MPU, SAU, instruction set |
| **Cortex-M33 Devices Generic User Guide** (Arm 100235) | Cortex-M33-specific programming information, CPUID, NVIC, SysTick |
| **Cortex-M33 Technical Reference Manual** (Arm 100230) | Implementation details, DWT, debug units |
| **Procedure Call Standard for the Arm Architecture (AAPCS)** | Calling convention: arguments, return value, preserved registers |
| **Arm Cortex-M for Beginners / Armv8-M whitepapers** | Overview and processor comparison |

Access: <https://developer.arm.com/documentation>

## Board / platform

| Document | Purpose |
|---|---|
| **Application Note AN505** (Arm DAI0505) | MPS2-AN505 FPGA image: memory map, peripherals |
| **Arm IoT Kit FVP User Guide** (Arm ECM0601256) | IoT Kit subsystem underlying AN505 |
| **Cortex-M System Design Kit TRM** (Arm DDI 0479) | CMSDK APB UART register layout |
| **MPS2+ Board TRM** (Arm 100112) | Board-level hardware |

## QEMU

| Source | Purpose |
|---|---|
| QEMU Arm MPS2/MPS3 board documentation | Modeled machines, differences between QEMU and real hardware |
| `qemu-system-arm -machine help` | Available machines (local verification) |
| `qemu-system-arm -cpu help` | Available processors (local verification) |

<https://www.qemu.org/docs/master/system/arm/mps2.html>

## Toolchain

| Source | Purpose |
|---|---|
| Arm GNU Toolchain downloads and release notes | Compiler version and features |
| GNU ld documentation | Linker-script syntax |
| GNU binutils documentation | objdump/objcopy/nm/size |
| GDB documentation | Debug commands, remote protocol |

<https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>

## CMSIS

| Source | Purpose |
|---|---|
| CMSIS-Core (M) documentation | Standard register names and API |

Note: this repository's baseline core API is **official CMSIS-Core**
(`external/CMSIS/`, Apache-2.0; entry point `platform/mps2_an505.h`).
A minimal handwritten teaching header also exists for lessons where registers
themselves are the subject (`core/include/core/cm33_teaching.h`). Naming is
kept identical in both. Rationale and usage rule: `docs/ARCHITECTURE.md`

## Source-use rule

- Verify a technical claim against official documentation before writing it
- If something can be measured, **measure it**, do not cite a source, and state "measured"
- Unverified information enters the repository as `OLCULMEDI` / `DOGRULANMADI`

---

## Source policy (clarified after repair)

The earlier version contained wording that could imply "measured data needs no
reference." Correction: **the measurement itself** needs no reference, but
**interpreting the measurement** does.

| Statement | Source |
|---|---|
| "`SAU_TYPE` read 0x8 in QEMU" | Measurement. Source: `tools/capability-probe`. |
| "This target therefore has 8 SAU regions" | Interpretation. Source: Armv8-M ARM, `SAU_TYPE` definition. |
| "Every Cortex-M33 therefore has 8 regions" | **Invalid.** Neither the measurement nor the documentation says this. |

A successful measurement does not prove a universal architectural claim.
Behavior observed on one target is that target's contract.

Primary sources (direct links):

- Armv8-M Architecture Reference Manual (Arm DDI 0553):
  <https://developer.arm.com/documentation/ddi0553/latest/>
- Cortex-M33 Devices Generic User Guide (Arm **100235**):
  <https://developer.arm.com/documentation/100235/latest/>
- Cortex-M33 Technical Reference Manual (Arm 100230):
  <https://developer.arm.com/documentation/100230/latest/>
- AAPCS32: <https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst>
- CMSIS-Core (M): <https://arm-software.github.io/CMSIS_6/latest/Core/index.html>
- QEMU v8.2.2 AN505 model:
  <https://github.com/qemu/qemu/blob/v8.2.2/hw/arm/mps2-tz.c>
- QEMU v8.2.2 NVIC:
  <https://github.com/qemu/qemu/blob/v8.2.2/hw/intc/armv7m_nvic.c>
- GNU as (ARM directives):
  <https://sourceware.org/binutils/docs/as/ARM-Directives.html>

Note: `tests/check-links.py` verifies only **relative** Markdown links.
It does not verify the external URLs above or in-document heading anchors.
