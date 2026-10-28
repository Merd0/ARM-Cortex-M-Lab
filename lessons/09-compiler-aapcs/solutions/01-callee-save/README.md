**Language:** English | [Türkçe](README.tr.md)

# Solution 01 — Repair an ABI-Broken Assembly Function

Exercise: [`../../exercises/01-callee-save/`](../../exercises/01-callee-save/)

Code: [`scale_and_sum.S`](scale_and_sum.S)

## Actual output

`debug`:

```text
[  CHECK_HARNESS_POSITIVE_CONTROL] PASS
[  RESULT] 36
[  MASK] 0x00000000
[  CORRUPTED] (yok)
[  SP_AT_BL] 0x30007FA8
[  CHECK_RESULT] PASS
[  CHECK_CALLEE_SAVED] PASS
[  CHECK_SP_ALIGNED_AT_CALL] PASS
===== END =====
```

In `release` and `minsize`, `SP_AT_BL = 0x30007F98`; verdicts are identical.

## Starter and solution side by side

```asm
/* BASLANGIC */                         /* COZUM */
push  {r4, lr}                          push  {r3, r4, r5, lr}
mov   r4, r0                            mov   r4, r0
mov   r5, r1                            mov   r5, r1
sub   sp, sp, #4
<SP kaydi>                              <SP kaydi>
mov   r0, r4                            mov   r0, r4
bl    helper_scale                      bl    helper_scale
add   sp, sp, #4
add   r0, r0, r5                        add   r0, r0, r5
pop   {r4, pc}                          pop   {r3, r4, r5, pc}
```

## Repair 1 — r5 must be preserved

`b` is held in **r5** across the call. r5 is **callee-saved**: it belongs to
the caller of `scale_and_sum`. If we use it, we must save it on entry and
restore it on exit.

The starter saved only `{r4, lr}`; the harness detected r5 corruption
(`MASK 0x00000002`, `CORRUPTED r5`).

Why not leave `b` in r1? Because r1 is **caller-clobbered**: `helper_scale`
may freely overwrite it. A value that must survive a call must be kept in
a callee-saved register or on the stack.

## Repair 2 — SP must be aligned to 8 at `BL`

SP is aligned to 8 on entry (measured at the harness's `BLX`). In the starter:

```text
push {r4, lr}   8 byte   -> aligned
sub  sp, #4     4 byte   -> at BL SP mod 8 = 4   (SP_AT_BL = 0x30007FAC)
```

The solution makes two changes:

- Removes the unnecessary `sub sp, #4` / `add sp, #4` pair — this function
  needs no local workspace.
- Registers to preserve are r4, r5, and LR — **three** registers, 12 bytes,
  not a multiple of 8. A **padding** register, r3, is therefore added to the
  list: four registers, 16 bytes in total.

Why r3? It is caller-clobbered, so saving and restoring it does no harm;
it only occupies space in the `push`/`pop` list. GCC uses the same pattern:
at `-Og`, it generated `push {r3, lr}` for `tail_call`. So does the harness:
`push {r3-r11, lr}`.

Alternative: `push {r4, r5, r6, lr}` also occupies 16 bytes and is correct;
it saves r6 unnecessarily, but harmlessly.

## LR and return

- **Why save LR?** `scale_and_sum` is not a leaf: `bl helper_scale` writes
  the address of the instruction after `BL` inside `scale_and_sum` to LR
  (with Thumb bit 0 = 1). Without saving the caller's return address, the
  function would try to “return” to this point inside itself, rather than
  to its caller.
- **Why `pop {…, pc}`?** The saved LR value is loaded directly into PC:
  restore and return in one instruction. `pop {r3, r4, r5, lr}` + `bx lr`
  is also correct; it is simply one instruction longer. Both preserve LR's
  bit 0 (Thumb).

## Why “the right result” was never proof

The starter produced `RESULT 36` — correct. However:

- Corrupting r5 would make the caller of `scale_and_sum` lose a value held
  in r5 **later**.
- Misaligned SP would give code in `helper_scale` (or something it calls)
  an invalid foundation if generated assuming an 8-byte-aligned stack.

Neither appears in `scale_and_sum`'s own result. The contract must be checked
separately from the result — that is why the harness exists.

---

[← Lesson 09](../../README.md)
