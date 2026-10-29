**Language:** English | [Türkçe](README.tr.md)

# external/ — vendored third-party sources

No file here is edited by hand. Doing so would remove its status as an "official
source" and make it impossible to update reliably.

---

## CMSIS-Core (Arm)

| | |
|---|---|
| Source | https://github.com/ARM-software/CMSIS_6 — `CMSIS/Core/Include` |
| Version header | CMSIS-Core(M) **6.3** (`__CM_CMSIS_VERSION_MAIN = 6`, `_SUB = 3`) |
| Retrieved ref | `main` (moving branch) |
| Download date | 2026-09-19 |
| Integrity record | [`CMSIS/SHA256SUMS.txt`](CMSIS/SHA256SUMS.txt) |
| License | Apache-2.0 — [`CMSIS/LICENSE`](CMSIS/LICENSE) |

This is the repository's **production / baseline core API**. Startup, system init,
platform drivers, `tools/`, and projects use it. The entry point is the CMSIS
device header:
[`platform/mps2-an505/include/platform/mps2_an505.h`](../platform/mps2-an505/include/platform/mps2_an505.h).

### Provenance — what was and was not verified

The audit correctly identified that the record does not contain an **immutable
upstream revision**. On 2026-09-20, I verified the following:

| Comparison | Result |
|---|---|
| Vendored `core_cm33.h` ↔ `main` (2026-09-20) | **Identical** (sha256 `7adc886f…`, 0-line diff) |
| Vendored `core_cm33.h` ↔ tag `v6.3.0` | **Different** — 37 lines; `main` includes `SCB_SFSR_*` definitions added after the tag |

Thus the snapshot is **a state of `main` after v6.3.0**, not a published tag.

**An immutable commit SHA could not be recorded.** Files were fetched through
`raw.githubusercontent.com` by branch name; the commit corresponding to that
request was not retained, and `main` may have advanced since that date. I am not
inventing a hash.

Instead, the available evidence was recorded: the sha256 digest of every vendored
file is in [`CMSIS/SHA256SUMS.txt`](CMSIS/SHA256SUMS.txt). This establishes **local
integrity** (verification succeeds if the files have not changed), but it does
not prove upstream identity.

To verify:

```bash
cd external/CMSIS && sha256sum -c SHA256SUMS.txt
```

**At the next update:** fetch from a tag (`v6.x.y`) or a known commit SHA and
record that reference here.

### Why vendored (not a submodule)

- Builds should not depend on network access; `git clone` + `cmake` should suffice.
- CI should not require an extra checkout step.
- The version should remain explicitly recorded in this file.

The cost is manual updates. The steps below serve that purpose.

### Copied files

Not all of CMSIS, only what is needed for Cortex-M33 (~490 KB):

```text
CMSIS/Core/Include/
    core_cm33.h              Cortex-M33 core peripheral access layer
    cmsis_version.h
    cmsis_compiler.h
    cmsis_gcc.h  cmsis_clang.h  cmsis_armclang.h  cmsis_iccarm.h
    m-profile/
        cmsis_gcc_m.h  cmsis_clang_m.h  cmsis_armclang_m.h  cmsis_iccarm_m.h
        armv8m_mpu.h  armv7m_mpu.h  armv7m_cachel1.h
        armv8m_pmu.h  armv81m_pac.h
CMSIS/LICENSE
```

A-profile and R-profile files and headers for other cores (`core_cm0.h`,
`core_cm55.h`, …) were deliberately excluded. If an M55 port is added,
`core_cm55.h` will also be added here.

### Updating

```bash
curl -L -o /tmp/cmsis.tar.gz \
  https://codeload.github.com/ARM-software/CMSIS_6/tar.gz/refs/heads/main
tar xzf /tmp/cmsis.tar.gz -C /tmp
cp -r /tmp/CMSIS_6-main/CMSIS/Core/Include/m-profile \
      external/CMSIS/Core/Include/
cp /tmp/CMSIS_6-main/CMSIS/Core/Include/{core_cm33.h,cmsis_*.h} \
      external/CMSIS/Core/Include/
cp /tmp/CMSIS_6-main/LICENSE external/CMSIS/LICENSE
```

Afterwards, **always** run:

```bash
./tests/smoke-test.sh
cd external/CMSIS && sha256sum -c SHA256SUMS.txt   # eski kayit artik gecersiz
```

and update the version/ref line in this file and `SHA256SUMS.txt`.
Use **a tag or commit SHA** instead of a branch where possible.

### Build integration

[`external/CMakeLists.txt`](CMakeLists.txt) defines one INTERFACE target:

```cmake
add_library(cmsis-core INTERFACE)
target_include_directories(cmsis-core SYSTEM INTERFACE ...)
```

`SYSTEM` matters: warnings from third-party headers should not drown out the
repository's own `-Wall -Wextra -Wshadow` warnings.

---

## License note

CMSIS is licensed under Apache-2.0 and this repository under MIT. They are
compatible; `external/CMSIS/LICENSE` is preserved unchanged and continues to
apply to the code in that directory.

---

[← Home](../README.md) · [Architecture](../docs/ARCHITECTURE.md)
