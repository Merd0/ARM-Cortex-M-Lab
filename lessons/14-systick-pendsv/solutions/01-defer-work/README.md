**Language:** English | [Türkçe](README.tr.md)

# Reference solution — Priority

`priorities.c` sets SysTick to 64 and PendSV to 255. It is reapplied after
SysTick_Config. `14-solution-priority` passed all checks in Debug/QEMU:
the initial event order was 1,2,3 and the timer counter was 4.
