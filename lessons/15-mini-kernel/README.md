# Lesson 15 â€” Educational Mini Kernel

`[DEEP]` Â· **STATUS: temel geliÅŸtirme doÄŸrulandÄ±; toplu kalite kapÄ±sÄ± bekliyor**

## TEACH â€” Bir task'Ä±n devam edebilmesi iÃ§in ne saklanÄ±r?

Ä°ki privileged Thread task'Ä± ayrÄ± PSP stack'leri kullanÄ±r. Handler'lar MSP
Ã¼zerindedir. DonanÄ±m basic frame'i (R0â€“R3, R12, LR, PC, xPSR), PendSV ise
R4â€“R11 ve EXC_RETURN'Ã¼ saklar. Sabit round-robin seÃ§iminden sonra yeni PSP ile
exception return yapÄ±lÄ±r. Bu eÄŸitim Ã§ekirdeÄŸi FPU task context'i, MPU task
izolasyonu, unprivileged task veya Ã¼retim RTOS garantisi saÄŸlamaz.

[Context assembly](example/context.S), [Ã§ekirdek](example/main.c) ve
[ilk frame](example/frame.c) birlikte okunmalÄ±dÄ±r. YazÄ±lÄ±m frame'i 9 word,
hardware frame'i 8 word'dÃ¼r. YazÄ±lÄ±m SP'si 4 mod 8 olabilir; public exception
boundary'deki hardware SP 8-byte hizalÄ±dÄ±r. Bu farkÄ± kaldÄ±rmak iÃ§in rastgele
bir padding word eklemek register eÅŸlemesini bozar.

## SHOW â€” BaÅŸlangÄ±Ã§ ve geÃ§iÅŸ sÄ±nÄ±rlarÄ±

Ä°lk SVC, C'ye Ã§aÄŸrÄ± yapmadan CONTROL.SPSEL/PSP/PSPLIM'i kurar, ilk register
setini yÃ¼kler ve MSP'yi stack top'a getirir. Eski main/MSP frame'i kalÄ±cÄ± olarak
terk edilir. Ä°lk task hardware frame'inden baÅŸlar; eski C frame'i PSP ile
devam ettirilmez. SVC'den sonraki bootstrap koduna dÃ¶nÃ¼lmez.

PendSV yalnÄ±z 0xFFFFFFFD basic Secure Thread/PSP dÃ¶nÃ¼ÅŸÃ¼nÃ¼ kabul eder. FPU veya
baÅŸka frame Ã§eÅŸidi sessizce destekleniyor sayÄ±lmaz. STM'den **Ã¶nce** Ã§Ä±kÄ±ÅŸ
stack sÄ±nÄ±rlarÄ± kontrol edilir; PSPLIM, R0 Ã¼zerinden yapÄ±lan STM iÃ§in koruma
deÄŸildir. Scheduler'a MSP Ã¼zerinde 8-byte hizalÄ± Ã§aÄŸrÄ± yapÄ±lÄ±r. KÄ±sa seÃ§im
bÃ¶lÃ¼mÃ¼ eski PRIMASK deÄŸerini korur; yeni context doÄŸrulanmadan yÃ¼klenmez.

## RUN

```bash
cmake --preset debug
cmake --build --preset debug --target 15-mini-kernel 15-kernel-broken 15-exercise-frame 15-solution-frame
./scripts/run-qemu.sh build/debug/lessons/15-mini-kernel/15-mini-kernel.elf
python tests/development-smoke.py 15
```

## OBSERVE

Debug/QEMU Ã§alÄ±ÅŸtÄ±rmasÄ±nda Ã¶rnek ve Ã§Ã¶zÃ¼mÃ¼n sekiz CHECK alanÄ± PASS oldu.
Kaydedilen bir Ã¶rnek: TASK0_RUNS=1193, TASK1_RUNS=976, TICKS=4,
SWITCHES=20, ASYNC_SWITCHES=4. Task sayaÃ§larÄ±/model zamanlamasÄ± deÄŸiÅŸebilir;
test bu rastlantÄ±sal sayÄ±larÄ± sabitlemez.

