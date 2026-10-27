# Release notes taslağı — v1.0.0

Öneri: **v1.0.0 — ARM Cortex-M Practical Lab**. Bu eğitim kapsamının ilk tam
sürüm önerisidir; production RTOS veya donanım sertifikası anlamına gelmez.
Tag ve GitHub Release oluşturulmadı. Kesin hedef SHA, denetim düzeltmeleri
[PR #12](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/12) merge commit'i
ve ardından main üç-preset CI sonucu doğrulanarak seçilir. v1.0.0 tag/Release
için ayrıca kullanıcı onayı gerekir.

- Cortex-M33 / QEMU mps2-an505 üzerinde 16 uygulamalı ders; ayrı eksik
  alıştırmalar ve referans çözümler, kontrollü Break It / Debug It deneyleri.
- Beş birleştirici proje: Protected Application, Fault Analyzer,
  From Reset to Main, Performance Profiler, Educational Mini Kernel.
- Secure/Non-secure sensor node: üç unprivileged görev, MPU stack izolasyonu,
  SVC servisleri, PendSV scheduling ve sınırlı Secure DSP/FP kalibrasyonu.
- Debug/Release/MinSize build, davranışsal smoke/probes, negatif senaryolar
  ve GDB kapıları. Son sonuç ve gerçek bulgular [iç denetim kaydında](ENGINEERING_AUDIT.md).
- Frame hizası, padding/mode doğrulaması ve stack-error negatif testleri güçlendirildi.
- SG girişinde henüz Non-secure durumda kesilen task'ın yanlış scheduler
  panic üretmesi giderildi; yalnız bilinen gateway girişleri kabul edilir.

Sınırlar: sentetik sensör; donanım testi ve üçüncü taraf audit yoktur.
QEMU cycle-accurate değildir; DWT CYCCNT'den performans veya WCET iddiası yoktur.
Genel TrustZone/FP-aware RTOS, kriptografik güvence ve fiziksel saldırı koruması
bu sürümün kapsamı değildir. İlk kurulum için [README](../README.md).
