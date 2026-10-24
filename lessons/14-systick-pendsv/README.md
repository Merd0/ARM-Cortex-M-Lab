# Lesson 14 â€” SysTick & PendSV

`[STANDARD]` Â· **STATUS: temel geliÅŸtirme doÄŸrulandÄ±; toplu kalite kapÄ±sÄ± bekliyor**

## TEACH â€” Tick bir scheduler deÄŸildir

SysTick zamanÄ± sayar; PendSV ertelenmiÅŸ iÅŸi Ã§alÄ±ÅŸtÄ±rÄ±r. Pending biti olay kuyruÄŸu
deÄŸildir: maskeli durumda aynÄ± biti iki kere set etmek iki Ã§aÄŸrÄ± biriktirmez.
Scheduler bu mekanizmalarÄ±n Ã¼zerine kurulur.

KÃ¼Ã§Ã¼k sayÄ±sal Ã¶ncelik daha yÃ¼ksek Ã¶nceliktir. Test edilen AN505/QEMU hedefinde
sekiz Ã¶ncelik biti vardÄ±r. SysTick=64, PendSV=255 seÃ§imi PendSV'nin SysTick'i
kesmesini engeller. Bunlar evrensel M33 sabitleri deÄŸildir. SysTick_Config
Ã¶nceliÄŸi de yazar; ders kendi ayarÄ±nÄ± bu Ã§aÄŸrÄ±dan sonra tekrar uygular.

## SHOW â†’ RUN

[main.c](example/main.c) kontrollÃ¼ exception sÄ±rasÄ±, pending birleÅŸmesi ve gerÃ§ek
timer ilerlemesini ayrÄ± deneylerde gÃ¶sterir. [Ã–ncelik ayarÄ±](example/priorities.c)
ayrÄ±dÄ±r. Handler'lar UART yazdÄ±rmaz; sonuÃ§lar Thread mode'da basÄ±lÄ±r.

```bash
cmake --preset debug
cmake --build --preset debug --target 14-systick-pendsv 14-pendsv-broken 14-exercise-priority 14-solution-priority
./scripts/run-qemu.sh build/debug/lessons/14-systick-pendsv/14-systick-pendsv.elf
python tests/development-smoke.py 14
```

## OBSERVE

Debug/QEMU'da Ã¶lÃ§Ã¼len ana sonuÃ§lar:

```text
[EVENT0] 1
[EVENT1] 2
[EVENT2] 3
[CHECK_DEFERRED] PASS
[CHECK_PENDING_COALESCES] PASS
[TICKS] 4
[CHECK_TICK_RELOAD] PASS
[CHECK_TICK_PROGRESS] PASS
```

20 MHz platform saati / 1000 iÃ§in LOAD=19999. Bu, donanÄ±m Ã¼zerinde Ã¶lÃ§Ã¼lmÃ¼ÅŸ
duvar saati doÄŸruluÄŸu iddiasÄ± deÄŸildir. DÃ¶rt tick beklenir; PendSV Ã§aÄŸrÄ± sayÄ±sÄ±
tick baÅŸÄ±na tam bir olmak zorunda deÄŸildir. Pending olaylarÄ± birleÅŸebilir.

GDB ile isteÄŸe baÄŸlÄ± inceleme (ayrÄ±ntÄ±lÄ± GDB kapÄ±sÄ± bu aÅŸamada Ã§alÄ±ÅŸtÄ±rÄ±lmadÄ±):

```text
break SysTick_Handler
break PendSV_Handler
print g_l14_phase
print g_l14_inside_tick
print g_l14_events
x/wx 0xE000E014
```

Breakpoint timer zamanlamasÄ±nÄ± deÄŸiÅŸtirir. Ä°lk kontrollÃ¼ deneyde timer kapalÄ±dÄ±r;
olay 1 SysTick giriÅŸini, 2 SysTick sonunu, 3 PendSV'yi gÃ¶sterir.

## MODIFY â†’ BREAK â†’ DEBUG

`14-pendsv-broken` PendSV'yi 0 yapar. Ã–lÃ§Ã¼len sÄ±ra 1,3,2 ve
CHECK_DEFERRED=FAIL: PendSV, SysTick Ã§alÄ±ÅŸÄ±rken girmiÅŸtir. Olay dizisini ve
`g_l14_nested` deÄŸerini incele; yalnÄ±zca son sayacÄ± okumak hatayÄ± gizler.

[AlÄ±ÅŸtÄ±rmada](exercises/01-defer-work/README.md) Ã¶ncelik ayarÄ± eksiktir.
Reset'teki eÅŸit Ã¶ncelikler sÄ±ra deneyini geÃ§ebilir; aÃ§Ä±k konfigÃ¼rasyon kontrolÃ¼
bu yÃ¼zden de gereklidir. [Referans Ã§Ã¶zÃ¼m](solutions/01-defer-work/README.md) ayrÄ±dÄ±r.

## VERIFY

Debug derleme ve dÃ¶rt QEMU senaryosu Ã§alÄ±ÅŸtÄ±rÄ±ldÄ±. Ã–rnek/Ã§Ã¶zÃ¼m tÃ¼m kontrolleri
geÃ§er; starter yalnÄ±z CHECK_PRIORITY, bozuk sÃ¼rÃ¼m CHECK_PRIORITY ve
CHECK_DEFERRED iÃ§in beklenen FAIL Ã¼retir. GeniÅŸ regresyon ve ayrÄ±ntÄ±lÄ± Ã¼Ã§-preset
karÅŸÄ±laÅŸtÄ±rmasÄ± toplu kalite aÅŸamasÄ±ndadÄ±r.

## Quick Check

1. Ä°ki PENDSVSET yazmasÄ± neden iki iÅŸ kaydÄ± deÄŸildir?
2. Starter neden doÄŸru olay sÄ±rasÄ±nÄ± yine de Ã¼retebilir?
3. SysTick_Config sonrasÄ±nda Ã¶nceliÄŸi niÃ§in yeniden ayarlÄ±yoruz?
4. Bu ders task deÄŸiÅŸtirdi mi?

[Cevaplar](answers/quick-check.md)

## References

- [Cortex-M33 Devices Generic User Guide](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d): ICSR, SysTick, exception priorities.
- [CMSIS implementation](../../external/CMSIS/CMSIS/Core/Include/core_cm33.h): SysTick_Config.

**Previous:** [13](../13-sync-fpu-dsp/) Â· **Next:** [15](../15-mini-kernel/)
