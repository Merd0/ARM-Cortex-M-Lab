# AlÄ±ÅŸtÄ±rma â€” Ä°lk task frame'i

YalnÄ±z `frame.c` TODO'sunu tamamla. Top 8-byte hizalÄ±dÄ±r ve 17 word alan vardÄ±r.
YazÄ±lÄ±m kÄ±smÄ± R4â€“R11 + EXC_RETURN; hardware kÄ±smÄ± R0â€“R3, R12, LR, PC, xPSR.
R0 task kimliÄŸi olmalÄ±; entry Thumb state ile baÅŸlatÄ±lmalÄ±, task dÃ¶nÃ¼ÅŸÃ¼ exit'e gitmeli.
Frame baÅŸlangÄ±cÄ± ile hardware SP hizasÄ±nÄ± karÄ±ÅŸtÄ±rma.

`15-exercise-frame` starter'Ä± CHECK_INITIAL_FRAME=FAIL, NO_LAUNCH/STACK_GUARDS/
NO_FAULT=PASS verir. Ã‡Ã¶zÃ¼mden sonra bÃ¼tÃ¼n sekiz kernel kontrolÃ¼ PASS olmalÄ±.
Scheduler veya testin beklediÄŸi deÄŸerleri deÄŸiÅŸtirerek hatayÄ± gizleme.
