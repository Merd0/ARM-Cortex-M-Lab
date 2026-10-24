# Referans çözüm — Öncelik

`priorities.c` SysTick'i 64, PendSV'yi 255 yapar. SysTick_Config sonrası tekrar
uygulanır. `14-solution-priority` Debug/QEMU'da bütün kontrolleri geçti:
ilk olay sırası 1,2,3 ve timer sayacı 4 oldu.
