**Language:** English | [Türkçe](README.tr.md)

# Reference solution — Initial task frame

`frame.c` allocates 17 words below the top; index 8 is EXC_RETURN=0xFFFFFFFD, index 9 R0,
14 Thumb LR, 15 entry PC, and 16 xPSR.T. R4–R11 are initially zero; each task sets up
its own canaries at its assembly entry. The hardware frame is 32 bytes below the top,
and the software context is 68 bytes below it. `15-solution-frame` passed all nine
checks in Debug/QEMU.
