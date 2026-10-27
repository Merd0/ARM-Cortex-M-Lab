# İç mühendislik denetimi — 2026-10-27

Başlangıç main: `11fde900835d4443a55d0e7da64438ea4da4c690`.
Yerel dal: `codex/final-engineering-audit`. Bu çalışma önceki geliştirme
kararlarını otomatik kabul etmeden yapılan bir iç mühendislik denetimidir;
bağımsız üçüncü taraf audit veya güvenlik sertifikası değildir.
Kod/test düzeltmesi: `142b3b9059c646cf27d57c12929f4f0fb028fbaa`.
Sonraki dokümantasyon commit'i test edilen firmware kaynaklarını değiştirmez.

## Gerçek bulgular ve düzeltmeler

### H1 — High: SG girişinde kesilen geçerli NS task scheduler panic üretiyordu

[kernel.c](../final-project/secure-real-time-sensor-node/nonsecure/kernel.c),
`valid()` dönüş PC kontrolü (satır 51–64). İlk temiz matrisin Debug bad-svc
çalıştırması 24 örnekten sonra panic üretti. 120 paralel hedefli tekrarın birinde
aynı hata doğal olarak yakalandı: saved EXC_RETURN `0xFFFFFFBC`, stacked PC
`0x10070008`, CFSR sıfır. Bu adres `l16_service` SG girişidir. SG yürütülmeden
kesilirse context hâlâ NS Thread'e aittir; NS flash aralığına ek olarak bu
geçerli dönüş noktası da mümkün olmalıdır. Önceki kontrol bunu reddediyordu.

Yalnız import edilen `l16_report` ve `l16_service` SG adresleri eklendi;
arbitrary Secure/NSC adresi açılmadı. PC bit0 da kontrol ediliyor. GDB'de gerçek
SysTick basic frame'inin PC'sini SG girişine ayarlayan kontrollü fixture eski
kodda panic ile FAIL, düzeltmeden sonra restore ile PASS verdi. Her iki SG
girişi ve SG+4 adresinin reddi kalıcı testtir. Bu fixture gerçek SG öncesi
kesilme ürettiğini iddia etmez; doğal panic kaydı ayrı kanıttır.

### M1 — Medium: M33 frame hizası yeterince doğrulanmıyordu

[diagnostic.c](../projects/fault-analyzer/diagnostic.c), `fa_capture` adres
kontrolü (satır 21–24); [cortex-m.gdb](../scripts/gdb/cortex-m.gdb), `cm-frame`
adres kontrolü (satır 297–301), padding/mode kontrolleri (satır 350–362).
İki yol da yalnız word hizasını arıyordu. Cortex-M33 CCR[9] RES1 olduğundan
exception frame tabanı 8-byte hizalıdır. 4 mod 8 adresindeki sentetik tampon,
düzeltme öncesi C ve GDB tarafından kabul edildi. Bu, yanlış crash verisini
makul frame olarak sunabilir; canlı sistemde bellek bozulması kanıtı değildir.

Kontrol 8-byte yapıldı; fixture açıkça hizalandı. GDB yardımcısında ayrıca
padding'in RAM penceresine sığması ve EXC_RETURN Mode ile stacked IPSR uyumu
doğrulanıyor. Yeni GDB oturumu geçerli MSP/PSP/NS/Handler frame'lerini kabul
ederken yanlış hizayı, eksik padding'i ve iki yöndeki mode uyuşmazlığını reddeder.
Bu kontroller bir RAM tamponunun gerçek exception geçmişini kanıtlamaz.

### M2 — Medium: stack-error negatif testi yanlış nedenle geçebiliyordu

[cases.c](../projects/fault-analyzer/src/cases.c), `faults` döngüsü (satır 20–35); eski test
yalnız sınır dışı pointer kullanıyordu. Bounds kontrolü bu pointer'ı zaten
reddettiğinden status veto'su kaldırılsa bile capture testi geçebilirdi.
Eksik üç bit de eklenerek yedi stack/unstack/lazy hata biti geçerli bir tamponla
sınanıyor; `fa_analyze` ayrıca caller'ın `frame_valid=1` iddiasına rağmen
reddetmek zorunda. STKERR veto'sunu geçici kaldıran mutasyon yeni testi FAIL
yaptı; mutasyon geri alındı. Gerçek hardware stacking fault üretildiği iddia edilmez.

