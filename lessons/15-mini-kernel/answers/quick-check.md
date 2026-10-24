# Quick Check cevaplarÄ±

1. Basic hardware frame caller-clobbered register'larÄ± saklar. Suspend edilen
   task'Ä±n callee-saved register'larÄ±nÄ± scheduler korumalÄ±dÄ±r.
2. STM, SP Ã¼zerinden yapÄ±lmaz. Adres sÄ±nÄ±rÄ± ilk store'dan Ã¶nce denetlenmelidir.
3. O frame PSP Ã¼zerinde kurulmadÄ±; stack seÃ§imini deÄŸiÅŸtirip eski C fonksiyonunu
   sÃ¼rdÃ¼rmek farklÄ± bellekte yerel deÄŸiÅŸken/return address arar.
4. Yield'ler bitince her iki task'Ä±n ilerlemesi ve kaydedilen PC yield dÃ¶nÃ¼ÅŸÃ¼
   dÄ±ÅŸÄ±nda olan timer switch'leri. YalnÄ±z tick saymak task geÃ§iÅŸini kanÄ±tlamaz.
