#!/usr/bin/env python3
"""Lesson 12 behavioral, debugger and emitted-code checks; Python stdlib only.

Run from any directory after building the selected preset. QEMU processes are
owned by this script and always terminated. Evidence stays in ignored build/.
GDB watchpoints use QEMU's gdbstub, NOT a claim of working DWT comparators.
"""
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


def run(args):
    return subprocess.check_output(args, cwd=ROOT, text=True, stderr=subprocess.STDOUT)


def require(condition, description):
    if not condition:
        raise AssertionError(description)
    print("PASS:", description, flush=True)


def qemu_command(elf):
    return ["qemu-system-arm", "-M", "mps2-an505", "-cpu", "cortex-m33",
            "-nographic", "-monitor", "none", "-serial", "stdio", "-kernel", str(elf)]


def stop(process):
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()


def firmware(elf, evidence):
    path = evidence / (elf.stem + "-uart.txt")
    with path.open("w") as output:
        process = subprocess.Popen(qemu_command(elf), stdout=output, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + 10
            while time.monotonic() < deadline:
                text = path.read_text(errors="replace")
                if "===== END =====" in text:
                    return text
                if process.poll() is not None:
                    break
                time.sleep(0.02)
            raise AssertionError("firmware did not reach END: " + str(path))
        finally:
            stop(process)


def debugger(elf, evidence, expected, stall_timer=False, expect_io=False):
    gdb = shutil.which("arm-none-eabi-gdb") or shutil.which("gdb-multiarch")
    require(gdb, "Arm-capable GDB available")
    with socket.socket() as port_socket:
        port_socket.bind(("127.0.0.1", 0))
        port = port_socket.getsockname()[1]
    label = elf.stem + ("-stopped-timer" if stall_timer else "")
    script = evidence / (label + ".gdb")
    # QEMU 8.2's debug-memory write path skips MMIO. Request the stop in RAM;
    # the handler performs real CPU MMIO writes. Never fake ticks/iterations.
    timer_control = """tbreak *SysTick_Handler
continue
if (($xpsr & 511) != 15) || (g_l12_ticks != 0) || (g_l12_stop_timer != 0)
    quit 1
end
set variable g_l12_stop_timer = 1
if g_l12_stop_timer != 1
    quit 1
end
""" if stall_timer else ""
    timer_failure = "(g_l12_ticks != 1) || (g_l12_iterations != 200000)" if stall_timer else "g_l12_ticks < 4"
    script.write_text(f"""set pagination off
set confirm off
set tcp connect-timeout 10
set tcp auto-retry on
target remote 127.0.0.1:{port}
tbreak main
continue
watch -l g_l12_watch
continue
bt
info frame
x/8wx $sp
disassemble /r l12_leaf
if g_l12_watch != {expected}
    quit 1
end
echo WRITE_WATCH_PASS\n
delete breakpoints
rwatch -l g_l12_watch
continue
if g_l12_watch != {expected}
    quit 1
end
echo READ_WATCH_PASS\n
delete breakpoints
set $previous_pc = $pc
stepi
if $pc == $previous_pc
    quit 1
end
echo SINGLE_STEP_PASS\n
{timer_control}break console_putc if (*(unsigned int*)0xE000E010 & 1) != 0
tbreak l12_done
continue
if (*(unsigned int*)0xE000E010 & 1) != 0
    echo UNEXPECTED_IO_IN_WINDOW\n
    quit 1
end
if (*(unsigned int*)0xE000ED28 != 0) || (*(unsigned int*)0xE000ED2C != 0)
    quit 1
end
if ((*(unsigned int*)0xE000E010 & 3) != 0) || (g_l12_sink != 424) || ({timer_failure})
    quit 1
end
if ($xpsr & 511) != 0
    quit 1
end
echo DONE_NO_FAULT_TIMER_OFF_PASS\n
detach
quit
""", encoding="utf-8")
    uart_path = evidence / (label + "-gdb-uart.txt")
    with uart_path.open("w") as output:
        process = subprocess.Popen(qemu_command(elf) + ["-S", "-gdb", f"tcp:127.0.0.1:{port}"],
                                   stdout=output, stderr=subprocess.STDOUT)
        try:
            result = subprocess.run([gdb, "-q", "-nx", "-batch", str(elf), "-x", str(script)],
                                    capture_output=True, text=True, timeout=30)
            log = result.stdout + result.stderr
            (evidence / (label + "-gdb.txt")).write_text(log, encoding="utf-8")
            if expect_io:
                require(result.returncode == 1 and "UNEXPECTED_IO_IN_WINDOW" in log
                        and "console_putc" in log and "WRITE_WATCH_PASS" in log,
                        "GDB independently rejects console I/O while SysTick is enabled")
                return
            require(result.returncode == 0 and all(s in log for s in (
                "Hardware watchpoint", "Hardware read watchpoint", "Old value = 0",
                f"New value = {expected}", "WRITE_WATCH_PASS",
                "READ_WATCH_PASS", "SINGLE_STEP_PASS", "DONE_NO_FAULT_TIMER_OFF_PASS")),
                label + " GDB write/read watchpoints, call chain, step and completion")
            frames = re.findall(r"^#\d+\s+.*", log, re.M)
            require(all(any(name in frame for frame in frames)
                        for name in ("l12_leaf", "l12_middle", "l12_outer", "main")),
                    label + " complete four-function backtrace")
            if stall_timer:
                text = uart_path.read_text(errors="replace")
                require("[CHECK_TIMER_PROGRESS] FAIL" in text and "===== END =====" in text,
                        "stopped timer rejected without hanging (bounded loop)")
        finally:
            stop(process)


def code_checks(build, evidence):
    for name in ("12-profile-o0", "12-profile-o2"):
        elf = build / (name + ".elf")
        symbols = run(["arm-none-eabi-nm", "-S", str(elf)])
        match = re.search(r"^\w+\s+(\w+)\s+T\s+l12_workload$", symbols, re.M)
        require(match is not None, name + " retains workload symbol")
        dis = run(["arm-none-eabi-objdump", "-d", "--no-show-raw-insn",
                   "--disassemble=l12_workload", str(elf)])
        count = len(re.findall(r"^\s*[0-9a-f]+:\s+[a-z][\w.]*\s", dis, re.M))
        require(count > 0, name + " has disassembled instructions")
        (evidence / (name + "-workload.txt")).write_text(dis, encoding="utf-8")
        print(name, "workload bytes =", int(match[1], 16), "static instructions =", count)

    commands = json.loads((build.parents[1] / "compile_commands.json").read_text())
    for opt in ("o0", "o2"):
        entries = [c for c in commands if f"l12-work-{opt}.dir" in c["command"]]
        require(len(entries) == 1, "one workload object at " + opt)
        flags = re.findall(r"(?<!\S)-O[\w]+", entries[0]["command"])
        require(flags[-1] == "-O" + opt[1], "effective workload optimization " + opt)
    # The two firmware drivers keep the same optimization as the preset.
    opts = []
    for opt in ("o0", "o2"):
        entries = [c for c in commands if f"12-profile-{opt}.dir" in c["command"]
                   and c["file"].replace("\\", "/").endswith("/example/main.c")]
        require(len(entries) == 1, "one measurement driver at " + opt)
        opts.append(re.findall(r"(?<!\S)-O[\w]+", entries[0]["command"])[-1])
    preset_opt = {"debug": "-Og", "release": "-O2", "minsize": "-Os"}[build.parents[1].name]
    require(opts == [preset_opt, preset_opt], "both drivers retain preset optimization")

    with tempfile.TemporaryDirectory() as tmp:
        for function in ("l12_read_cycle", "l12_read_ipsr"):
            sections = []
            for api in ("cmsis", "teaching"):
                obj = build / f"CMakeFiles/l12-api-{api}.dir/example/header_probe.c.obj"
                binary = Path(tmp) / (api + ".bin")
                run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text." + function,
                     str(obj), str(binary)])
                sections.append(binary.read_bytes())
            require(bool(sections[0]) and sections[0] == sections[1],
                    function + " identical CMSIS/teaching emitted bytes")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("preset", choices=("debug", "release", "minsize"), default="debug", nargs="?")
    args = parser.parse_args()
    build = ROOT / "build" / args.preset / "lessons/12-debug-dwt"
    evidence = ROOT / "build" / args.preset / "lesson12-evidence"
    evidence.mkdir(parents=True, exist_ok=True)
    for name in ("12-debug-dwt", "12-debug-dwt-broken", "12-profile-io", "12-profile-o0", "12-profile-o2"):
        text = firmware(build / (name + ".elf"), evidence)
        checks = re.findall(r"^\[(CHECK_\w+)\] (PASS|FAIL)$", text, re.M)
        expected_fail = {"CHECK_WATCH_VALUE"} if name.endswith("broken") else (
            {"CHECK_CLEAN_WINDOW"} if name == "12-profile-io" else set())
        required_checks = {"CHECK_WATCH_VALUE", "CHECK_WORKLOAD", "CHECK_TIMER_CONFIG",
                           "CHECK_TIMER_PROGRESS", "CHECK_CLEAN_WINDOW", "CHECK_NO_FAULT"}
        require(len(checks) == 6 and {k for k, _ in checks} == required_checks
                and {k for k, v in checks if v == "FAIL"} == expected_fail,
                name + " exact expected checks")
        require("[CHECKSUM] 424" in text, name + " workload result preserved")
        require("[DWT_CHANGED] NO" in text and "[DWT_T0] 0" in text and "[DWT_T1] 0" in text,
                name + " current QEMU DWT observation (re-audit if target changes)")
        require("[CYCLE_BENCHMARK] NOT VALIDATED ON QEMU" in text, name + " no invented cycle measurement")
    for name, expected in (("12-exercise-measurement", 0), ("12-solution-measurement", 12)):
        text = firmware(build / (name + ".elf"), evidence)
        checks = re.findall(r"^\[[A-Z_]+\] (PASS|FAIL)$", text, re.M)
        require(len(checks) == 12 and checks.count("PASS") == expected
                and f"[CASES_PASSED] {expected}" in text, name + " all 12 synthetic cases")
    for name, expected in (("12-debug-dwt", 42), ("12-debug-dwt-broken", 43)):
        debugger(build / (name + ".elf"), evidence, expected)
    debugger(build / "12-debug-dwt.elf", evidence, 42, stall_timer=True)
    debugger(build / "12-profile-io.elf", evidence, 42, expect_io=True)
    code_checks(build, evidence)
    print("Lesson 12", args.preset, "PASS; evidence:", evidence)


if __name__ == "__main__":
    main()
