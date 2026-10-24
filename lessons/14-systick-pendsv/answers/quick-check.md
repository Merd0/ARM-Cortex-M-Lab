# Quick Check cevapları

1. Pending biti tek durumdur; birden çok istek birleşebilir.
2. Eşit öncelikler preemption sağlamaz; SysTick bitince PendSV yine çalışabilir.
3. CMSIS yardımcısı SysTick önceliğini en düşük değere yazar.
4. Hayır. Thread MSP üzerinde kaldı; context switching sonraki derstedir.
