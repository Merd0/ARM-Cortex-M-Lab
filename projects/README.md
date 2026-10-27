**Language:** English | [Türkçe](README.tr.md)

# Projects

`lessons/` isolates one concept. `projects/` **combines several lessons** to build
something that works. The distinction matters:

| | `lessons/` | `projects/` |
|---|---|---|
| Scope | One concept | Combined behavior |
| Code size | Minimal | Moderate |
| Goal | Understand | Prove you can build it |
| When | First encountering a topic | After completing the relevant lessons |

Do not start a project before completing the lessons it depends on.
The dependency table is below.

---

## Project list

| Project | What it does | Required lessons | Status |
|---|---|---|---|
| [protected-application](protected-application/) | Unprivileged thread + PSP + MPU + SVC | 04, 05, 06, 07, 08 | On main; three-preset CI PASS |
| [fault-analyzer](fault-analyzer/) | Turns a crash into a readable report | 07, 08 | On main; three-preset CI PASS |
| [from-reset-to-main](from-reset-to-main/) | Startup + linker script from scratch | 10, 11 | On main; three-preset CI PASS |
| [performance-profiler](performance-profiler/) | Measurement methodology + optimization comparison | 09, 12 | On main; three-preset CI PASS |
| [mini-kernel](mini-kernel/) | SysTick + PendSV + SVC round-robin scheduler | 14, 15 | On main; three-preset CI PASS |

All of them combined: [`final-project/`](../final-project/)

---

## Shared rule: student vs infrastructure

The projects' src/ directories contain working reference applications. The same
mechanism is not copied solely to create a separate project view: Lesson 06, 08,
11, 12, and 15 sources are recompiled in the relevant targets. `TODO(student)`
exercises and separate solutions are preserved under lessons/; reference project
code is not leaked into the starter.

When you finish, you can say `review mode` — I will review the code for
architectural correctness, AAPCS, register preservation, stack alignment, volatile,
privilege, and MPU usage; I will first state the problem and its severity, rather
than writing the solution directly.

---

## Project template

Each project directory contains:

```text
<proje>/
    README.md        problem definition + architecture diagram + acceptance criteria
    src/             working reference integration
    CMakeLists.txt   add_firmware() call
```

At a minimum, `README.md` contains:

- **Problem** — what real need does it address?
- **Architecture** — a mermaid diagram
- **Acceptance criteria** — what "done" means in measurable terms
- **Lesson dependencies**
- **Verification** — QEMU output and what should be visible in GDB

---

[← Home](../README.md) · [Bootcamp](../BOOTCAMP.md)

## Shared verification

```bash
cmake --preset debug
cmake --build --preset debug
python tests/system-smoke.py --group projects
python tests/profile-report.py
```

Each project produces an independent ELF. System smoke verifies all expected
CHECK fields in 15 scenarios; the detailed record is in [Progress](../docs/PROGRESS.md).
