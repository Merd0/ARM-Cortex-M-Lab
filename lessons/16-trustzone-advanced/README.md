**Language:** English | [Türkçe](README.tr.md)

# Lesson 16 — TrustZone & Advanced Features

`[DEEP]` · **STATUS: on main; Debug/Release/MinSize CI PASS (11fde900)**

## TEACH — Two security states, one processor

Secure/Non-secure state is distinct from privileged/unprivileged execution.
The SAU, the platform IDAU, and the bus-side MPC work together. Configuring only
the SAU does not make Non-secure RAM usable. Entry into an NSC region uses SG,
a Secure-to-Non-secure call uses BLXNS, and gateway return uses BXNS.
The CMSE compiler also generates the register clearing/preservation code for the
transition.

This lesson does not pretend to demonstrate TrustZone inside a single Secure
firmware image: each scenario loads two separately linked ELFs. The
[Secure startup](secure/main.c), [Non-secure startup](nonsecure/startup.c), and
[NS application](nonsecure/main.c) are separate. Shared platform/startup code and
the linker script used by earlier lessons remain unchanged.

## SHOW — This target's memory partition

- Secure code: 0x10000000, 448 KiB budget.
- NSC veneers: 0x10070000, 4 KiB budget; actual space used is 32 bytes.
- Non-secure code: 0x00080000, 128 KiB.
- Secure RAM: 0x30000000–0x30003FFF; last 4 KiB for MSP.
- Non-secure RAM: 0x20004000–0x20007FFF; last 4 KiB for MSP_NS.

These are AN505 aliases; the physical RAM is not a second copy. The Secure linker
limits the RAM budget to 16 KiB to avoid overlapping the NS half. Each image's
initialized .data LMA is allocated in its own FLASH region.

IoT Kit NSCCFG.CODENSC permits the IDAU's Code NSC usage; the SAU marks only the
veneer region NSC. The SSRAM MPC (0x58007000) opens NS code blocks, and the SRAM MPC
(0x50083000) opens NS RAM blocks. The block size is read from BLK_CFG; QEMU's block
size is not assumed to be fixed for the FPGA. Other blocks remain Secure as at
reset. NS UART/PPC access is not enabled; the Secure side prints trusted fixed labels.

The [API](api.h) accepts only scalars; it does not return pointers, buffers, or
secrets. The service maps inputs [0,15] to 2*x+1 and rejects others with L16_DENIED.
This toy policy is not authentication, secure boot, or product security.

## RUN

```bash
cmake --preset debug
cmake --build --preset debug
python tests/development-smoke.py 16
```

To run a pair manually (a single Secure ELF is not enough):

```bash
qemu-system-arm -M mps2-an505 -cpu cortex-m33 -nographic -monitor none -serial stdio \
  -kernel build/debug/lessons/16-trustzone-advanced/16-trustzone-secure.elf \
  -device loader,file=build/debug/lessons/16-trustzone-advanced/16-trustzone-nonsecure.elf
```

CMake generates the CMSE import object from the Secure link; the Non-secure link
depends on this object. Use both images from the same scenario. GCC/ld generate SG
veneers; do not manually write an ordinary branch into the gateway body.

## OBSERVE

In the Debug / QEMU 11.1.0 run:

```text
[CHECK_PARTITION] PASS
[CHECK_NS_VECTORS] PASS
[CHECK_NS_STARTUP] PASS
[CHECK_SERVICE] PASS
[CHECK_POLICY_DENY] PASS
[CHECK_CALLER_NS] PASS
[SFSR] 0x00000008
[SFAR] INVALID
[CHECK_SECURE_REJECT] PASS
[CHECK_SECRET] PASS
[CHECK_SECURE_HANDLER] PASS
[CHECK_NO_ESCALATION] PASS
```

NS startup initializes its own .data/.bss. Five gateway calls test correct results
for 0,7,15 and rejection for 16,0xFFFFFFFF. The Secure side also checks the calls'
NS origin using cmse_nonsecure_caller. Result bits reported by NS are not security
attestation; they are observations from this controlled test.

