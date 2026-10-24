#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Smoke + davranissal regresyon testi.
#
# Kullanim:
#   ./tests/smoke-test.sh              # preset: debug
#   ./tests/smoke-test.sh release
#
# Cikis kodu 0 ise her sey yolunda.
#
# TASARIM (bagimsiz denetim bulgusu I8 sonrasi)
#   Eski surum cogunlukla "cikti icinde su metin var mi" diye bakiyordu ve
#   bu yuzden -O2/-Os'de BASARISIZ olan PSP cozumu testten GECIYORDU.
#   Artik her hedef turune gore SINIFLANDIRILIP beklenen DAVRANISI
#   dogrulaniyor:
#
#     normal ornek      -> dogru sonuc + tamamlanma isareti
#     referans cozum    -> tum CHECK satirlari PASS + tamamlanma
#     bilerek bozuk     -> BEYAN EDILEN yanlis sonuc ya da fault
#     bitmemis odev     -> eksik davranis (cozulmus gibi raporlanmaz)
#
#   Hicbir testte "END gorundu" tek basina basari sayilmaz.
#
# NOT: bu bir unit test cercevesi degildir ve olmayacaktir. Derslerin asil
# dogrulamasi GDB ile elle yapilir.
# ---------------------------------------------------------------------------
set -u

PRESET="${1:-debug}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build/$PRESET"
QEMU_TIMEOUT="${QEMU_TIMEOUT:-8}"

pass=0
fail=0

ok()   { echo "  [PASS] $*"; pass=$((pass + 1)); }
bad()  { echo "  [FAIL] $*"; fail=$((fail + 1)); }
sect() { echo; echo "=== $* ==="; }

# firmware'ler sonsuz dongude bitiyor; timeout ile kesiyoruz.
# Yalnizca bu script'in baslattigi sureci sonlandirir.
run_qemu() {
    timeout "$QEMU_TIMEOUT" qemu-system-arm \
        -M mps2-an505 -cpu cortex-m33 \
        -nographic -monitor none -serial stdio \
        -kernel "$1" 2>/dev/null | tr -d '\r'
}

# expect_line <cikti> <etiket> <aranan metin>
expect_line() {
    if printf '%s\n' "$1" | grep -qF -- "$3"; then ok "$2"; else bad "$2 (beklenen: '$3')"; fi
}
# reject_line <cikti> <etiket> <olmamasi gereken metin>
reject_line() {
    if printf '%s\n' "$1" | grep -qF -- "$3"; then bad "$2 (istenmeyen: '$3')"; else ok "$2"; fi
}

# --- 0. Gerekli araclar ----------------------------------------------------
sect "0. Tools"
for tool in arm-none-eabi-gcc arm-none-eabi-nm arm-none-eabi-objdump cmake ninja qemu-system-arm; do
    if command -v "$tool" >/dev/null 2>&1; then ok "$tool"; else bad "$tool YOK"; fi
done
if [ "$fail" -ne 0 ]; then
    echo; echo "Arac eksik. scripts/check-environment.sh calistir."; exit 1
fi

# --- 1. Configure + build --------------------------------------------------
sect "1. Configure + build ($PRESET)"
if cmake --preset "$PRESET" >/dev/null 2>&1; then ok "cmake --preset $PRESET"
else bad "configure basarisiz"; cmake --preset "$PRESET"; exit 1; fi
if cmake --build --preset "$PRESET" >/dev/null 2>&1; then ok "cmake --build --preset $PRESET"
else bad "build basarisiz"; cmake --build --preset "$PRESET"; exit 1; fi

# --- 2. Uretilen dosyalar --------------------------------------------------
sect "2. Build ciktilari"
CAP="$BUILD/tools/capability-probe/capability-probe.elf"
L02="$BUILD/lessons/02-programmer-model/02-programmer-model.elf"
L02B="$BUILD/lessons/02-programmer-model/02-programmer-model-broken.elf"
L02E="$BUILD/lessons/02-programmer-model/02-exercise-apsr.elf"
L02S="$BUILD/lessons/02-programmer-model/02-solution-apsr.elf"
L03="$BUILD/lessons/03-c-and-assembly/03-c-and-assembly.elf"
L03B="$BUILD/lessons/03-c-and-assembly/03-c-and-assembly-broken.elf"
L03E="$BUILD/lessons/03-c-and-assembly/03-exercise-sub.elf"
L03S="$BUILD/lessons/03-c-and-assembly/03-solution-sub.elf"
L03M="$BUILD/lessons/03-c-and-assembly/03-solution-mla.elf"
L04="$BUILD/lessons/04-stack-msp-psp/04-stack-msp-psp.elf"
L04B="$BUILD/lessons/04-stack-msp-psp/04-stack-msp-psp-broken.elf"
L04E="$BUILD/lessons/04-stack-msp-psp/04-exercise-psp.elf"
L04S="$BUILD/lessons/04-stack-msp-psp/04-solution-psp.elf"
L05="$BUILD/lessons/05-privilege-svc/05-privilege-svc.elf"
L05B="$BUILD/lessons/05-privilege-svc/05-privilege-svc-broken.elf"
L05E="$BUILD/lessons/05-privilege-svc/05-exercise-svc2.elf"
L05S="$BUILD/lessons/05-privilege-svc/05-solution-svc2.elf"
L06="$BUILD/lessons/06-mpu/06-mpu.elf"
L06B="$BUILD/lessons/06-mpu/06-mpu-broken.elf"
L06P="$BUILD/lessons/06-mpu/06-mpu-permissive.elf"
L06E="$BUILD/lessons/06-mpu/06-exercise-mpu.elf"
L06S="$BUILD/lessons/06-mpu/06-solution-mpu.elf"
L07="$BUILD/lessons/07-exceptions/07-exceptions.elf"
L07M="$BUILD/lessons/07-exceptions/07-exceptions-msp.elf"
L07F="$BUILD/lessons/07-exceptions/07-exceptions-memmanage.elf"
L07X="$BUILD/lessons/07-exceptions/07-exceptions-escalation.elf"
L07E="$BUILD/lessons/07-exceptions/07-exercise-decode.elf"
L07S="$BUILD/lessons/07-exceptions/07-solution-decode.elf"
L08M="$BUILD/lessons/08-faults/08-faults-memmanage.elf"
L08U="$BUILD/lessons/08-faults/08-faults-usage.elf"
L08B="$BUILD/lessons/08-faults/08-faults-bus.elf"
L08X="$BUILD/lessons/08-faults/08-faults-escalation.elf"
L08E="$BUILD/lessons/08-faults/08-exercise-fault-report.elf"
L08S="$BUILD/lessons/08-faults/08-solution-fault-report.elf"
L09A="$BUILD/lessons/09-compiler-aapcs/09-compiler-aapcs.elf"
L09B="$BUILD/lessons/09-compiler-aapcs/09-compiler-aapcs-broken.elf"
L09O="$BUILD/lessons/09-compiler-aapcs/09-compiler-optimization.elf"
L09E="$BUILD/lessons/09-compiler-aapcs/09-exercise-callee-save.elf"
L09S="$BUILD/lessons/09-compiler-aapcs/09-solution-callee-save.elf"
L09R="$BUILD/lessons/09-compiler-aapcs/09-harness-sp-regression.elf"
L10A="$BUILD/lessons/10-linker-elf-memory/10-linker-elf-memory.elf"
L10E="$BUILD/lessons/10-linker-elf-memory/10-exercise-linker.elf"
L10S="$BUILD/lessons/10-linker-elf-memory/10-solution-linker.elf"
L11A="$BUILD/lessons/11-startup/11-startup.elf"
L11B="$BUILD/lessons/11-startup/11-startup-broken.elf"
L11E="$BUILD/lessons/11-startup/11-exercise-startup.elf"
L11S="$BUILD/lessons/11-startup/11-solution-startup.elf"
L12DIR="$BUILD/lessons/12-debug-dwt"

