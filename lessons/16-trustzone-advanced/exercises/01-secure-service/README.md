# Alıştırma — Scalar Secure API politikası

Yalnız `policy.c` TODO'sunu tamamla: [0,15] kapalı aralığını kabul et.
Diğer bütün uint32_t girdiler reddedilmeli. SAU, MPC, SG, startup veya test
beklentisini değiştirme. Starter fail-closed'dur: yalnız CHECK_SERVICE=FAIL;
geçersiz girdiler ve Secure bellek ihlali kontrolleri PASS kalır.

`16-exercise-policy-nonsecure` hedefi kendi Secure bağımlılığını da derler.
İki exercise ELF'ini beraber yükle veya `python tests/development-smoke.py 16`
çalıştır. Çözüldüğünde tüm CHECK alanları PASS olmalı. 0,7,15 kabul edilir;
16 ve 0xFFFFFFFF reddedilir. Referans çözüm ayrı dizindedir.
