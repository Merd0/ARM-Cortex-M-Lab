#!/usr/bin/env python3
"""Check real Lesson 07 answer validation in disposable firmware copies.

Run from any directory: python3 lessons/07-exceptions/tests/check-answer-validation.py
Optional arguments select presets, for example: ... debug release minsize
Requires Git, CMake, Ninja, arm-none-eabi tools and qemu-system-arm on PATH.
The starter and solution in the working tree are never modified.
"""

import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import threading


ROOT = pathlib.Path(__file__).resolve().parents[3]
TARGETS = {
    "exercises": "07-exercise-decode",
    "solutions": "07-solution-decode",
}
CASES = [
    ("baseline", None, None),
    ("exact", "Thread", "PSP"),
    ("suffix", "ThreadX", "PSPX"),
    ("same-initial", "Timer", "Peripheral"),
    ("prefix", "T", "P"),
    ("empty", "", ""),
]
FIELDS = ("MODE", "STACK", "EXCNUM", "R0", "PC", "XPSR")


def command(args, cwd):
    result = subprocess.run(args, cwd=cwd, capture_output=True, text=True,
                            encoding="utf-8", errors="replace", timeout=120)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    return result.stdout


def firmware_output(elf):
    process = subprocess.Popen(
        ["qemu-system-arm", "-M", "mps2-an505", "-cpu", "cortex-m33",
         "-nographic", "-monitor", "none", "-serial", "stdio", "-kernel", str(elf)],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    lines = []
    finished = threading.Event()

    def read_output():
        for line in iter(process.stdout.readline, b""):
            lines.append(line)
            if b"===== END =====" in line:
                finished.set()
                break

    reader = threading.Thread(target=read_output, daemon=True)
    reader.start()
    try:
        finished.wait(timeout=8)
    finally:
        process.terminate()
        process.wait(timeout=5)
        reader.join(timeout=5)
        process.stdout.close()
    output = b"".join(lines).decode("utf-8", errors="replace")
    if "===== END =====" not in output:
        raise RuntimeError(f"Firmware did not complete: {elf}\n{output}")
    return output


def main():
    presets = sys.argv[1:] or ["debug", "release", "minsize"]
    if any(p not in ("debug", "release", "minsize") for p in presets):
        raise SystemExit("Expected debug, release, or minsize")
    files = command(["git", "ls-files", "-z"], ROOT).strip("\0").split("\0")
    passed = 0
    with tempfile.TemporaryDirectory(prefix="lesson07-answer-validation-") as temp:
        work = pathlib.Path(temp)
        for name in files:
            destination = work / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / name, destination)
        sources = {
            kind: work / "lessons/07-exceptions" / kind / "01-decode-exception-entry/main.c"
            for kind in TARGETS
        }
        originals = {kind: path.read_text(encoding="utf-8") for kind, path in sources.items()}
        for preset in presets:
            command(["cmake", "--preset", preset], work)
            for case, mode, stack in CASES:
                for kind, path in sources.items():
                    source = originals[kind]
                    if mode is not None:
                        for field, value in (("interrupted_mode", mode), ("frame_stack", stack)):
                            source, count = re.subn(
                                r"out->" + field + r"\s*=\s*[^;]+;",
                                f'out->{field} = "{value}";', source)
                            if count != 1:
                                raise RuntimeError(f"Expected one assignment: {kind}/{field}")
                    path.write_text(source, encoding="utf-8")
                command(["cmake", "--build", "--preset", preset, "--parallel", "4",
                         "--target", *TARGETS.values()], work)
                for kind, target in TARGETS.items():
                    elf = work / "build" / preset / "lessons/07-exceptions" / (target + ".elf")
                    output = firmware_output(elf)
                    checks = dict(re.findall(r"\[  (CHECK_\w+)\] (PASS|FAIL)", output))
                    for field in FIELDS:
                        expected = "FAIL" if kind == "exercises" else "PASS"
                        if case != "baseline" and field in ("MODE", "STACK"):
                            expected = "PASS" if case == "exact" else "FAIL"
                        if checks.get("CHECK_DECODE_" + field) != expected:
                            raise RuntimeError(f"{preset}/{kind}/{case}/{field}: expected {expected}\n{output}")
                    if any(value != "PASS" for key, value in checks.items()
                           if not key.startswith("CHECK_DECODE_")):
                        raise RuntimeError(f"Unrelated check failed: {preset}/{kind}/{case}\n{output}")
                    passed += 1
                    print(f"PASS {preset}/{kind}/{case}", flush=True)
    print(f"PASS: {passed} firmware runs")


if __name__ == "__main__":
    main()
