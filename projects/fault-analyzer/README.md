# Project — Fault Analyzer

**STATUS: Debug temel kapısı PASS; son toplu kalite kapısı bekliyor.**

[diagnostic.c](diagnostic.c) Lesson 08'in referans decoder'ını yeniden kullanır.
Önüne konservatif frame capture sınırı ekler: trusted stack bounds, desteklenen
EXC_RETURN, minimum uzunluk, hizalama, xPSR.T/exception mode ve padding.
MSTKERR, STKERR, STKOF, unstack/lazy-save hatalarında **pointer dereference edilmez**.
Basic same-state Secure ve Non-secure frame desteklenir. FP ve cross-security
frame'ler sessizce yanlış decode edilmek yerine reddedilir.

## Kritik düzeltme

`0xFFFFFFFD` bu temel Secure bağlamda **Thread Mode / PSP / basic frame**
dönüşüdür. Handler/MSP dönüşü değildir. Non-secure same-state karşılığı
`0xFFFFFFBC` olur; security bitlerini yok sayarak aynı sabit kullanılamaz.

Örneğin CFSR=0x9200 hem PRECISERR hem STKERR ve BFARVALID içerir.
HFSR.FORCED=1 yükseltilmiş fault'u gösterir. BFAR ancak validity bitiyle adres
kanıtıdır; stacking başarısız olduğundan stacked PC **güvenilir değildir**.
Bitlerin birlikte bulunması tek başına olayların nedensel sırasını kanıtlamaz.

## Çalıştırma

```bash
cmake --preset debug
cmake --build --preset debug
./scripts/run-qemu.sh build/debug/projects/fault-analyzer/project-fault-bus.elf
python tests/system-smoke.py
```

Gerçek firmware'ler: project-fault-udf, -unaligned, -bus, -mpu, -escalated.
Ayrı project-fault-cases, sentetik negatif snapshot testlerini çalıştırır.
Bunlar gerçek stacking fault üretilmiş gibi sunulmaz.

Debug/QEMU'da beş gerçek fault senaryosunda CAUSE/FRAME/PRIVILEGE/CLEAR PASS;
dokuz sentetik kontrolde MSP/PSP/NS basic frame, stack-error reddi, FP/cross-state
reddi, bounds ve MMFAR/BFAR validity PASS. HFSR temizliği Cortex-M33 DEBUGEVT=1
yazma kuralını korur; generic CMSIS IMPRECISERR sabiti M33 BFSR bit2 uygulanmış
gibi yorumlanmaz. Raw CFSR/HFSR kaybolmaz; birincil sınıf bütün root-cause zinciri değildir.

Handler privilege CONTROL.nPRIV'den çıkarılmaz. Handler her zaman privileged'dır.
Capture, corrupt MSP üzerinde güvenli bir emergency-stack mekanizması değildir;
böyle bir durumda exception entry/raporlama zaten çalışamayabilir.

GDB ve geniş negatif set son kapıda yapılacak. Öğrenci starter'ı ve referans
decoder [Lesson 08](../../lessons/08-faults/) içinde ayrı kalır.

[Projects](../README.md)