for name in 12-debug-dwt 12-debug-dwt-broken 12-profile-io 12-profile-o0 12-profile-o2 \
            12-exercise-measurement 12-solution-measurement; do
    [ -f "$L12DIR/$name.elf" ] && ok "$name.elf" || bad "uretilmedi: $name.elf"
done

for f in "$CAP" "$L02" "$L02B" "$L02E" "$L02S" \
         "$L03" "$L03B" "$L03E" "$L03S" "$L03M" \
         "$L04" "$L04B" "$L04E" "$L04S" \
         "$L05" "$L05B" "$L05E" "$L05S" \
         "$L06" "$L06B" "$L06P" "$L06E" "$L06S" \
         "$L07" "$L07M" "$L07F" "$L07X" "$L07E" "$L07S" \
         "$L08M" "$L08U" "$L08B" "$L08X" "$L08E" "$L08S" \
         "$L09A" "$L09B" "$L09O" "$L09E" "$L09S" "$L09R" \
         "$L10A" "$L10E" "$L10S" \
         "$L11A" "$L11B" "$L11E" "$L11S"; do
    [ -f "$f" ] && ok "$(basename "$f")" || bad "uretilmedi: $f"
done
for f in "${L02%.elf}.bin" "${L02%.elf}.map" "${L02%.elf}.lst"; do
    [ -f "$f" ] && ok "$(basename "$f")" || bad "uretilmedi: $f"
done

# --- 3. Sembol / section ---------------------------------------------------
sect "3. Semboller ve section'lar"
if [ -f "$L02" ]; then
    syms="$(arm-none-eabi-nm "$L02")"
    for s in Reset_Handler __stack_top main; do
        echo "$syms" | grep -qw "$s" && ok "sembol: $s" || bad "sembol eksik: $s"
    done
    secs="$(arm-none-eabi-objdump -h "$L02")"
    for s in .vectors .text; do
        echo "$secs" | grep -q -- "$s" && ok "section: $s" || bad "section eksik: $s"
    done
    echo "$secs" | grep -- ".vectors" | grep -q "10000000" \
        && ok ".vectors adresi 0x10000000" || bad ".vectors beklenen adreste degil"
fi

# ===========================================================================
sect "4. capability-probe (arac)"
out="$(run_qemu "$CAP")"
for pat in "[CPUID]" "[MPU_TYPE]" "[SAU_TYPE]" "[DWT_CYCCNT_IMPLEMENTED]"; do
    expect_line "$out" "cikti: $pat" "$pat"
done
expect_line "$out" "hedef kimligi: Cortex-M33 (PARTNO 0xD21)" "PARTNO] 0x00000D21"
expect_line "$out" "MPU bolge sayisi olculdu" "[MPU_REGIONS] 8"
expect_line "$out" "SAU bolge sayisi olculdu" "[SAU_REGIONS] 8"
expect_line "$out" "NVIC oncelik biti derleme<->olcum uyumu" "[NVIC_PRIO_BITS_MATCH] YES"

# ===========================================================================
sect "5. Lesson 02 - normal ornek"
out="$(run_qemu "$L02")"
expect_line "$out" "Thread Mode raporlaniyor" "[MODE] Thread Mode"
expect_line "$out" "aktif stack MSP" "[ACTIVE_STACK] MSP"
expect_line "$out" "APSR 5-5 -> Z,C" "[APSR_AFTER_5_MINUS_5] 0x60000000"
expect_line "$out" "APSR 5-7 -> N" "[APSR_AFTER_5_MINUS_7] 0x80000000"
expect_line "$out" "tamamlandi" "===== END ====="