NS code then attempts to read 0x30000000. Measured values were SFSR.AUVIOL=1,
Secure handler IPSR=7, and CFSR/HFSR=0. **Since SFARVALID=0, SFAR is not used as a
fault address.** The test also validates the address if SFARVALID is set; it does
not require the bit to be set. An allowed gateway call and rejected direct access
are separate pieces of evidence.

## MODIFY → BREAK → DEBUG

The `16-invalid-entry` pair branches four bytes past SG. Measured values are
SFSR=0x00000001 (INVEP), SFAR=INVALID; the service body does not run, and the secret
is preserved. The expected fault is this scenario's PASS result.

The [exercise](exercises/01-secure-service/README.md) asks only for completion of
the scalar input policy; the fail-closed starter also rejects valid input.
The [reference solution](solutions/01-secure-service/README.md) is separate.

Optional GDB observation (the comprehensive GDB gate was not run at this stage):

```text
break SecureFault_Handler
x/wx 0xE000EDE4
x/wx 0xE000EDE8
print calls
print from_ns
disassemble l16_service
```

Secure ELF symbols show the gateway/handler; for NS symbols, use
`add-symbol-file build/debug/lessons/16-trustzone-advanced/16-trustzone-nonsecure.elf`.
Do not assume the cm-frame/cm-fault helpers decode all cross-security frames.
This lesson does not guess and dereference a frame pointer.

## Advanced features — target limits

M33 does not include MVE/Helium; M55/M85 results do not carry over to this target.
The M33 core has no integrated cache; possible system-level caches are a separate
platform matter. No PACBTI runtime experiment was performed here; the OLCULMEDI
status in the current capability table is preserved. Another CPU's instruction
support is not evidence of success in this lab.

Working FPU/DSP checks are in [Lesson 13](../13-sync-fpu-dsp/); the DWT limitation
of a non-advancing CYCCNT is in [Lesson 12](../12-debug-dwt/) and the
[capability record](../../docs/TARGET_CAPABILITIES.md). This lesson does not
demonstrate cycle/performance measurements, MVE, PACBTI, or cache acceleration.

## VERIFY

The Debug build and QEMU tests of four pairs passed: example/solution all checks
PASS, starter only CHECK_SERVICE=FAIL; skipping SG was caught with the expected
INVEP. SG / BLXNS / BXNS were seen in disassembly. These results are not a real-board
test or a complete security audit. Detailed GDB checks, a three-preset comparison,
and broad negative tests are to be performed at the combined quality gate.

## Quick Check

1. Why is the SAU alone insufficient?
2. Does NSC make all Secure code Non-secure?
3. Why can SFAR not automatically be considered valid when AUVIOL appears?
4. Which pointer problem does the scalar API eliminate in this example?
5. Is this test passing evidence of secure boot?

[Answers](answers/quick-check.md)

## References

- [Cortex-M33 DUG](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d), 4.5: SAU, SFSR/SFAR.
- [IoT Kit FVP User Guide](https://documentation-service.arm.com/static/5ed13622ca06a95ce53f911b), Tables 15/21/49: SRAM MPC and NSCCFG.
- [QEMU AN505 implementation](https://github.com/qemu/qemu/blob/v8.2.2/hw/arm/mps2-tz.c): SSRAM MPC and alias layout.
- [GCC CMSE](https://gcc.gnu.org/onlinedocs/gcc/ARM-ARMv8-M-Security-Extensions.html): nonsecure_entry / nonsecure_call.
- [QEMU fault handling](https://github.com/qemu/qemu/blob/v8.2.2/target/arm/tcg/m_helper.c): AUVIOL path for an NS data attribution fault; context for the model observation.

**Previous:** [15](../15-mini-kernel/) · **Next:** combined quality gate, then [projects](../../projects/).

## Final combined gate

Three-preset clean builds and four paired QEMU scenarios PASS. The Secure transition
and fault boundary of the same boot/SAU/NSC sources reused in the final system were
also verified with GDB. No separate comprehensive GDB/audit certification is claimed
for standalone Lesson 16. [Progress](../../docs/PROGRESS.md) distinguishes all local
gates from remote CI status.
