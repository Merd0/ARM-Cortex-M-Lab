**Language:** English | [Türkçe](quick-check.tr.md)

# Quick Check answers

1. A pending bit is a single state; multiple requests can coalesce.
2. Equal priorities do not allow preemption; PendSV can still run after SysTick finishes.
3. The CMSIS helper writes the lowest priority to SysTick.
4. No. Thread remained on MSP; context switching is covered in the next lesson.