sect "5b. Lesson 02 - referans cozum (APSR decode)"
out="$(run_qemu "$L02S")"
expect_line "$out" "5-5 bayraklari" "[FLAGS] N=0 Z=1 C=1 V=0"
expect_line "$out" "5-7 bayraklari" "[FLAGS] N=1 Z=0 C=0 V=0"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "TODO kalmamis" "TODO(student)"

sect "5c. Lesson 02 - bitmemis odev (eksik olmali)"
out="$(run_qemu "$L02E")"
expect_line "$out" "odev hala doldurulmamis (beklenen)" "TODO(student)"
reject_line "$out" "odev cozulmus gibi raporlanmiyor" "[FLAGS] N=0 Z=1 C=1 V=0"

sect "5d. Lesson 02 - bilerek bozuk (beyan edilen fault)"
out="$(run_qemu "$L02B")"
expect_line "$out" "break it bolumune ulasti" "--- BREAK IT: PSP bilerek gecersiz"
reject_line "$out" "fault nedeniyle TAMAMLANMAMALI" "===== END ====="

# ===========================================================================
sect "6. Lesson 03 - normal ornek"
out="$(run_qemu "$L03")"
expect_line "$out" "asm toplama dogru" "[ADD_NUMBERS_ASM(7,5)] 12"
expect_line "$out" "C toplama dogru" "[ADD_NUMBERS_C(7,5)] 12"
expect_line "$out" "ikisi ayni" "[SAME_RESULT] YES"
expect_line "$out" "callee-saved korunmus" "[ACCUMULATE(7,5)] 12"
expect_line "$out" "tamamlandi" "===== END ====="

sect "6b. Lesson 03 - bilerek bozuk (beyan edilen YANLIS sonuc)"
out="$(run_qemu "$L03B")"
expect_line "$out" "ABI ihlali beyan edilen yanlis sonucu veriyor" "[ACCUMULATE(7,5)] 5"
reject_line "$out" "dogru sonuc CIKMAMALI (yoksa ihlal etkisiz)" "[ACCUMULATE(7,5)] 12"
expect_line "$out" "crash yok, tamamlaniyor" "===== END ====="

sect "6c. Lesson 03 - referans cozumler"
out="$(run_qemu "$L03S")"
expect_line "$out" "12-5 = 7" "[SUB_NUMBERS(12,5)] 7"
expect_line "$out" "5-12 ikiye tumleyen" "[SUB_NUMBERS(5,12)] 4294967289"
expect_line "$out" "CHECK 1" "[CHECK_12_MINUS_5] PASS"
expect_line "$out" "CHECK 2" "[CHECK_5_MINUS_12] PASS"
reject_line "$out" "hicbir FAIL yok" "FAIL"
out="$(run_qemu "$L03M")"
expect_line "$out" "3*4+5 = 17" "[MULTIPLY_ADD(3,4,5)] 17"
expect_line "$out" "CHECK" "[CHECK_3x4p5] PASS"

sect "6d. Lesson 03 - bitmemis odev (eksik olmali)"
out="$(run_qemu "$L03E")"
expect_line "$out" "odev hala FAIL veriyor (beklenen)" "[CHECK_12_MINUS_5] FAIL"
reject_line "$out" "odev cozulmus gibi raporlanmiyor" "[SUB_NUMBERS(12,5)] 7"

# ===========================================================================
sect "7. Lesson 04 - normal ornek"
out="$(run_qemu "$L04")"
expect_line "$out" "asama 2: PSP dolu ama SPSEL degismemis" "[2] PSP yazildi, SPSEL DEGISMEDI"
expect_line "$out" "asama 3: PSP aktif" "[3] psp_start() sonrasi -> PSP aktif"
expect_line "$out" "push/pop 16 byte" "[  PUSH_DELTA_BYTES] 16"
expect_line "$out" "SP geri yuklendi" "[  SP_RESTORED] YES"
expect_line "$out" "stack tasma nobetcisi bozulmamis" "[  CHECK_STACK_GUARD] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "7b. Lesson 04 - referans PSP cozumu (asil regresyon)"
out="$(run_qemu "$L04S")"
expect_line "$out" "SPSEL" "[CHECK_SPSEL] PASS"
expect_line "$out" "yerel degiskenler task stack icinde" "[CHECK_REGION] PASS"
expect_line "$out" "tahsis disina yazilmamis" "[CHECK_STACK_GUARD] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "7c. Lesson 04 - bitmemis odev (eksik olmali)"
out="$(run_qemu "$L04E")"
expect_line "$out" "SPSEL hala FAIL (beklenen)" "[CHECK_SPSEL] FAIL"
expect_line "$out" "REGION hala FAIL (beklenen)" "[CHECK_REGION] FAIL"
expect_line "$out" "odev yine de guvenli calisiyor" "===== END ====="

sect "7d. Lesson 04 - bilerek bozuk (beyan edilen fault)"
out="$(run_qemu "$L04B")"
expect_line "$out" "PSPLIM kuruldu" "[  PSPLIM]"
expect_line "$out" "recursion basladi" "recursion basliyor..."
reject_line "$out" "stack tasmasi nedeniyle TAMAMLANMAMALI" "===== END ====="