### M3 — Medium: yayımlanmış çalışmanın durumu eskiydi

Örneğin final-project/README.md satır 3 ve projects/README.md satır 22–26:
README, BOOTCAMP, PROGRESS, ROADMAP, Lesson 13–16 ve proje/final girişleri
tamamlanan PR #9–11 için hâlâ CI/merge beklediğini söylüyordu. Güncel durum
[main CI 40105930630](https://github.com/Merd0/ARM-Cortex-M-Lab/actions/runs/40105930630)
ile ilişkilendirildi. Tarihsel yerel kayıtlar silinmedi. ENVIRONMENT'in yalnız
Linux doğrulaması iddiası ve boş Windows sürüm kaydı da gerçek ortamla düzeltildi.
Bu CI sonucu yeni yerel denetim değişikliklerine mal edilmez.

İncelenen kapsamda Critical bulgu doğrulanmadı. High bulgu H1 düzeltildi;
bu ifade tüm olası hataların yokluğu veya saldırıya dayanıklılık garantisi değildir.

## Mimari inceleme

- Lesson 15/final PendSV: ilk indirect store öncesi bounds; R4–R11 ve
  EXC_RETURN save/restore; C çağrısında 8-byte MSP; önceki PRIMASK'ın dönüşü;
  bootstrap'ta eski MSP C frame'ine dönülmemesi. Normal scheduler basic frame
  ile sınırlı; Secure Thread kesildiğinde aynı owner/token ile defer yolu kullanılır.
- Final MPU: 32-byte hizalı task stack mapping'i, privileged kernel RAM,
  unprivileged NS görevler, Handler-only Secure işleme. Kuyruklar SVC içinde
  seri erişilir; SysTick yalnız tick/pending sayacına dokunur. Genel RTOS/FP
  context switching veya keyfi Secure callback desteği iddia edilmez.
- Lesson 13 ve Secure FP leaf: integer public ABI, callee-saved register'lar,
  FPSCR kontrolleri; leaf S0/S1 temizliği ve FPCA/SFPA geri yükleme. NS FPU
  izni kapalı. Eager/lazy deneyler ayrı Lesson 13 kapısında gözlenir.
- Lesson 16: SAU/IDAU/MPC sınırları, NSC veneer, CMSE entry/call türleri,
  geçerli NS reset vektörü, Secure fault adres validity. Non-secure giriş
  payload'ı güvenilir pointer olarak alınmaz; gateway scalar API kullanır.
- Linker/startup: RAM/FLASH bölgeleri, `.data > RAM AT > FLASH`, NSC sınırları,
  `.bss` ve stack ayrımı; libgcc sırası ve etkin Release `-O2` korunur.
  Linker sembolleri/fonksiyon adresleri GNU Arm bare-metal ABI sözleşmesidir;
  bu kod host üzerinde taşınabilir ISO C programı olarak sunulmaz.
- Test sözleşmeleri: starter ve kasıtlı bozuk firmware'lerin beklenen FAIL
  kümeleri; tam CHECK anahtar kümesi/END, bağımsız integer pipeline oracle,
  gerçek MMFAR/CFSR/GDB bağlam gözlemleri. Mevcut assertion'lar gevşetilmedi.

## Birincil kaynaklar

- [Arm Cortex-M33 Devices Generic User Guide](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d),
  CCR tabloları 4-13/4-14: bit 9 RES1. M1'in M33'e özel dayanağıdır.
- [Arm AAPCS32](https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst):
  public interface SP hizası, callee-saved core/FP register ve FPSCR kuralları.
- [Armv8-M Architecture Reference Manual, SG](https://community.arm.com/cfs-file/__key/communityserver-discussions-components-files/471/DDI0553B_5F00_y_5F00_armv8m_5F00_arm.pdf),
  C2.4.180: NS çağrı hedefi SG'dir; Secure durumuna geçiş SG yürütülürken yapılır.
  H1'de PC adresi tek başına yürütme güvenlik durumunu belirlemiyordu.
- [CMSIS 6.3 Armv8-M MPU API](https://arm-software.github.io/CMSIS_6/v6.3.0/Core/group__mpu8__functions.html):
  RBAR/RLAR 32-byte sınırları ve RO/NP/XN parametreleri vendored header ile karşılaştırıldı.
- [GCC ARM options](https://gcc.sourceware.org/onlinedocs/gcc/ARM-Options.html):
  `-mgeneral-regs-only` assembler'ı kısıtlamaz; assembly FP leaf ayrıca incelendi.
- [QEMU NS alias düzeltmesi](https://github.com/qemu/qemu/commit/4a934d284dfac9fa19b0f47874f12d9b3519c21c):
  main'deki eski QEMU 8.2.2 workaround'u mimari adres yasağı olarak yorumlanmadı.

## Son kalite kapısı

**Yerel son kapı PASS.** Windows / xPack GCC 15.2.1 / QEMU 11.1.0 /
Arm GDB 16.3.90 / CMake 4.3.2 / Ninja 1.13.2; Debug, Release ve MinSize:

- Temiz build: 109 ELF/preset, 0 warning, 0 error.
- Smoke: 566 PASS / 0 FAIL, build probes: 47 PASS / 0 FAIL, her preset.
- Lesson 12/13 runtime/GDB/ABI kapıları; Lesson 14–16'nın 12 senaryosu PASS.
- Beş projenin 15 ve final sistemin 7 senaryosu PASS; normal pipeline
  32 örnek / 298 toplam, telemetry bağımsız integer oracle ile eşleşti.
- 13 GDB oturumu/preset: önceki dokuz context/fault/FP/security oturumu,
  frame sınırları ve üç SG giriş/ret senaryosu. Debug'da SG single-step
  assertion'ı son eklenince yalnız ilgili iki oturum ayrıca yeniden çalıştırıldı.
- H1 sonrası Debug bad-svc stress: 120/120 PASS, altı paralel QEMU süreci.
- 402 relative Markdown link; `git diff --check` PASS. CI matrisi üç preset'te
  bütün bu test script'lerini çağırıyor; workflow gevşetilmedi/değiştirilmedi.
- PowerShell environment check bütün araçları buldu. Platform/CMSIS ve
  Lesson 01–12 çalışma kodu değişmedi.

İlk kapının Debug system testi H1 nedeniyle FAIL oldu; bu kayıt silinmedi.
H1 sonrası üç preset temiz derlendi, bütün etkilenen system/GDB ve Lesson 12–16
kapıları yenilendi. Release/MinSize eski smoke/probe kanıtı ancak yeniden
derlenen 95 non-final ELF'in SHA-256'ları birebir eşleştikten sonra korundu;
aynı testler gereksiz yere tekrarlanmadı. Debug smoke/probes son düzeltmeyle
çalıştırıldı. Başarısız ilk koşu, son başarı gibi sunulmaz.

Ham kayıtlar ignored `build/audit-final-2026-10-27/` (ilk koşu),
`build/audit-final-2026-10-27/after-sg-fix/` (sonuç JSON/logları ve ELF hash'leri),
`build/debug/audit-evidence/` (mutasyon, doğal panic ve stress) ve
`build/<preset>/{system,development,lesson12,lesson13}-evidence/` altında.
Bu dosyalar Git'e eklenmedi; kalıcı tekrar üretme araçları tests/ altındadır.

## Kapsam sınırları ve yayın kararı

Gerçek donanım testi yoktur; QEMU cycle-accurate değildir. CYCCNT ilerlemediği
için cycle/throughput/WCET sonucu yoktur. Sensör sentetik, Secure servis
kalibrasyondur; kriptografik kimlik, DMA/debugger saldırıları, code injection/ROP,
genel Secure/FP task switching ve üretim RTOS güvenilirliği doğrulanmadı.
VS Code etkileşimli akışı ve fiziksel kart kurulumu bu kapının parçası değildir.

Yerel teslim anında mühendislik kapısı geçti; düzeltmeler henüz yayımlanmamıştı.
Bu tarihsel sonuç, uzak CI veya merge gerçekleştiği iddiası değildir.

Sonraki yayın süreci [PR #12](https://github.com/Merd0/ARM-Cortex-M-Lab/pull/12)
ile izlenir: üç preset CI geçmeden merge edilmez; merge sonrası main CI da
doğrulanır. Kesin merge SHA ve uzak sonuçlar PR/Actions kayıtlarının parçasıdır.
Eski `11fde900` H1 düzeltmesini içermez ve v1.0.0 hedefi değildir.
Tag/Release ve görünürlük değişikliği bu teslimin dışında, ayrıca onaya bağlıdır. [Release notes taslağı](RELEASE_NOTES_DRAFT.md).
