**Language:** English | [Türkçe](TOOL_VERSIONS.tr.md)

# Tool Versions

Why it matters: changing the compiler version changes the generated assembly.
If a lesson says "you will see this instruction," the version used to verify
that observation must be recorded.

## Bootstrap verification environment

> **Note:** The verification below was performed on **Linux (Ubuntu 24.04 x86_64)**.
> Even with the same versions on Windows, compiler output can differ slightly.
> Add your own Windows versions to the table below.

| Tool | Version | Source |
|---|---|---|
| arm-none-eabi-gcc | 13.2.1 20231009 (package 15:13.2.rel1-2) | Ubuntu 24.04 `gcc-arm-none-eabi` |
| arm-none-eabi-binutils (as/ld/objdump) | 2.42 | Ubuntu 24.04 |
| qemu-system-arm | 8.2.2 (Debian 1:8.2.2+ds-0ubuntu1.18) | Ubuntu 24.04 |
| cmake | 3.28.3 | Ubuntu 24.04 |
| ninja | 1.11.1 | Ubuntu 24.04 |
| gdb-multiarch | 15.1 | Ubuntu 24.04 |
| OS | Ubuntu 24.04.4 LTS, x86_64 | - |

`arm-none-eabi-gdb` was not included in the package in this environment;
`gdb-multiarch` was used. Both are sufficient for Cortex-M remote debugging.

## Record your own environment

```bash
./scripts/check-environment.sh | tee docs/tool-versions-local.txt
```

```powershell
.\scripts\check-environment.ps1 | Tee-Object docs\tool-versions-local.txt
```

## Windows local audit environment — 2026-10-27

| Tool | Version | Date |
|---|---|---|
| arm-none-eabi-gcc | xPack 15.2.1 20251203 | 2026-10-27 |
| qemu-system-arm | 11.1.0 (e470268ff4) | 2026-10-27 |
| cmake | 4.3.2 | 2026-10-27 |
| ninja | 1.13.2 | 2026-10-27 |
| arm-none-eabi-gdb | 16.3.90.20250906-git | 2026-10-27 |

## Vendored library versions

| Component | Version | Source | License |
|---|---|---|---|
| CMSIS-Core(M) | 6.3 | ARM-software/CMSIS_6, `main`, 2026-09-19 | Apache-2.0 |

Details and update steps: [`external/README.md`](../external/README.md).