# ===========================================================================
sect "8. Lesson 05 - normal ornek"
out="$(run_qemu "$L05")"
expect_line "$out" "baslangic: ayricalikli Thread" "[1] Thread Mode, ayricalikli, PSP uzerinde"
expect_line "$out" "ayricalik dusuruldu" "[  CHECK_PRIV_DROPPED] PASS"
expect_line "$out" "kendini yukseltme engellendi" "[  CHECK_SELF_PROMOTION_BLOCKED] PASS"
expect_line "$out" "ayricaliksiz cevre birimi erisimi bloklu" "[  CHECK_PERIPHERAL_BLOCKED_UNPRIV] PASS"
expect_line "$out" "SVC servisi calisti" "[  CHECK_SVC_SERVICE] PASS"
expect_line "$out" "SVC otomatik yukseltme YAPMADI" "[  CHECK_SVC_DID_NOT_PROMOTE] PASS"
expect_line "$out" "ayricaligi handler geri verdi" "[  CHECK_PRIV_RESTORED_BY_HANDLER] PASS"
expect_line "$out" "Thread normal devam etti" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "8b. Lesson 05 - referans cozum (SVC #2)"
out="$(run_qemu "$L05S")"
expect_line "$out" "SVC #2 surum degeri" "[  SVC2_VERSION] 0x00010002"
expect_line "$out" "CHECK" "[  CHECK_SVC2_VERSION] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "8c. Lesson 05 - bitmemis odev (eksik olmali)"
out="$(run_qemu "$L05E")"
expect_line "$out" "SVC #2 hala tanimsiz (beklenen)" "[  UNKNOWN_SVC] 2"
expect_line "$out" "odev hala FAIL veriyor (beklenen)" "[  CHECK_SVC2_VERSION] FAIL"
reject_line "$out" "odev cozulmus gibi raporlanmiyor" "[  SVC2_VERSION] 0x00010002"
expect_line "$out" "odev yine de guvenli calisiyor" "===== END ====="

sect "8d. Lesson 05 - bilerek bozuk (tanimsiz SVC)"
out="$(run_qemu "$L05B")"
expect_line "$out" "handler tanimsiz servisi bildirdi" "[  UNKNOWN_SVC] 99"
expect_line "$out" "tanimli hata degeri dondu" "[  SVC99_RESULT] 0xFFFFFFFF"
expect_line "$out" "reddedildi" "[  CHECK_UNKNOWN_SVC_REJECTED] PASS"
expect_line "$out" "sistem saglam kaldi" "===== END ====="

# ===========================================================================
sect "9. Lesson 06 - normal ornek (MPU)"
out="$(run_qemu "$L06")"
expect_line "$out" "hedef konfigurasyonu: 8 MPU bolgesi" "[  MPU_REGIONS] 8"
expect_line "$out" "reset'te MPU kapali" "[  CHECK_MPU_INITIALLY_DISABLED] PASS"
expect_line "$out" "MPU etkinlestirildi" "[  CHECK_MPU_ENABLED] PASS"
expect_line "$out" "PRIVDEFENA" "[  CHECK_PRIVDEFENA] PASS"
expect_line "$out" "MemManage etkin" "[  CHECK_MEMFAULT_ENABLED] PASS"
expect_line "$out" "FLASH bolgesi RO + calistirilabilir" "[  CHECK_FLASH_REGION_RO_EXEC] PASS"
expect_line "$out" "Thread bolgesi RW + XN" "[  CHECK_STACK_REGION_RW_XN] PASS"
expect_line "$out" "Thread ayricaliksiz" "[  CHECK_THREAD_UNPRIV] PASS"
expect_line "$out" "izinli bolgeye yazabiliyor" "[  CHECK_UNPRIV_REGION_WRITABLE] PASS"
expect_line "$out" "korunan veriye SVC uzerinden erisim" "[  CHECK_SVC_PROTECTED_ACCESS] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "9b. Lesson 06 - referans cozum"
out="$(run_qemu "$L06S")"
expect_line "$out" "MPU etkin" "[  CHECK_MPU_ENABLED] PASS"
expect_line "$out" "FLASH bolgesi" "[  CHECK_FLASH_REGION_RO_EXEC] PASS"
expect_line "$out" "Thread bolgesi" "[  CHECK_STACK_REGION_RW_XN] PASS"
expect_line "$out" "SVC yolu" "[  CHECK_SVC_PROTECTED_ACCESS] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "9c. Lesson 06 - bitmemis odev (koruma YOK olmali)"
out="$(run_qemu "$L06E")"
expect_line "$out" "MPU hala kapali (beklenen)" "[  CHECK_MPU_ENABLED] FAIL"
expect_line "$out" "bolge programlanmamis (beklenen)" "[  CHECK_FLASH_REGION_RO_EXEC] FAIL"
expect_line "$out" "SVC yolu yine de calisiyor" "[  CHECK_SVC_PROTECTED_ACCESS] PASS"
expect_line "$out" "odev guvenli calisiyor" "===== END ====="

sect "9d. Lesson 06 - bilerek bozuk (dogrudan erisim -> MemManage)"
out="$(run_qemu "$L06B")"
expect_line "$out" "MPU politikasi kuruldu" "[  CHECK_STACK_REGION_RW_XN] PASS"
reject_line "$out" "fault nedeniyle TAMAMLANMAMALI" "===== END ====="
reject_line "$out" "ayricaliksiz bolum raporuna ULASILMAMALI" "[3] Ayricaliksiz bolumde"

sect "9e. Lesson 06 - deney: politika gevsetilince ayni yazma gecer"
out="$(run_qemu "$L06P")"
expect_line "$out" "dogrudan yazma tamamlandi" "[  DIRECT_WRITE_MARKER] 2"
expect_line "$out" "sayac dogrudan degistirildi" "[  COUNTER_AFTER_DIRECT] 0x0BADC0DE"
expect_line "$out" "MPU izin verdi" "[  CHECK_DIRECT_WRITE_SUCCEEDED] PASS (MPU izin verdi)"
expect_line "$out" "tamamlandi" "===== END ====="

