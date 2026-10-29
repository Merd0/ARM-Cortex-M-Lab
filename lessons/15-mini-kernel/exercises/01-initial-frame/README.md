**Language:** English | [Türkçe](README.tr.md)

# Exercise — Initial task frame

Complete only the `frame.c` TODO. The top is 8-byte aligned and 17 words are available.
The software part is R4–R11 + EXC_RETURN; the hardware part is R0–R3, R12, LR, PC, xPSR.
R0 must hold the task identity; the entry must start in Thumb state, and task return
must go to exit. Do not confuse frame-start alignment with hardware SP alignment.

The `15-exercise-frame` starter gives CHECK_INITIAL_FRAME=FAIL and NO_LAUNCH/STACK_GUARDS/
NO_FAULT=PASS. After solving it, all nine kernel checks should PASS.
Do not hide the error by changing the scheduler or the values expected by the test.
