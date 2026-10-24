# AlÄ±ÅŸtÄ±rma â€” Scalar Secure API politikasÄ±

YalnÄ±z `policy.c` TODO'sunu tamamla: [0,15] kapalÄ± aralÄ±ÄŸÄ±nÄ± kabul et.
DiÄŸer bÃ¼tÃ¼n uint32_t girdiler reddedilmeli. SAU, MPC, SG, startup veya test
beklentisini deÄŸiÅŸtirme. Starter fail-closed'dur: yalnÄ±z CHECK_SERVICE=FAIL;
geÃ§ersiz girdiler ve Secure bellek ihlali kontrolleri PASS kalÄ±r.

`16-exercise-policy-nonsecure` hedefi kendi Secure baÄŸÄ±mlÄ±lÄ±ÄŸÄ±nÄ± da derler.
Ä°ki exercise ELF'ini beraber yÃ¼kle veya `python tests/development-smoke.py 16`
Ã§alÄ±ÅŸtÄ±r. Ã‡Ã¶zÃ¼ldÃ¼ÄŸÃ¼nde tÃ¼m CHECK alanlarÄ± PASS olmalÄ±. 0,7,15 kabul edilir;
16 ve 0xFFFFFFFF reddedilir. Referans Ã§Ã¶zÃ¼m ayrÄ± dizindedir.
