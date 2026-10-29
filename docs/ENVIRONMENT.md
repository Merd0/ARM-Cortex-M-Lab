**Language:** English | [Türkçe](ENVIRONMENT.tr.md)

# Environment Setup

Goal: build, run, and debug Cortex-M33 firmware without a physical board.

---

## Verification environments

Bootstrap was performed in a Linux container. Ubuntu 24.04 GitHub Actions passed
three presets for project/final integration commit `11fde900`. Local engineering
verification runs on Windows with native Arm GCC/QEMU/GDB and MSYS2 Bash;
the [final gate record](ENGINEERING_AUDIT.md) shows local results separately.

- Windows: native CMake/Ninja builds, Python QEMU/GDB gates;
  Bash tests run through MSYS2. `scripts/check-environment.ps1` ran on
  2026-10-27 and found all tools.
- Ubuntu: build, QEMU, GDB, and regression gates in CI.
- WSL2 and macOS: not verified in this final gate. The absence of a WSL service
  does not prevent native Windows testing.
- The interactive VS Code tasks/launch flow and package installation from scratch
  were not tested in this audit; not all `.ps1` tools should be considered tested.

Versions actually used are in [Tool Versions](TOOL_VERSIONS.md).
Model differences between QEMU versions are not physical Cortex-M33 guarantees.

---

## Five things you need

**Arm GNU Toolchain · CMake · Ninja · QEMU · an editor (VS Code)**

All are free; installation takes ~15 minutes.

---

# Route A — Native Windows installation

## A1. Arm GNU Toolchain

Official download page:
<https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>

- Target: **`arm-none-eabi`** (AArch32 bare-metal)
- The `.exe` installer is easiest on Windows
- Check **"Add path to environment variable"** during installation

Alternatively, use winget:

```powershell
winget install Arm.GnuArmEmbeddedToolchain
```

> Arm may change the winget package name. If it is not found,
> try `winget search arm-none-eabi`; otherwise use the official installer.

## A2. CMake, Ninja, Git

```powershell
winget install Kitware.CMake
winget install Ninja-build.Ninja
winget install Git.Git
```

## A3. QEMU

```powershell
winget install SoftwareFreedomConservancy.QEMU
```

If the winget version causes trouble, official Windows binaries are available at:
<https://www.qemu.org/download/#windows>

If the QEMU directory (typically `C:\Program Files\qemu`) is not on PATH:

```powershell
[Environment]::SetEnvironmentVariable(
    "Path",
    [Environment]::GetEnvironmentVariable("Path", "User") + ";C:\Program Files\qemu",
    "User")
```

After changing PATH, **close and reopen the terminal.**

## A4. VS Code

```powershell
winget install Microsoft.VisualStudioCode
```

When you open the repository, VS Code will suggest the recommended extensions
(`.vscode/extensions.json`): C/C++ and CMake Tools.

## A5. Verify the installation

In a new PowerShell window:

```powershell
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
cmake --version
ninja --version
qemu-system-arm --version
```

If all five print versions, you are ready. The repository script does the same:

```powershell
.\scripts\check-environment.ps1
```

> This script passed in the current Windows environment. If a tool is missing
> in another installation, check PATH and versions using the commands above.

## A6. Build and run

```powershell
git clone <repo-url>
cd ARM-Cortex-M-Lab

cmake --preset debug
cmake --build --preset debug
```

Expected: `--print-memory-usage` output and a `size` table for each firmware.

```powershell
cmake --build --preset debug --target run-02-programmer-model
```

Exit QEMU: **Ctrl-A**, then **X**.

> Some terminals swallow Ctrl-A. It works in Windows Terminal.

## A7. Debug

Two terminals are required.

Terminal 1 — QEMU waits halted for GDB:

```powershell
cmake --build --preset debug --target debug-02-programmer-model
```

Terminal 2 — GDB connects:

```powershell
arm-none-eabi-gdb build\debug\lessons\02-programmer-model\02-programmer-model.elf
```

Inside GDB:

```gdb
set architecture armv8-m.main
target remote :3333
source scripts/gdb/cortex-m.gdb
cm-mode
b main
continue
```

From VS Code: **F5** → "Attach to QEMU (lesson 02)". This route was not tested on
Windows either; if it fails, the two-terminal method above always works — and
is the more instructive approach anyway.

---

# Route B — WSL2 (closest to the verified environment)

If something does not work on Windows, this is how to set up the same environment
used for verification.

```powershell
wsl --install -d Ubuntu-24.04
```

Inside WSL:

```bash
sudo apt-get update
sudo apt-get install -y \
    gcc-arm-none-eabi binutils-arm-none-eabi \
    qemu-system-arm cmake ninja-build gdb-multiarch git
```

Verify and run:

```bash
./scripts/check-environment.sh
cmake --preset debug
cmake --build --preset debug
./tests/smoke-test.sh
```

Result of this command sequence in the container: **26 PASS / 0 FAIL.**

## WSL notes

- `arm-none-eabi-gdb` is not in the Ubuntu repositories; use `gdb-multiarch`.
  It makes no difference for this repository — all GDB verification used it:
  ```bash
  gdb-multiarch -q build/debug/lessons/02-programmer-model/02-programmer-model.elf
  (gdb) set architecture armv8-m.main
  (gdb) target remote :3333
  ```
- Keep code in the WSL filesystem (`~/`), not on the Windows disk (`/mnt/c/...`).
  Builds under `/mnt/c` are noticeably slower.
- With VS Code + the "WSL" extension, you can open the WSL folder directly:
  editor on Windows, toolchain on Linux.

---

## Troubleshooting

| Symptom | Likely cause | Solution |
|---|---|---|
| `arm-none-eabi-gcc: command not found` | Not added to PATH | Reopen the terminal; if necessary, add the installation's `bin` directory to PATH |
| `CMAKE_C_COMPILER not found` | Toolchain not on PATH | `cmake -DARM_TOOLCHAIN_BIN=C:/path/to/bin --preset debug` |
| `Ninja generator not found` | Ninja not installed | `winget install Ninja-build.Ninja` |
| `unsupported machine type` | QEMU too old | `qemu-system-arm -M help \| findstr mps2` → `mps2-an505` must appear in the list |
| QEMU opens without output | Missing `-serial stdio` | Use the repository's `run-*` targets |
| GDB `Remote connection closed` | Connected before QEMU started | Ensure Terminal 1 is waiting |
| Inconsistent register values in GDB | Architecture not set | `set architecture armv8-m.main` |
| `.ps1 cannot be loaded` | Execution policy | `powershell -ExecutionPolicy Bypass -File scripts\check-environment.ps1` |

---

## After setup

Compare the verified Windows versions in `docs/TOOL_VERSIONS.md` with your own
output. Record results from a different environment separately; do not replace
measured version records with assumptions.

---

[← Home](../README.md) · [Tool versions](TOOL_VERSIONS.md) · [Known limitations](LIMITATIONS.md)
