# Lesson 15 — Educational Mini Kernel

`[DEEP]` · **STATUS: temel geliştirme doğrulandı; toplu kalite kapısı bekliyor**

## TEACH — Bir task'ın devam edebilmesi için ne saklanır?

İki privileged Thread task'ı ayrı PSP stack'leri kullanır. Handler'lar MSP
üzerindedir. Donanım basic frame'i (R0–R3, R12, LR, PC, xPSR), PendSV ise
R4–R11 ve EXC_RETURN'ü saklar. Sabit round-robin seçiminden sonra yeni PSP ile
exception return yapılır. Bu eğitim çekirdeği FPU task context'i, MPU task
izolasyonu, unprivileged task veya üretim RTOS garantisi sağlamaz.

[Context assembly](example/context.S), [çekirdek](example/main.c) ve
[ilk frame](example/frame.c) birlikte okunmalıdır. Yazılım frame'i 9 word,
hardware frame'i 8 word'dür. Yazılım SP'si 4 mod 8 olabilir; public exception
boundary'deki hardware SP 8-byte hizalıdır. Bu farkı kaldırmak için rastgele
bir padding word eklemek register eşlemesini bozar.

## SHOW — Başlangıç ve geçiş sınırları

İlk SVC, C'ye çağrı yapmadan CONTROL.SPSEL/PSP/PSPLIM'i kurar, ilk register
setini yükler ve MSP'yi stack top'a getirir. Eski main/MSP frame'i kalıcı olarak
terk edilir. İlk task hardware frame'inden başlar; eski C frame'i PSP ile
devam ettirilmez. SVC'den sonraki bootstrap koduna dönülmez.

PendSV yalnız 0xFFFFFFFD basic Secure Thread/PSP dönüşünü kabul eder. FPU veya
başka frame çeşidi sessizce destekleniyor sayılmaz. STM'den **önce** çıkış
stack sınırları kontrol edilir; PSPLIM, R0 üzerinden yapılan STM için koruma
değildir. Scheduler'a MSP üzerinde 8-byte hizalı çağrı yapılır. Kısa seçim
bölümü eski PRIMASK değerini korur; yeni context doğrulanmadan yüklenmez.

## RUN

```bash
cmake --preset debug
cmake --build --preset debug --target 15-mini-kernel 15-kernel-broken 15-exercise-frame 15-solution-frame
./scripts/run-qemu.sh build/debug/lessons/15-mini-kernel/15-mini-kernel.elf
python tests/development-smoke.py 15
```

## OBSERVE

Debug/QEMU çalıştırmasında örnek ve çözümün dokuz CHECK alanı PASS oldu.
Kaydedilen bir örnek: TASK0_RUNS=719, TASK1_RUNS=1123, TICKS=4,
SWITCHES=20, ASYNC_SWITCHES=4. Task sayaçları/model zamanlaması değişebilir;
test bu rastlantısal sayıları sabitlemez.

Task'lar ilk sekiz tur SVC #0 ile yield eder. Sonra yield etmeden busy-loop
yaparlar: ikinci task'ın ilerleyebilmesi için gerçek SysTick/PendSV preemption
gerekir. ASYNC_SWITCHES, kaydedilen PC yield dönüş noktası olmadığında sayılır.
R4–R11'in her birine task'a özgü canary konur. Her task'ın alt/üst stack
guard'ları, ilerleme, yield ve timer switch'leri bitişte denetlenir.
Task gözleminde CONTROL/IPSR, PSP aralığı, PSPLIM ve güncel TCB kimliği
de kontrol edilir; Handler/MSP ayrı sınanır.

GDB'de isteğe bağlı gözlem (bu aşamada kapsamlı GDB kapısı çalıştırılmadı):

```text
break PendSV_Handler
print/x $psp
print g_l15_current
x/17wx g_l15_current->sp
print g_l15_runs
print g_l15_bad
```

Handler girişinde TCB'nin SP'si henüz güncellenmemiş olabilir. TCB dump'ını
ASM'nin store'undan sonra veya scheduler içinde al. Breakpoint zamanlamayı etkiler.

## MODIFY → BREAK → DEBUG

`15-kernel-broken` yazılım context alanını ayırır ama R4–R11'i yazmaz.
CHECK_REGISTERS=FAIL olur; task'ların C gözlem çağrısı boyunca IRQ maskelenir.
Böylece kasıtlı bozuk restore, askıda kalan C pointer'larını bozup rastgele
çökmek yerine ASM canary kontrolünde görünür. Üretim scheduler'ı için bir
C çağrısını maskelemek register saklama zorunluluğunu ortadan kaldırmaz.

[Alıştırmada](exercises/01-initial-frame/README.md) ilk frame oluşturucu eksiktir.
NULL, PSP kurulmadan reddedilir. [Çözüm](solutions/01-initial-frame/README.md)
ayrı dosyadadır.

## VERIFY

Debug build ve dört QEMU senaryosu çalıştırıldı: örnek/çözüm PASS, eksik starter
yalnız CHECK_INITIAL_FRAME=FAIL, bozuk save yalnız CHECK_REGISTERS=FAIL.
Bu testler WCET, adil zaman payı, sonsuz çalışma veya donanım doğrulaması değildir.
Üç-preset karşılaştırması, geniş negatif set ve bağımsız audit toplu kapıda yapılacak.

## Quick Check

1. Exception entry neden R4–R11'i kurtarmıyor?
2. PSPLIM neden PendSV'nin STM adresini ayrıca doğrulama gereğini kaldırmıyor?
3. İlk SVC'nin terk ettiği MSP frame'ine dönmek neden hatadır?
4. Yield dışında switch olduğunu hangi gözlem ayırır?

[Cevaplar](answers/quick-check.md)

## References

- [Cortex-M33 Devices Generic User Guide](https://documentation-service.arm.com/static/5f16e93d20b7cf4bc524af1d): basic frames, PSP/MSP, exception return.
- [AAPCS32](https://github.com/ARM-software/abi-aa/blob/main/aapcs32/aapcs32.rst): callee-saved registers and public stack alignment.
- [Lesson 04](../04-stack-msp-psp/): fresh C frame sınırı.
- [Lesson 13](../13-sync-fpu-dsp/): bu çekirdeğin desteklemediği FP frame'leri.

**Previous:** [14](../14-systick-pendsv/) · **Next:** [16](../16-trustzone-advanced/)
