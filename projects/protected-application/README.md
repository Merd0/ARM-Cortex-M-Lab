# Project — Protected Application

**STATUS: yerel Debug/Release/MinSize PASS; uzak CI/merge onayı bekliyor.**

[main.c](src/main.c), protected counter için sınırlı bir SVC API kurar.
Lesson 06'nın gerçek PSP geçişi, SVC entry ve tek-store probe assembly kaynakları
yeniden derlenir. CONTROL.SPSEL, eski MSP C frame'i içinde değiştirilmez:
assembly yeni PSP üzerinde fresh Thread C girişine dallanır.

MPU yalnız FLASH'ı unprivileged RO/execute, app stack/sonuç bloğunu RW/XN açar.
Kernel counter açık bölgelere dahil değildir. Handler MSP kullanır; PRIVDEFENA
privileged varsayılan haritaya izin verir. nPRIV=1 task kendini yükseltmez;
uygulamada privilege-restore servisi bulunmaz.

```bash
cmake --preset debug
cmake --build --preset debug --target project-protected-normal project-protected-denied
./scripts/run-qemu.sh build/debug/projects/protected-application/project-protected-normal.elf
python tests/system-smoke.py
```

SVC #1 yalnız [0,31] scalar değerini counter'a ekler. 1,2,3 çağrıları 1,3,6
döndürür; 32 reddedilir ve counter değişmez. SVC frame'i decode edilmeden önce
Fault Analyzer capture sınırıyla doğrulanır. SVC #2 sonucu privileged Handler'dan
raporlar. UART'ın unprivileged task'a açılması gerekmez.

Debug/QEMU normal: UNPRIV_PSP/SVC/REJECT/HANDLER/NO_FAULT PASS.
Denied sürümde doğrudan store, IPSR=4, CFSR=0x82 ve doğru MMFAR ile MemManage
üretir; frame geçerli, kernel counter=6 ve HFSR=0 kalır. Bu beklenen fault PASS'tir.

ISB, CONTROL güncellemesinden sonraki instruction stream'i senkronize eder;
tek başına bellek izin politikası kurmaz. MSP/PSP seçimi ile privilege ayrıdır.
Öğrenci MPU/SVC alıştırması [Lesson 06](../../lessons/06-mpu/) içinde ayrıdır.

[Fault Analyzer](../fault-analyzer/) · [Projects](../README.md)