Task'lar ilk sekiz tur SVC #0 ile yield eder. Sonra yield etmeden busy-loop
yaparlar: ikinci task'Ä±n ilerleyebilmesi iÃ§in gerÃ§ek SysTick/PendSV preemption
gerekir. ASYNC_SWITCHES, kaydedilen PC yield dÃ¶nÃ¼ÅŸ noktasÄ± olmadÄ±ÄŸÄ±nda sayÄ±lÄ±r.
R4â€“R11'in her birine task'a Ã¶zgÃ¼ canary konur. Her task'Ä±n alt/Ã¼st stack
guard'larÄ±, ilerleme, yield ve timer switch'leri bitiÅŸte denetlenir.

GDB'de isteÄŸe baÄŸlÄ± gÃ¶zlem (bu aÅŸamada kapsamlÄ± GDB kapÄ±sÄ± Ã§alÄ±ÅŸtÄ±rÄ±lmadÄ±):

```text
break PendSV_Handler
print/x $psp
print g_l15_current
x/17wx g_l15_current->sp
print g_l15_runs
print g_l15_bad
```

Handler giriÅŸinde TCB'nin SP'si henÃ¼z gÃ¼ncellenmemiÅŸ olabilir. TCB dump'Ä±nÄ±
ASM'nin store'undan sonra veya scheduler iÃ§inde al. Breakpoint zamanlamayÄ± etkiler.

## MODIFY â†’ BREAK â†’ DEBUG

`15-kernel-broken` yazÄ±lÄ±m context alanÄ±nÄ± ayÄ±rÄ±r ama R4â€“R11'i yazmaz.
CHECK_REGISTERS=FAIL olur; task'larÄ±n C gÃ¶zlem Ã§aÄŸrÄ±sÄ± boyunca IRQ maskelenir.
BÃ¶ylece kasÄ±tlÄ± bozuk restore, askÄ±da kalan C pointer'larÄ±nÄ± bozup rastgele
Ã§Ã¶kmek yerine ASM canary kontrolÃ¼nde gÃ¶rÃ¼nÃ¼r. Ãœretim scheduler'Ä± iÃ§in bir
C Ã§aÄŸrÄ±sÄ±nÄ± maskelemek register saklama zorunluluÄŸunu ortadan kaldÄ±rmaz.

[AlÄ±ÅŸtÄ±rmada](exercises/01-initial-frame/README.md) ilk frame oluÅŸturucu eksiktir.
NULL, PSP kurulmadan reddedilir. [Ã‡Ã¶zÃ¼m](solutions/01-initial-frame/README.md)
ayrÄ± dosyadadÄ±r.

## VERIFY

Debug build ve dÃ¶rt QEMU senaryosu Ã§alÄ±ÅŸtÄ±rÄ±ldÄ±: Ã¶rnek/Ã§Ã¶zÃ¼m PASS, eksik starter
yalnÄ±z CHECK_INITIAL_FRAME=FAIL, bozuk save yalnÄ±z CHECK_REGISTERS=FAIL.
Bu testler WCET, adil zaman payÄ±, sonsuz Ã§alÄ±ÅŸma veya donanÄ±m doÄŸrulamasÄ± deÄŸildir.
ÃœÃ§-preset karÅŸÄ±laÅŸtÄ±rmasÄ±, geniÅŸ negatif set ve baÄŸÄ±msÄ±z audit toplu kapÄ±da yapÄ±lacak.

## Quick Check

1. Exception entry neden R4â€“R11'i kurtarmÄ±yor?
2. PSPLIM neden PendSV'nin STM adresini ayrÄ±ca doÄŸrulama gereÄŸini kaldÄ±rmÄ±yor?
3. Ä°lk SVC'nin terk ettiÄŸi MSP frame'ine dÃ¶nmek neden hatadÄ±r?
4. Yield dÄ±ÅŸÄ±nda switch olduÄŸunu hangi gÃ¶zlem ayÄ±rÄ±r?

[Cevaplar](answers/quick-check.md)

## References

- [Cortex-M33 Devices Generic User Guide](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d): basic frames, PSP/MSP, exception return.
- [AAPCS32](https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst): callee-saved registers and public stack alignment.
- [Lesson 04](../04-stack-msp-psp/): fresh C frame sÄ±nÄ±rÄ±.
- [Lesson 13](../13-sync-fpu-dsp/): bu Ã§ekirdeÄŸin desteklemediÄŸi FP frame'leri.

**Previous:** [14](../14-systick-pendsv/) Â· **Next:** [16](../16-trustzone-advanced/)
