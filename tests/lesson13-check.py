#!/usr/bin/env python3
"""Lesson 13 runtime, debugger and ABI checks. Evidence stays in ignored build/."""
import argparse
import json
from pathlib import Path
import re
import shutil
import socket
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def require(ok, name):
    if not ok:
        raise AssertionError(name)
    print("PASS:", name, flush=True)


def stop(process):
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()


def qemu(elf):
    return ["qemu-system-arm", "-M", "mps2-an505", "-cpu", "cortex-m33",
            "-nographic", "-monitor", "none", "-serial", "stdio", "-kernel", str(elf)]


def firmware(elf, evidence):
    path = evidence / (elf.stem + "-uart.txt")
    with path.open("w") as output:
        process = subprocess.Popen(qemu(elf), stdout=output, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + 10
            while time.monotonic() < deadline:
                text = path.read_text(errors="replace")
                if "===== END =====" in text:
                    return text
                if process.poll() is not None:
                    break
                time.sleep(0.02)
            raise AssertionError("no END: " + str(path))
        finally:
            stop(process)


def expect(text, name, value):
    require(re.search(r"^\[" + re.escape(name) + r"\] " + re.escape(str(value)) + r"$",
                      text, re.M), name + " = " + str(value))


def sync_checks(build, evidence):
    for name in ("13-sync-fpu-dsp", "13-sync-broken"):
        text = firmware(build / (name + ".elf"), evidence)
        for check in ("LOST_UPDATE", "INTERRUPTED_RESERVATION", "DSP", "BASELINE_SOFT", "NO_FAULT"):
            expect(text, "CHECK_" + check, "PASS")
        broken = name.endswith("broken")
        expect(text, "CHECK_ATOMIC_TOTAL", "FAIL" if broken else "PASS")
        expect(text, "RACY_TOTAL", 1)
        expect(text, "EXCLUSIVE_TOTAL", 1 if broken else 2)
        expect(text, "FIRST_STREX_STATUS", 1)
        expect(text, "RETRIES", 1)
        expect(text, "DOT_C", "0xFFFF7360")
        expect(text, "DOT_DSP", "0xFFFF7360")
        expect(text, "SATURATION_AND_Q", "PASS")
    for name, count in (("13-exercise-critical", 2), ("13-solution-critical", 6)):
        text = firmware(build / (name + ".elf"), evidence)
        for key in ("OPEN_MASK", "NESTED_MASK"):
            expect(text, key, "PASS")
        for key in ("OPEN_IRQ", "OPEN_TOTAL", "NESTED_IRQ", "NESTED_TOTAL"):
            expect(text, key, "PASS" if count == 6 else "FAIL")
        expect(text, "CASES_PASSED", count)


def debugger(elf, evidence, commands, label=None):
    gdb = shutil.which("arm-none-eabi-gdb") or shutil.which("gdb-multiarch")
    require(gdb, "Arm GDB available")
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        port = sock.getsockname()[1]
    label = label or elf.stem
    script = evidence / (label + ".gdb")
    script.write_text(f"""set pagination off
set confirm off
set tcp connect-timeout 10
set tcp auto-retry on
target remote 127.0.0.1:{port}
tbreak main
continue
{commands}
echo L13_GDB_PASS\n
detach
quit
""", encoding="utf-8")
    with (evidence / (label + "-gdb-uart.txt")).open("w") as output:
        process = subprocess.Popen(qemu(elf) + ["-S", "-gdb", f"tcp:127.0.0.1:{port}"],
                                   stdout=output, stderr=subprocess.STDOUT)
        try:
            try:
                result = subprocess.run([gdb, "-q", "-nx", "-batch", str(elf), "-x", str(script)],
                                        capture_output=True, text=True, timeout=30)
            except subprocess.TimeoutExpired as error:
                pieces = (error.stdout, error.stderr)
                partial = "".join(p.decode(errors="replace") if isinstance(p, bytes) else (p or "") for p in pieces)
                (evidence / (label + "-gdb.txt")).write_text(partial, encoding="utf-8")
                raise AssertionError(label + " GDB timeout; see evidence") from error
            log = result.stdout + result.stderr
            (evidence / (label + "-gdb.txt")).write_text(log, encoding="utf-8")
            require(result.returncode == 0 and "L13_GDB_PASS" in log, label + " GDB assertions")
            return log
        finally:
            stop(process)


def sync_debugger(build, evidence):
    commands = "tbreak *l13_exclusive_increment\ncontinue\nset $entry_sp = $sp\nset $return_pc = $lr & ~1\n"
    for reg in range(4, 12):
        commands += f"set $saved_r{reg} = $r{reg}\n"
    commands += """tbreak *$return_pc
tbreak *SysTick_Handler
continue
if (($xpsr & 511) != 15) || (($lr & 0x1C) != 0x18)
    quit 1
end
info registers sp lr xpsr
x/8wx $sp
continue
if ($sp != $entry_sp) || ($r0 != 1) || (g_l13_counter != 2) || (g_l13_first_status != 1) || (g_l13_retries != 1)
    quit 1
end
"""
    for reg in range(4, 12):
        commands += f"if $r{reg} != $saved_r{reg}\n    quit 1\nend\n"
    commands += """tbreak l13_done
continue
if (*(unsigned int*)0xE000ED28 != 0) || (*(unsigned int*)0xE000ED2C != 0) || ($xpsr & 511) != 0
    quit 1
end
"""
    debugger(build / "13-sync-fpu-dsp.elf", evidence, commands)


def fp_checks(build, evidence):
    for mode in ("basic", "eager", "lazy", "hard", "denied"):
        text = firmware(build / ("13-fpu-" + mode + ".elf"), evidence)
        expect(text, "CHECK_RESET_PERMISSION", "PASS")
        if mode == "denied":
            for key in ("NOCP", "USAGE_HANDLER", "DENIED_PC", "DENIED_BASIC"):
                expect(text, "CHECK_" + key, "PASS")
            expect(text, "CFSR", "0x00080000")
            expect(text, "HFSR", "0x00000000")
            expect(text, "EXC_RETURN", "0xFFFFFFF9")
        else:
            require("] FAIL" not in text, mode + " no unexpected FAIL")
            for key in ("PERMISSION", "FRAME_TYPE", "STACK_LAYOUT", "LAZY_STATE", "FP_PAYLOAD",
                        "FP_RESTORE", "FLOAT_ABI", "NO_FAULT"):
                expect(text, "CHECK_" + key, "PASS")
            expect(text, "FRAME_BYTES", 32 if mode == "basic" else 104)
            expect(text, "EXC_RETURN", "0xFFFFFFF9" if mode == "basic" else "0xFFFFFFE9")
            expect(text, "RAW_FP_SLOT_BEFORE", "0xA5A5A5A5" if mode == "lazy" else (
                "0x00000000" if mode == "basic" else "0x3FC00000"))
            expect(text, "ABI_ADD_BITS", "0x40600000")


def fp_debugger(build, evidence):
    for mode in ("basic", "eager", "lazy", "hard"):
        extended = mode != "basic"
        size = 104 if extended else 32
        slot = "0xA5A5A5A5" if mode == "lazy" else "0x3FC00000"
        commands = "tbreak *l13_fp_run\ncontinue\nset $call_sp = $sp\nset $return_pc = $lr & ~1\n"
        for reg in range(4, 12):
            commands += f"set $saved_r{reg} = $r{reg}\n"
        commands += f"""tbreak *SVC_Handler
continue
if (($xpsr & 511) != 11) || ($lr != {"0xFFFFFFE9" if extended else "0xFFFFFFF9"}) || (g_l13_before_sp - (unsigned int)$sp != {size})
    quit 1
end
if ((*(unsigned int*)0xE000EF34 & 1) != {1 if mode == "lazy" else 0})
    quit 1
end
set $frame = (unsigned int)$sp
if ($frame < (unsigned int)&__stack_limit) || (g_l13_before_sp > (unsigned int)&__stack_top)
    quit 1
end
x/{26 if extended else 8}wx $frame
"""
        if extended:
            commands += f"if *(unsigned int*)($frame + 32) != {slot}\n    quit 1\nend\n"
        commands += """tbreak *l13_frame_ready
continue
if ($sp != $frame - 8) || ((g_l13_fpccr_after & 1) != 0) || (($xpsr & 511) != 11)
    quit 1
end
"""
        if extended:
            commands += """if (*(unsigned int*)($frame + 32) != 0x3FC00000) || (g_l13_fp_saved[0] != 0x3FC00000) || ($s0 != 8.0)
    quit 1
end
x/26wx $frame
"""
        commands += """tbreak *(unsigned int)&l13_after_svc
continue
if ($sp != g_l13_before_sp) || (($xpsr & 511) != 0)
    quit 1
end
"""
        if extended:
            commands += "if ($s0 != 1.5) || ($s1 != 2.0) || ($s2 != 3.5)\n    quit 1\nend\n"
        commands += """tbreak *$return_pc
continue
if $sp != $call_sp
    quit 1
end
"""
        for reg in range(4, 12):
            commands += f"if $r{reg} != $saved_r{reg}\n    quit 1\nend\n"
        commands += """tbreak l13_done
continue
if (*(unsigned int*)0xE000ED28 != 0) || (*(unsigned int*)0xE000ED2C != 0)
    quit 1
end
"""
        debugger(build / ("13-fpu-" + mode + ".elf"), evidence, commands)
    debugger(build / "13-fpu-denied.elf", evidence, """tbreak *UsageFault_Handler
continue
if (($xpsr & 511) != 6) || ($lr != 0xFFFFFFF9) || (*(unsigned int*)0xE000ED28 != 0x80000) || (*(unsigned int*)($sp + 24) != ((unsigned int)&l13_fp_first & ~1))
    quit 1
end
x/8wx $sp
tbreak l13_done
continue
if (($xpsr & 511) != 6) || (g_l13_cfsr != 0x80000) || (g_l13_hfsr != 0)
    quit 1
end
""")


def abi_checks(evidence):
    # These are link probes, not executed firmware. Never suppress ABI errors.
    compiler = shutil.which("arm-none-eabi-gcc")
    require(compiler, "Arm GCC available for ABI probes")
    flags = [compiler, "-mcpu=cortex-m33", "-mthumb", "-mfpu=fpv5-sp-d16", "-O2", "-ffreestanding"]
    with tempfile.TemporaryDirectory() as temp:
        folder = Path(temp)
        caller = folder / "caller.c"
        caller.write_text("extern float l13_abi_add(float,float);\nfloat caller(void) { return l13_abi_add(1.5f,2.0f); }\n")
        for abi in ("softfp", "hard"):
            for name, source in (("caller", caller), ("add", ROOT / "lessons/13-sync-fpu-dsp/example/abi_add.c")):
                subprocess.run(flags + ["-mfloat-abi=" + abi, "-c", str(source), "-o", str(folder / (name + abi + ".o"))],
                               check=True, capture_output=True, text=True)
        for caller_abi in ("softfp", "hard"):
            for add_abi in ("softfp", "hard"):
                result = subprocess.run(flags + ["-mfloat-abi=" + caller_abi, "-nostdlib", "-Wl,-e,caller",
                    str(folder / ("caller" + caller_abi + ".o")), str(folder / ("add" + add_abi + ".o")),
                    "-o", str(folder / "probe.elf")], capture_output=True, text=True)
                log = result.stdout + result.stderr
                (evidence / ("abi-" + caller_abi + "-" + add_abi + ".txt")).write_text(log, encoding="utf-8")
                if caller_abi == add_abi:
                    require(result.returncode == 0, "matching " + caller_abi + " ABI links")
                else:
                    require(result.returncode != 0 and "VFP register arguments" in log,
                            "mixed " + caller_abi + "/" + add_abi + " ABI rejected")


def code_checks(build, evidence):
    def command(args):
        return subprocess.check_output(args, text=True, stderr=subprocess.STDOUT)
    commands = json.loads((build.parents[1] / "compile_commands.json").read_text())
    for mode in ("basic", "eager", "lazy", "hard", "denied"):
        entries = [c for c in commands if f"13-fpu-{mode}.dir" in c["command"]]
        require(entries, mode + " compile commands present")
        abi = "hard" if mode == "hard" else "softfp"
        require(all(re.findall(r"-mfloat-abi=(\w+)", c["command"])[-1] == abi for c in entries),
                mode + " all driver/platform objects share " + abi + " ABI")
        require(all("-mgeneral-regs-only" in c["command"] for c in entries),
                mode + " setup/reporters restricted to general registers")
        attrs = command(["arm-none-eabi-readelf", "-A", str(build / ("13-fpu-" + mode + ".elf"))])
        require(("Tag_ABI_VFP_args: VFP registers" in attrs) == (abi == "hard"), mode + " ELF ABI attribute")
        (evidence / ("13-fpu-" + mode + "-attributes.txt")).write_text(attrs, encoding="utf-8")
    for abi in ("softfp", "hard"):
        obj = build / f"CMakeFiles/l13-abi-{abi}.dir/example/abi_add.c.obj"
        dis = command(["arm-none-eabi-objdump", "-d", str(obj)])
        require("vadd.f32" in dis, abi + " C helper actually emits hardware FP")
        (evidence / ("abi-add-" + abi + "-disassembly.txt")).write_text(dis, encoding="utf-8")
    elf = str(build / "13-sync-fpu-dsp.elf")
    dis = command(["arm-none-eabi-objdump", "-d", elf])
    require(all(m in dis for m in ("ldrex", "strex", "clrex", "dmb", "smlad", "qadd")),
            "exclusive/barrier/DSP instructions emitted (not a timing claim)")
    (evidence / "sync-disassembly.txt").write_text(dis, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("preset", choices=("debug", "release", "minsize"))
    args = parser.parse_args()
    build = ROOT / "build" / args.preset / "lessons/13-sync-fpu-dsp"
    evidence = ROOT / "build" / args.preset / "lesson13-evidence"
    evidence.mkdir(parents=True, exist_ok=True)
    sync_checks(build, evidence)
    sync_debugger(build, evidence)
    fp_checks(build, evidence)
    fp_debugger(build, evidence)
    abi_checks(evidence)
    code_checks(build, evidence)
    print("Lesson 13", args.preset, "PASS; evidence:", evidence)


if __name__ == "__main__":
    main()