# ===========================================================================
sect "10. Lesson 07 - PSP kaynakli exception"
out="$(run_qemu "$L07")"
expect_line "$out" "vector[0] bir adres degil" "[  CHECK_VEC0_IS_NOT_ADDRESS] PASS"
expect_line "$out" "handler girdisinde Thumb biti" "[  CHECK_SVCALL_THUMB_BIT] PASS"
expect_line "$out" "Thread PSP uzerinde basladi" "[  ACTIVE_STACK] PSP"
expect_line "$out" "IPSR = 11 (SVCall)" "[  CHECK_IPSR_IS_SVCALL] PASS"
expect_line "$out" "handler MSP kullandi" "[  CHECK_HANDLER_USED_MSP] PASS"
expect_line "$out" "aktif SP frame adresi DEGIL" "[  CHECK_ACTIVE_SP_NOT_FRAME] PASS"
expect_line "$out" "EXC_RETURN = 0xFFFFFFFD" "[  ENTRY_EXC_RETURN] 0xFFFFFFFD"
expect_line "$out" "frame PSP uzerinde" "[  CHECK_FRAME_ON_PSP] PASS"
expect_line "$out" "frame R0 isareti" "[  +0x00 R0] 0x11111111"
expect_line "$out" "frame R12 isareti" "[  +0x10 R12] 0xCCCCCCCC"
expect_line "$out" "8 word'un hepsi eslesiyor" "[  CHECK_FRAME_MATCHES_MARKERS] PASS"
expect_line "$out" "yigilmis xPSR T biti" "[  xPSR_T_BIT] 1"
expect_line "$out" "Thread devam etti" "[  CHECK_THREAD_RESUMED] PASS"
expect_line "$out" "servis sonucu R0'dan dondu" "[  CHECK_SVC_RESULT] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "10b. Lesson 07 - MSP kaynakli exception (deney)"
out="$(run_qemu "$L07M")"
expect_line "$out" "Thread MSP uzerinde" "[  ACTIVE_STACK] MSP"
expect_line "$out" "EXC_RETURN = 0xFFFFFFF9" "[  ENTRY_EXC_RETURN] 0xFFFFFFF9"
expect_line "$out" "frame MSP uzerinde" "[  CHECK_FRAME_ON_MSP] PASS"
expect_line "$out" "handler yine MSP kullandi" "[  CHECK_HANDLER_USED_MSP] PASS"
expect_line "$out" "IPSR yine 11" "[  CHECK_IPSR_IS_SVCALL] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "10c. Lesson 07 - MPU ihlali, MemManage ETKIN"
out="$(run_qemu "$L07F")"
expect_line "$out" "IPSR = 4 (MemManage)" "[  IPSR] 4"
expect_line "$out" "DACCVIOL + MMARVALID" "[  MMFSR] 0x00000082"
expect_line "$out" "escalation YOK" "[  HFSR_FORCED] 0"
expect_line "$out" "MMFAR gecerli" "[  MMARVALID] 1"
expect_line "$out" "MemManage etkin" "[  MEMFAULTENA] 1"
expect_line "$out" "fault raporu tamamlandi" "===== FAULT END ====="
reject_line "$out" "normal tamamlanma YOK" "===== END ====="

sect "10d. Lesson 07 - ayni ihlal, MemManage DEVRE DISI (escalation)"
out="$(run_qemu "$L07X")"
expect_line "$out" "IPSR = 3 (HardFault)" "[  IPSR] 3"
expect_line "$out" "MMFSR AYNI kaldi" "[  MMFSR] 0x00000082"
expect_line "$out" "HFSR.FORCED = 1" "[  HFSR_FORCED] 1"
expect_line "$out" "MemManage devre disi" "[  MEMFAULTENA] 0"
expect_line "$out" "fault raporu tamamlandi" "===== FAULT END ====="
reject_line "$out" "normal tamamlanma YOK" "===== END ====="

sect "10e. Lesson 07 - bitmemis odev (cozumleme bos olmali)"
out="$(run_qemu "$L07E")"
expect_line "$out" "cozumleme hala bos (beklenen)" "[  CHECK_DECODE_MODE] FAIL"
expect_line "$out" "exception numarasi cozulmedi (beklenen)" "[  CHECK_DECODE_EXCNUM] FAIL"
reject_line "$out" "odev cozulmus gibi raporlanmiyor" "[  INTERRUPTED_MODE] Thread"
expect_line "$out" "odev guvenli calisiyor" "===== END ====="

sect "10f. Lesson 07 - referans cozum"
out="$(run_qemu "$L07S")"
expect_line "$out" "kesilen mod" "[  INTERRUPTED_MODE] Thread"
expect_line "$out" "frame stack" "[  FRAME_STACK] PSP"
expect_line "$out" "exception numarasi" "[  EXCEPTION_NUMBER] 11"
expect_line "$out" "stacked R0" "[  CHECK_DECODE_R0] PASS"
expect_line "$out" "stacked xPSR" "[  CHECK_DECODE_XPSR] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

# ===========================================================================
sect "11. Lesson 08 - MemManage (temiz referans vaka)"
out="$(run_qemu "$L08M")"
expect_line "$out" "MemManage handler" "[  ACTIVE_EXCEPTION] MemManage"
expect_line "$out" "IPSR = 4" "[  IPSR] 4"
expect_line "$out" "DACCVIOL" "[  MMFSR.DACCVIOL] 1"
expect_line "$out" "MMARVALID" "[  MMFSR.MMARVALID] 1"
expect_line "$out" "MMFAR = korunan adres" "[  FAULT_ADDRESS(MMFAR)] 0x30000000"
expect_line "$out" "escalation yok" "[  ESCALATED] NO"
expect_line "$out" "frame gecerli" "[  FRAME_VALID] YES"
expect_line "$out" "fault raporu" "===== FAULT END ====="
reject_line "$out" "normal tamamlanma YOK" "===== END ====="

sect "11b. Lesson 08 - UsageFault"
out="$(run_qemu "$L08U")"
expect_line "$out" "UsageFault handler" "[  ACTIVE_EXCEPTION] UsageFault"
expect_line "$out" "IPSR = 6" "[  IPSR] 6"
expect_line "$out" "UNDEFINSTR" "[  UFSR.UNDEFINSTR] 1"
expect_line "$out" "escalation yok" "[  ESCALATED] NO"
expect_line "$out" "adres register'i kanit degil" "[  MMFAR] GECERSIZ - kanit degil"
expect_line "$out" "frame gecerli" "[  FRAME_VALID] YES"
reject_line "$out" "normal tamamlanma YOK" "===== END ====="

