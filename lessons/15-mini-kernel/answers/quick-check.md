**Language:** English | [Türkçe](quick-check.tr.md)

# Quick Check answers

1. The basic hardware frame saves caller-clobbered registers. The scheduler must
   preserve the suspended task's callee-saved registers.
2. STM is not addressed through SP. Address bounds must be checked before the first store.
3. That frame was not established on PSP; changing the stack selection and continuing
   the old C function looks for local variables/return addresses in different memory.
4. Both tasks progressing after the yields finish, and timer switches whose saved PC
   is outside the yield return point. Counting ticks alone does not prove task switching.
