# Alıştırma — İlk task frame'i

Yalnız `frame.c` TODO'sunu tamamla. Top 8-byte hizalıdır ve 17 word alan vardır.
Yazılım kısmı R4–R11 + EXC_RETURN; hardware kısmı R0–R3, R12, LR, PC, xPSR.
R0 task kimliği olmalı; entry Thumb state ile başlatılmalı, task dönüşü exit'e gitmeli.
Frame başlangıcı ile hardware SP hizasını karıştırma.

`15-exercise-frame` starter'ı CHECK_INITIAL_FRAME=FAIL, NO_LAUNCH/STACK_GUARDS/
NO_FAULT=PASS verir. Çözümden sonra bütün dokuz kernel kontrolü PASS olmalı.
Scheduler veya testin beklediği değerleri değiştirerek hatayı gizleme.