sect "11c. Lesson 08 - BusFault (precise)"
out="$(run_qemu "$L08B")"
expect_line "$out" "BusFault handler" "[  ACTIVE_EXCEPTION] BusFault"
expect_line "$out" "IPSR = 5" "[  IPSR] 5"
expect_line "$out" "PRECISERR" "[  BFSR.PRECISERR] 1"
expect_line "$out" "BFARVALID" "[  BFSR.BFARVALID] 1"
expect_line "$out" "BFAR = hedef adres" "[  FAULT_ADDRESS(BFAR)] 0x30100000"
expect_line "$out" "MMFAR kullanilmadi" "[  MMFAR] GECERSIZ - kanit degil"
expect_line "$out" "escalation yok" "[  ESCALATED] NO"
reject_line "$out" "normal tamamlanma YOK" "===== END ====="

sect "11d. Lesson 08 - escalation (UsageFault devre disi)"
out="$(run_qemu "$L08X")"
expect_line "$out" "HardFault handler" "[  ACTIVE_EXCEPTION] HardFault"
expect_line "$out" "IPSR = 3" "[  IPSR] 3"
expect_line "$out" "HFSR.FORCED" "[  ESCALATED] YES"
expect_line "$out" "HFSR = 0x40000000" "[  HFSR] 0x40000000"
expect_line "$out" "asil kanit CFSR'de korundu" "[  UFSR.UNDEFINSTR] 1"
expect_line "$out" "FORCED temizlikten ONCE yakalandi" "[  SNAPSHOT_HFSR] 0x40000000"
expect_line "$out" "maske tabanli HFSR temizligi (QEMU)" "[  HFSR_AFTER_CLEAR] 0x00000000"
expect_line "$out" "CFSR geri yazma temizligi" "[  CFSR_AFTER_CLEAR] 0x00000000"
expect_line "$out" "frame gecerli" "[  FRAME_VALID] YES"
reject_line "$out" "normal tamamlanma YOK" "===== END ====="

sect "11e. Lesson 08 - bitmemis odev (kararlar bos olmali)"
out="$(run_qemu "$L08E")"
expect_line "$out" "karar bos (beklenen)" "[  CHECK_D_ACTIVE] FAIL"
expect_line "$out" "sinif bos (beklenen)" "[  CHECK_D_CLASS] FAIL"
reject_line "$out" "odev cozulmus gibi raporlanmiyor" "[  D_PRIMARY_BIT] DACCVIOL"
expect_line "$out" "ham snapshot yine de uretildi" "[  MMFSR.DACCVIOL] 1"

sect "11f. Lesson 08 - referans cozum"
out="$(run_qemu "$L08S")"
for c in ACTIVE NOT_ESCALATED CLASS BIT ADDRESS FRAME; do
    expect_line "$out" "CHECK_D_$c" "[  CHECK_D_$c] PASS"
done
expect_line "$out" "asil bit" "[  D_PRIMARY_BIT] DACCVIOL"
reject_line "$out" "hicbir FAIL yok" "] FAIL"

# ===========================================================================
sect "12. Lesson 09 - AAPCS olcumu"
out="$(run_qemu "$L09A")"
expect_line "$out" "4 x u32 -> r0-r3" "[  CHECK_ARGS_4U32] PASS"
expect_line "$out" "5. u32 -> [SP+0]" "[  CHECK_ARG5_ON_STACK] PASS"
expect_line "$out" "u64 -> r2:r3 (C.3)" "[  CHECK_U64_IN_R2_R3] PASS"
expect_line "$out" "u64 -> stack, word sirasi" "[  CHECK_U64_ON_STACK] PASS"
expect_line "$out" "stack argumaninda SP mod 8 = 0" "[  CHECK_U64_STACK_SP_ALIGNED] PASS"
expect_line "$out" "u64 donus -> r0:r1" "[  CHECK_U64_RETURN_R0_R1] PASS"
expect_line "$out" "C -> asm -> C zinciri" "[  CHECK_CHAIN_RESULT] PASS"
expect_line "$out" "BL aninda SP mod 8 = 0" "[  CHECK_SP_ALIGNED_AT_CALL] PASS"
expect_line "$out" "harness pozitif kontrol" "[  CHECK_HARNESS_POSITIVE_CONTROL] PASS"
expect_line "$out" "harness BLX aninda hizali" "[  CHECK_HARNESS_SP_ALIGNED] PASS"
expect_line "$out" "good_callee sonucu" "[  CHECK_RESULT] PASS"
expect_line "$out" "good_callee sozlesmesi" "[  CHECK_CALLEE_SAVED] PASS"
expect_line "$out" "bozulan register yok" "[  CORRUPTED] (yok)"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "12b. Lesson 09 - bilerek bozuk (dogru sonuc, bozuk sozlesme)"
out="$(run_qemu "$L09B")"
expect_line "$out" "olcum aleti once dogrulandi" "[  CHECK_HARNESS_POSITIVE_CONTROL] PASS"
expect_line "$out" "sonuc DOGRU" "[  CHECK_RESULT] PASS"
expect_line "$out" "sozlesme BOZUK (beklenen)" "[  CHECK_CALLEE_SAVED] FAIL"
expect_line "$out" "bozulan register r4" "[  CORRUPTED] r4"
expect_line "$out" "maske yalnizca r4" "[  TESTED_MASK] 0x00000001"
expect_line "$out" "normal tamamlandi (cokme yok)" "===== END ====="

