# Referans Ã§Ã¶zÃ¼m â€” Scalar politika

`value <= 15U` tÃ¼m unsigned [0,15] aralÄ±ÄŸÄ±nÄ± kabul eder. Ä°ÅŸaretli cast veya
yalnÄ±z bir Ã¶rneÄŸe eÅŸitlik kullanmak sÄ±nÄ±r koÅŸullarÄ±nÄ± bozar. `16-solution-policy`
Secure/Non-secure Ã§ifti Debug/QEMU'da tÃ¼m kontrolleri geÃ§ti. Politika bellek
attribution'Ä±nÄ± deÄŸiÅŸtirmez; NS'den doÄŸrudan Secure okuma hÃ¢lÃ¢ AUVIOL Ã¼retir.
