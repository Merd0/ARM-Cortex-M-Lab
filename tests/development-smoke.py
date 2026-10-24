#!/usr/bin/env python3
"""Fast development gate for Lessons 14-16; comprehensive audit remains separate."""
import argparse
from pathlib import Path
import re
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]


def firmware(elf, evidence, extra=()):
    path = evidence / (elf.stem + ".txt")
    with path.open("w") as output:
        process = subprocess.Popen(["qemu-system-arm", "-M", "mps2-an505", "-cpu", "cortex-m33",
            "-nographic", "-monitor", "none", "-serial", "stdio", "-kernel", str(elf), *extra],
            stdout=output, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + 8
            while time.monotonic() < deadline:
                text = path.read_text(errors="replace")
                if "===== END =====" in text:
                    return text
                if process.poll() is not None:
                    break
                time.sleep(0.02)
            raise AssertionError("no END: " + str(path))
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()


def verify(elf, evidence, failed=(), values=None, extra=(), required=()):
    text = firmware(elf, evidence, extra)
    checks = re.findall(r"^\[(CHECK_\w+)\] (PASS|FAIL)$", text, re.M)
    assert len(checks) >= 4 and len({key for key, _ in checks}) == len(checks), elf.name
    assert {key for key, _ in checks} == set(required), text
    assert {key for key, value in checks if value == "FAIL"} == set(failed), text
    for key, value in (values or {}).items():
        assert re.search(r"^\[" + re.escape(key) + r"\] " + re.escape(str(value)) + "$", text, re.M), text
    print("PASS:", elf.stem, "expected failures:", ",".join(failed) or "none", flush=True)
    return text


def lesson14(build, evidence):
    folder = build / "lessons/14-systick-pendsv"
    for name, failed in (("14-systick-pendsv", ()), ("14-pendsv-broken", ("CHECK_PRIORITY", "CHECK_DEFERRED")),
                         ("14-exercise-priority", ("CHECK_PRIORITY",)), ("14-solution-priority", ())):
        verify(folder / (name + ".elf"), evidence, failed,
               {"TICKS": 4, "EVENT0": 1, "EVENT1": 3 if "broken" in name else 2,
                "EVENT2": 2 if "broken" in name else 3},
               required=("CHECK_PRIORITY", "CHECK_DEFERRED", "CHECK_PENDING_COALESCES",
                         "CHECK_TICK_RELOAD", "CHECK_TICK_PROGRESS", "CHECK_THREAD_MSP", "CHECK_NO_FAULT"))


def lesson15(build, evidence):
    folder = build / "lessons/15-mini-kernel"
    for name, failed in (("15-mini-kernel", ()), ("15-kernel-broken", ("CHECK_REGISTERS",)),
                         ("15-exercise-frame", ("CHECK_INITIAL_FRAME",)), ("15-solution-frame", ())):
        checks = ("CHECK_INITIAL_FRAME", "CHECK_STACK_GUARDS", "CHECK_NO_FAULT")
        checks += (("CHECK_NO_LAUNCH",) if "exercise" in name else
                   ("CHECK_TASK_PROGRESS", "CHECK_REGISTERS", "CHECK_PREEMPTION", "CHECK_YIELD", "CHECK_HANDLER_MSP", "CHECK_TASK_STACKS"))
        verify(folder / (name + ".elf"), evidence, failed, required=checks)


def lesson16(build, evidence):
    folder = build / "lessons/16-trustzone-advanced"
    for name in ("16-trustzone", "16-invalid-entry", "16-exercise-policy", "16-solution-policy"):
        checks = ("CHECK_PARTITION", "CHECK_NS_VECTORS", "CHECK_SECRET", "CHECK_SECURE_HANDLER", "CHECK_NO_ESCALATION")
        invalid = name == "16-invalid-entry"
        checks += (("CHECK_INVALID_ENTRY",) if invalid else
                   ("CHECK_NS_STARTUP", "CHECK_SERVICE", "CHECK_POLICY_DENY", "CHECK_CALLER_NS", "CHECK_SECURE_REJECT"))
        text = verify(folder / (name + "-secure.elf"), evidence,
               ("CHECK_SERVICE",) if "exercise" in name else (),
               {"SFSR": "0x00000001", "SFAR": "INVALID"} if invalid else {},
               ("-device", "loader,file=" + str(folder / (name + "-nonsecure.elf"))), required=checks)
        if not invalid:
            status = int(re.search(r"^\[SFSR\] (0x[0-9A-F]+)$", text, re.M)[1], 16)
            assert status in (0x08, 0x48), text
            address = "0x30000000" if status & 0x40 else "INVALID"
            assert "[SFAR] " + address in text, text


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("lesson", choices=("14", "15", "16", "all"))
    parser.add_argument("--preset", choices=("debug", "release", "minsize"), default="debug")
    args = parser.parse_args()
    build = ROOT / "build" / args.preset
    evidence = build / "development-evidence"
    evidence.mkdir(parents=True, exist_ok=True)
    for lesson, run in (("14", lesson14), ("15", lesson15), ("16", lesson16)):
        if args.lesson in (lesson, "all"):
            run(build, evidence)


if __name__ == "__main__":
    main()