sect "12c. Lesson 09 - optimizasyon fikstuu (anlam preset'ten bagimsiz)"
out="$(run_qemu "$L09O")"
expect_line "$out" "checksum sabit" "[  OPT_CHECKSUM] 0x79DC404C"
expect_line "$out" "checksum kontrolu" "[  CHECK_OPT_CHECKSUM] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "12d. Lesson 09 - bitmemis odev (sonuc dogru, sozlesme bozuk olmali)"
out="$(run_qemu "$L09E")"
expect_line "$out" "olcum aleti dogrulandi" "[  CHECK_HARNESS_POSITIVE_CONTROL] PASS"
expect_line "$out" "sonuc dogru (beklenen)" "[  CHECK_RESULT] PASS"
expect_line "$out" "r5 korunmuyor (beklenen)" "[  CHECK_CALLEE_SAVED] FAIL"
expect_line "$out" "bozulan register r5" "[  CORRUPTED] r5"
expect_line "$out" "BL aninda hizasiz (beklenen)" "[  CHECK_SP_ALIGNED_AT_CALL] FAIL"
reject_line "$out" "odev cozulmus gibi raporlanmiyor" "[  CORRUPTED] (yok)"
expect_line "$out" "odev guvenli calisiyor" "===== END ====="

sect "12e. Lesson 09 - referans cozum"
out="$(run_qemu "$L09S")"
expect_line "$out" "sonuc" "[  CHECK_RESULT] PASS"
expect_line "$out" "callee-saved korundu" "[  CHECK_CALLEE_SAVED] PASS"
expect_line "$out" "BL aninda hizali" "[  CHECK_SP_ALIGNED_AT_CALL] PASS"
expect_line "$out" "bozulan register yok" "[  CORRUPTED] (yok)"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

# ===========================================================================
sect "12f. Lesson 09 - olcum aleti regresyonu (SP bozan callee)"
out="$(run_qemu "$L09R")"
expect_line "$out" "pozitif kontrol" "[1] good_callee (pozitif kontrol)"
expect_line "$out" "SP +4 vakasina ulasildi" "[2] sp_up_callee (SP +4)"
expect_line "$out" "SP -8 vakasina ulasildi" "[3] sp_down_callee (SP -8)"
expect_line "$out" "r4 + SP vakasina ulasildi" "[4] sp_up_r4_callee (r4 + SP +4)"
expect_line "$out" "SP biti raporlandi" "[  MASK] 0x00000100"
expect_line "$out" "r4 + SP birlikte raporlandi" "[  MASK] 0x00000101"
expect_line "$out" "cagiranin r4'u isaretci sanilmadi" "[  CALLER_R4_AFTER] 0xDEAD0000"
expect_line "$out" "harness normal dondu (BusFault yok)" "===== END ====="
reject_line "$out" "hicbir FAIL yok (maske/sonuc/r4-r11/SP)" "] FAIL"

# ===========================================================================
sect "13. Lesson 10 - linker, ELF ve bellek yerlesimi"
out="$(run_qemu "$L10A")"
expect_line "$out" "fonksiyon FLASH'ta" "[  CHECK_TEXT_IN_FLASH] PASS"
expect_line "$out" "fonksiyon isaretcisinde Thumb biti" "[  CHECK_FN_PTR_THUMB_BIT] PASS"
expect_line "$out" "rodata + ozel section FLASH salt-okunur bolgede" "[  CHECK_RODATA_IN_FLASH_RO] PASS"
expect_line "$out" "g_init + asm_word .data icinde" "[  CHECK_DATA_IN_DATA] PASS"
expect_line "$out" "g_zero + static yerel .bss icinde" "[  CHECK_BSS_IN_BSS] PASS"
expect_line "$out" ".data ilk degeri LMA'da (FLASH)" "[  CHECK_DATA_LMA_HOLDS_INIT] PASS"
expect_line "$out" ".data RAM'e kopyalandi" "[  CHECK_DATA_COPIED] PASS"
expect_line "$out" ".bss sifir" "[  CHECK_BSS_ZERO] PASS"
expect_line "$out" "static yerel calisiyor" "[  CHECK_STATIC_LOCAL] PASS"
expect_line "$out" "__flash_load_end tutarli" "[  CHECK_FLASH_LOAD_END] PASS"
expect_line "$out" "vector[0] = __stack_top" "[  CHECK_VECTOR0_IS_STACK_TOP] PASS"
expect_line "$out" "TU'lar arasi cagri" "[  CHECK_CROSS_TU_CALL] PASS"
expect_line "$out" "asm TU link edildi" "[  CHECK_ASM_LINKED] PASS"
expect_line "$out" "ilk deger LMA'da okundu" "[  deger @ LMA (FLASH)] 0xC0DEFACE"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "13b. Lesson 10 - bitmemis odev (hesaplar bos olmali)"
out="$(run_qemu "$L10E")"
expect_line "$out" "FLASH hesabi bos (beklenen)" "[  CHECK_FLASH_USED] FAIL"
expect_line "$out" "RAM hesabi bos (beklenen)" "[  CHECK_RAM_STATIC] FAIL"
expect_line "$out" "LMA bos (beklenen, fault yok)" "[  CHECK_LMA_HOLDS_INIT] FAIL"
expect_line "$out" "bolge testi bos (beklenen)" "[  CHECK_REGIONS] FAIL"
reject_line "$out" "odev cozulmus gibi raporlanmiyor" "[  CHECK_LMA_HOLDS_INIT] PASS"
expect_line "$out" "odev guvenli calisiyor" "===== END ====="

sect "13c. Lesson 10 - referans cozum"
out="$(run_qemu "$L10S")"
expect_line "$out" "FLASH kullanimi" "[  CHECK_FLASH_USED] PASS"
expect_line "$out" "statik RAM" "[  CHECK_RAM_STATIC] PASS"
expect_line "$out" "LMA ilk degeri tutuyor" "[  CHECK_LMA_HOLDS_INIT] PASS"
expect_line "$out" "bolgeler" "[  CHECK_REGIONS] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

