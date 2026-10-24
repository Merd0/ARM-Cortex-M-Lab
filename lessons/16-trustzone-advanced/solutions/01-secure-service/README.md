# Referans çözüm — Scalar politika

`value <= 15U` tüm unsigned [0,15] aralığını kabul eder. İşaretli cast veya
yalnız bir örneğe eşitlik kullanmak sınır koşullarını bozar. `16-solution-policy`
Secure/Non-secure çifti Debug/QEMU'da tüm kontrolleri geçti. Politika bellek
attribution'ını değiştirmez; NS'den doğrudan Secure okuma hâlâ AUVIOL üretir.
