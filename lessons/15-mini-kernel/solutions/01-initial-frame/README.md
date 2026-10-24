# Referans Ã§Ã¶zÃ¼m â€” Ä°lk task frame'i

`frame.c` top'tan 17 word ayÄ±rÄ±r; indeks 8 EXC_RETURN=0xFFFFFFFD, indeks 9 R0,
14 Thumb LR, 15 entry PC, 16 xPSR.T olur. R4â€“R11 baÅŸlangÄ±Ã§ta sÄ±fÄ±rdÄ±r; task
ASM giriÅŸinde kendine Ã¶zgÃ¼ canary'leri kurar. Hardware frame top'tan 32 byte,
yazÄ±lÄ±m context 68 byte aÅŸaÄŸÄ±dadÄ±r. `15-solution-frame` Debug/QEMU'da sekiz
kontrolÃ¼n tamamÄ±nÄ± geÃ§ti.
