# Referans çözüm — İlk task frame'i

`frame.c` top'tan 17 word ayırır; indeks 8 EXC_RETURN=0xFFFFFFFD, indeks 9 R0,
14 Thumb LR, 15 entry PC, 16 xPSR.T olur. R4–R11 başlangıçta sıfırdır; task
ASM girişinde kendine özgü canary'leri kurar. Hardware frame top'tan 32 byte,
yazılım context 68 byte aşağıdadır. `15-solution-frame` Debug/QEMU'da dokuz
kontrolün tamamını geçti.
