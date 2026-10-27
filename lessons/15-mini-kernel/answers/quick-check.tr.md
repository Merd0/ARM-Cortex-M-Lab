**Dil:** [English](quick-check.md) | Türkçe

# Quick Check cevapları

1. Basic hardware frame caller-clobbered register'ları saklar. Suspend edilen
   task'ın callee-saved register'larını scheduler korumalıdır.
2. STM, SP üzerinden yapılmaz. Adres sınırı ilk store'dan önce denetlenmelidir.
3. O frame PSP üzerinde kurulmadı; stack seçimini değiştirip eski C fonksiyonunu
   sürdürmek farklı bellekte yerel değişken/return address arar.
4. Yield'ler bitince her iki task'ın ilerlemesi ve kaydedilen PC yield dönüşü
   dışında olan timer switch'leri. Yalnız tick saymak task geçişini kanıtlamaz.