# ===========================================================================
sect "14. Lesson 11 - gercek startup yolu (reset -> main)"
out="$(run_qemu "$L11A")"
expect_line "$out" "vector[0] = __stack_top" "[  CHECK_VECTOR0_EQ_STACK_TOP] PASS"
expect_line "$out" "vector[1] = Reset_Handler" "[  CHECK_VECTOR1_EQ_RESET_HANDLER] PASS"
expect_line "$out" "vector[1] Thumb biti" "[  CHECK_VECTOR1_THUMB_BIT] PASS"
expect_line "$out" ".data ilk degeri" "[  CHECK_DATA_INIT] PASS"
expect_line "$out" ".bss sifir (tek basina kanit degil)" "[  CHECK_BSS_ZERO] PASS"
expect_line "$out" "main girisinde SP 8'e hizali" "[  CHECK_MAIN_SP_ALIGNED] PASS"
expect_line "$out" "main girisinde SP stack bolgesinde" "[  CHECK_MAIN_SP_BELOW_STACK_TOP] PASS"
expect_line "$out" "main'in LR'si Reset_Handler'a" "[  CHECK_MAIN_LR_IN_RESET_HANDLER] PASS"
expect_line "$out" "SystemInit UART'i baslatti" "[  CHECK_SYSTEMINIT_UART] PASS"
expect_line "$out" "VTOR = vektor tablosu" "[  CHECK_VTOR_IS_VECTORS] PASS"
expect_line "$out" "Thread mode, ayricalikli" "[  CHECK_THREAD_MODE_PRIVILEGED] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
expect_line "$out" "main donuyor" "main donuyor -> Reset_Handler'in WFI dongusu"
reject_line "$out" "hicbir FAIL yok" "] FAIL"

sect "14b. Lesson 11 - bilerek bozuk (.bss sifirlanmiyor)"
out="$(run_qemu "$L11B")"
expect_line "$out" ".data yine dogru" "[  CHECK_DATA_INIT] PASS"
expect_line "$out" ".bss sifirlanmadi (beklenen)" "[  CHECK_BSS_ZERO] FAIL"
expect_line "$out" "zehir gorunuyor" "[  g_l11_zero] 0xA5A5A5A5"
expect_line "$out" "faz sirasi dogru" "[  CHECK_PHASE_ORDER] PASS"
expect_line "$out" "normal tamamlandi (fault yok)" "===== END ====="

sect "14c. Lesson 11 - bitmemis odev (iki dongu eksik)"
out="$(run_qemu "$L11E")"
expect_line "$out" ".data kopyalanmadi (beklenen)" "[  CHECK_DATA_INIT] FAIL"
expect_line "$out" ".bss sifirlanmadi (beklenen)" "[  CHECK_BSS_ZERO] FAIL"
expect_line "$out" "faz sirasi korunuyor" "[  CHECK_PHASE_ORDER] PASS"
reject_line "$out" "odev cozulmus gibi raporlanmiyor" "[  g_l11_init] 0x11C0FFEE"
expect_line "$out" "odev guvenli calisiyor" "===== END ====="

sect "14d. Lesson 11 - referans cozum"
out="$(run_qemu "$L11S")"
expect_line "$out" ".data" "[  CHECK_DATA_INIT] PASS"
expect_line "$out" ".bss" "[  CHECK_BSS_ZERO] PASS"
expect_line "$out" "faz sirasi" "[  CHECK_PHASE_ORDER] PASS"
expect_line "$out" "vector[0]" "[  CHECK_VECTOR0_EQ_STACK_TOP] PASS"
expect_line "$out" "tamamlandi" "===== END ====="
reject_line "$out" "hicbir FAIL yok" "] FAIL"

# ===========================================================================
sect "15. Lesson 12 - debugging ve olcum sinirlari"
for name in 12-debug-dwt 12-debug-dwt-broken 12-profile-io 12-profile-o0 12-profile-o2; do
    out="$(run_qemu "$L12DIR/$name.elf")"
    watch=PASS; clean=PASS
    [ "$name" = "12-debug-dwt-broken" ] && watch=FAIL
    [ "$name" = "12-profile-io" ] && clean=FAIL
    expect_line "$out" "$name: veri yazimi" "[CHECK_WATCH_VALUE] $watch"
    expect_line "$out" "$name: temiz pencere" "[CHECK_CLEAN_WINDOW] $clean"
    for check in WORKLOAD TIMER_CONFIG TIMER_PROGRESS NO_FAULT; do
        expect_line "$out" "$name: $check" "[CHECK_$check] PASS"
    done
    expect_line "$out" "$name: sonuc" "[CHECKSUM] 424"
    expect_line "$out" "$name: DWT gozlemi" "[DWT_CHANGED] NO"
    expect_line "$out" "$name: cycle iddiasi yok" "[CYCLE_BENCHMARK] NOT VALIDATED ON QEMU"
    expect_line "$out" "$name: tamamlandi" "===== END ====="
done
for kind in exercise solution; do
    out="$(run_qemu "$L12DIR/12-$kind-measurement.elf")"
    expected=FAIL; count=0
    if [ "$kind" = solution ]; then expected=PASS; count=12; fi
    for case_name in FORWARD WRAP STOPPED_ZERO STOPPED_NONZERO NO_TRACE DISABLED \
                     NO_COUNTER SECURE_DISABLED IO IO_AND_STOPPED DELTA_FORWARD DELTA_WRAP; do
        expect_line "$out" "$kind: $case_name" "[$case_name] $expected"
    done
    expect_line "$out" "$kind: tum vakalar" "[CASES_PASSED] $count"
    expect_line "$out" "$kind: tamamlandi" "===== END ====="
done

# --- Ozet -----------------------------------------------------------------
echo
echo "---------------------------------------------"
echo " PASS: $pass    FAIL: $fail    (preset: $PRESET)"
echo "---------------------------------------------"
[ "$fail" -eq 0 ] || exit 1
echo "Smoke + regresyon testleri gecti."
