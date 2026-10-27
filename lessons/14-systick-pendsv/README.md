# Lesson 14 — SysTick & PendSV

`[STANDARD]` · **STATUS: main üzerinde; Debug/Release/MinSize CI PASS (11fde900)**

## TEACH — Tick bir scheduler değildir

SysTick zamanı sayar; PendSV ertelenmiş işi çalıştırır. Pending biti olay kuyruğu
değildir: maskeli durumda aynı biti iki kere set etmek iki çağrı biriktirmez.
Scheduler bu mekanizmaların üzerine kurulur.

Küçük sayısal öncelik daha yüksek önceliktir. Test edilen AN505/QEMU hedefinde
sekiz öncelik biti vardır. SysTick=64, PendSV=255 seçimi PendSV'nin SysTick'i
kesmesini engeller. Bunlar evrensel M33 sabitleri değildir. SysTick_Config
önceliği de yazar; ders kendi ayarını bu çağrıdan sonra tekrar uygular.

## SHOW → RUN

[main.c](example/main.c) kontrollü exception sırası, pending birleşmesi ve gerçek
timer ilerlemesini ayrı deneylerde gösterir. [Öncelik ayarı](example/priorities.c)
ayrıdır. Handler'lar UART yazdırmaz; sonuçlar Thread mode'da basılır.

```bash
cmake --preset debug
cmake --build --preset debug --target 14-systick-pendsv 14-pendsv-broken 14-exercise-priority 14-solution-priority
./scripts/run-qemu.sh build/debug/lessons/14-systick-pendsv/14-systick-pendsv.elf
python tests/development-smoke.py 14
```

## OBSERVE

Debug/QEMU'da ölçülen ana sonuçlar:

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

20 MHz platform saati / 1000 için LOAD=19999. Bu, donanım üzerinde ölçülmüş
duvar saati doğruluğu iddiası değildir. Dört tick beklenir; PendSV çağrı sayısı
tick başına tam bir olmak zorunda değildir. Pending olayları birleşebilir.
ISR SEV üretir, Thread WFE ile bekler: son tick koşul kontrolüyle uyku arasına
girse bile sticky event kaybolmaz. Optimizasyona/host hızına bağlı bir spin
iteration bütçesi timer süresi gibi kullanılmaz; durmuş timer dış test timeout'u
ile başarısız sayılır.

GDB ile isteğe bağlı inceleme (ayrıntılı GDB kapısı bu aşamada çalıştırılmadı):

```text
break SysTick_Handler
break PendSV_Handler
print g_l14_phase
print g_l14_inside_tick
print g_l14_events
x/wx 0xE000E014
```

Breakpoint timer zamanlamasını değiştirir. İlk kontrollü deneyde timer kapalıdır;
olay 1 SysTick girişini, 2 SysTick sonunu, 3 PendSV'yi gösterir.

## MODIFY → BREAK → DEBUG

`14-pendsv-broken` PendSV'yi 0 yapar. Ölçülen sıra 1,3,2 ve
CHECK_DEFERRED=FAIL: PendSV, SysTick çalışırken girmiştir. Olay dizisini ve
`g_l14_nested` değerini incele; yalnızca son sayacı okumak hatayı gizler.

[Alıştırmada](exercises/01-defer-work/README.md) öncelik ayarı eksiktir.
Reset'teki eşit öncelikler sıra deneyini geçebilir; açık konfigürasyon kontrolü
bu yüzden de gereklidir. [Referans çözüm](solutions/01-defer-work/README.md) ayrıdır.

## VERIFY

Debug derleme ve dört QEMU senaryosu çalıştırıldı. Örnek/çözüm tüm kontrolleri
geçer; starter yalnız CHECK_PRIORITY, bozuk sürüm CHECK_PRIORITY ve
CHECK_DEFERRED için beklenen FAIL üretir. Geniş regresyon ve ayrıntılı üç-preset
karşılaştırması toplu kalite aşamasındadır.

## Quick Check

1. İki PENDSVSET yazması neden iki iş kaydı değildir?
2. Starter neden doğru olay sırasını yine de üretebilir?
3. SysTick_Config sonrasında önceliği niçin yeniden ayarlıyoruz?
4. Bu ders task değiştirdi mi?

[Cevaplar](answers/quick-check.md)

## References

- [Cortex-M33 Devices Generic User Guide](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d): ICSR, SysTick, exception priorities.
- [CMSIS implementation](../../external/CMSIS/Core/Include/core_cm33.h): SysTick_Config.

**Previous:** [13](../13-sync-fpu-dsp/) · **Next:** [15](../15-mini-kernel/)

## Son toplu kapı

Debug/Release/MinSize clean build ve dört QEMU senaryosu PASS. GDB'de PendSV
girişinde SysTick bitmiş/inside_tick=0 ve ilk olaylar 1,2 doğrulandı.
Önceki Debug kaydı yukarıdadır; geniş repo doğrulaması [Progress](../../docs/PROGRESS.md) içindedir.
