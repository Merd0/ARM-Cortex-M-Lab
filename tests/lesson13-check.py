#!/usr/bin/env python3
"""Lesson 13 runtime, debugger and ABI checks. Evidence stays in ignored build/."""
import argparse
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
            result = subprocess.run([gdb, "-q", "-nx", "-batch", str(elf), "-x", str(script)],
                                    capture_output=True, text=True, timeout=30)
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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("preset", choices=("debug", "release", "minsize"))
    args = parser.parse_args()
    build = ROOT / "build" / args.preset / "lessons/13-sync-fpu-dsp"
    evidence = ROOT / "build" / args.preset / "lesson13-evidence"
    evidence.mkdir(parents=True, exist_ok=True)
    sync_checks(build, evidence)
    sync_debugger(build, evidence)
    print("Lesson 13", args.preset, "PASS; evidence:", evidence)


if __name__ == "__main__":
    main()
