**Language:** English | [Türkçe](README.tr.md)

# Exercise 01 — Add an SVC Service

**Goal:** Build the path that turns an unprivileged request into a privileged service.

## Task

Add a `case` for **`SVC #2`** to `svc_dispatch()` in `main.c`.

```c
#define SVC_GET_VERSION      2u
```

Service: `get_service_version()` — returns a fixed version/capability value.

| Requirement | Note |
|---|---|
| Request originates in unprivileged Thread Mode | Already called this way |
| Dispatcher recognizes `SVC #2` | Your added `case` |
| Service runs in privileged Handler Mode | Dispatcher already runs there |
| Return value is `0x00010002` | |
| Thread continues normally | |

## Build & Run

```bash
cmake --build --preset debug --target 05-exercise-svc2
./scripts/run-qemu.sh build/debug/lessons/05-privilege-svc/05-exercise-svc2.elf
```

Initial output (actual):

```text
[  UNKNOWN_SVC] 2
...
[6] SVC #2 (get_service_version)
[  SVC2_VERSION] 0xFFFFFFFF
[  CHECK_SVC2_VERSION] FAIL
```

Currently, `SVC #2` matches no `case` and takes the `default` branch.
This is the **expected initial state**, not a bug.

## Verification

`CHECK_SVC2_VERSION` must become `PASS`, **and** every other `CHECK_*` line
must remain `PASS`:

```text
CHECK_PRIV_DROPPED
CHECK_SELF_PROMOTION_BLOCKED
CHECK_PERIPHERAL_BLOCKED_UNPRIV
CHECK_SVC_SERVICE
CHECK_SVC_DID_NOT_PROMOTE
CHECK_SVC2_VERSION
CHECK_PRIV_RESTORED_BY_HANDLER
```

## The real question

**How** will you deliver the result to Thread?

You cannot use `return`: `svc_dispatch` returns to **exception return**,
not Thread (see why `svc_handler.S` uses `b` rather than `bl`). The `R0`
seen by Thread is the value restored from the exception frame.

Look at `SVC_INCREMENT`: where does it write the result, and when does
hardware restore that word into `R0`?

## Verify in GDB

```gdb
break svc_dispatch
continue
# SVC #2 cagrisina kadar ilerle
p svc_number
p/x frame->r0
```

After implementing the service, on return:

```gdb
info registers r0
```

## If you are stuck

Reference solution: [`../../solutions/01-add-svc-service/`](../../solutions/01-add-svc-service/)

---

[← Lesson 05](../../README.md)
