#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Build seviyesi regresyon probe'lari.
#
# Iki bagimsiz denetim bulgusunu kalici olarak yakalar:
#
#   I4  libgcc siralamasi
#       -lgcc link satirinda nesne dosyalarindan ONCE gelirse, GNU ld'nin
#       tek gecisli arsiv cozumlemesi derleyicinin urettigi yardimci
#       rutinleri (ornek: __aeabi_uldivmod) bulamaz.
#
#   I5  FLASH yuk goruntusu siniri
#       ".data : AT(adres) > RAM" LMA'yi soyler ama o baytlari FLASH
#       bolgesinden DUSMEZ; tasma sessiz kalir.
#       ".data : > RAM AT > FLASH" ise tasmayi link hatasina cevirir.
#
#   L10 Ders 10 (Linker, ELF & Memory)
#       (a) Break It: ".global" olmayan bir asm fonksiyonu -> link hatasi
#           "undefined reference" (kasitli, deterministik).
#       (b) Calisma zamaninin kanitlayamadigi statik gozlemler: GC, ozel
#           input section yerlesimi, relocation turleri, .data VMA/LMA,
#           .bss dosya/bellek boyutu.
#       (c) Exercise surucusunun sozlesmesi: ogrencinin dondurdugu HICBIR
#           LMA degeri firmware'i dusurmemeli. Tasma yapan adresler
#           (0xFFFFFFFC ...) dahil, gecersiz her deger FAIL + END uretmeli.
#
#   L11 Ders 11 (Startup: Reset -> main)
#       (a) Statik: vektor tablosu <-> linker sembolleri, Reset_Handler'in
#           MANTIKSAL sirasi (komut kaliplari degil), weak alias / guclu
#           override, weak SystemInit'in atilmasi, ozel startup hedeflerinin
#           paylasilan startup'i LINK ETMEMESI, yan urunler.
#       (b) Runtime (QEMU): ders-yerel startup ile "QEMU maskeleme" deneyi.
#           Eksik .bss sifirlamasi zehirleme OLMADAN gorunmez (QEMU GOZLEMI).
#
# Kullanim:
#   ./tests/build-probes.sh              # preset: debug
#   ./tests/build-probes.sh release
#
# Cikis kodu 0 = hepsi gecti.
# ---------------------------------------------------------------------------
set -u

PRESET="${1:-debug}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LD="$ROOT/platform/mps2-an505/linker/mps2-an505-secure.ld"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

CFLAGS=(-mcpu=cortex-m33 -mthumb -mfloat-abi=soft -ffreestanding -Og -g3
        -I"$ROOT/platform/mps2-an505/include"
        -I"$ROOT/common/include"
        -I"$ROOT/external/CMSIS/Core/Include")
PLATFORM_SRC=(
    "$ROOT/platform/mps2-an505/src/startup_mps2_an505.c"
    "$ROOT/platform/mps2-an505/src/system_init.c"
    "$ROOT/platform/mps2-an505/src/uart_cmsdk.c"
    "$ROOT/common/src/console.c"
    "$ROOT/common/src/tiny_libc.c")

pass=0
fail=0
ok()  { echo "  [PASS] $*"; pass=$((pass + 1)); }
bad() { echo "  [FAIL] $*"; fail=$((fail + 1)); }

# ===========================================================================
echo
echo "=== I4: libgcc siralamasi ==="

cat > "$WORK/udiv.c" <<'EOF'
#include <stdint.h>
volatile uint64_t g_num = 1234567890123456789ULL;
volatile uint64_t g_den = 987654321ULL;
volatile uint64_t g_res;
int main(void) { g_res = g_num / g_den; for (;;) { } }
EOF

arm-none-eabi-gcc "${CFLAGS[@]}" -c "$WORK/udiv.c" -o "$WORK/udiv.o" 2>/dev/null
if arm-none-eabi-nm "$WORK/udiv.o" | grep -q "U __aeabi_uldivmod"; then
    ok "probe gercekten __aeabi_uldivmod talep ediyor (test anlamli)"
else
    bad "probe yardimci rutin talep etmiyor - test anlamsiz, probe'u guncelle"
fi

# Yanlis sira: kutuphane nesne dosyasindan ONCE -> cozumlenmemeli.
if arm-none-eabi-gcc "${CFLAGS[@]}" -nostdlib -T"$LD" \
        -lgcc "$WORK/udiv.o" "${PLATFORM_SRC[@]}" \
        -o "$WORK/wrong.elf" >/dev/null 2>&1; then
    bad "beklenmedik: yanlis sirada link BASARILI oldu (toolchain davranisi farkli?)"
    echo "         not: bu bir regresyon degil, ortam farki olabilir - bkz. rapor"
else
    ok "yanlis sira (-lgcc once) beklendigi gibi link edilemiyor"
fi

# Dogru sira: repo'nun kullandigi duzen.
if arm-none-eabi-gcc "${CFLAGS[@]}" -nostdlib -T"$LD" \
        "$WORK/udiv.o" "${PLATFORM_SRC[@]}" -lgcc \
        -o "$WORK/right.elf" >/dev/null 2>&1; then
    ok "dogru sira (nesneler once, -lgcc sonra) link ediliyor"
else
    bad "dogru sirada link BASARISIZ - repo build'i bozuk"
fi

# Repo'nun gercek link komutu libgcc'yi nesnelerden sonra mi koyuyor?
DB="$ROOT/build/$PRESET"
if [ -d "$DB" ] && command -v ninja >/dev/null 2>&1; then
    CMD="$(ninja -C "$DB" -t commands capability-probe 2>/dev/null | tail -1)"
    OBJ_POS=$(echo "$CMD" | tr ' ' '\n' | grep -n '\.obj$' | tail -1 | cut -d: -f1)
    LIB_POS=$(echo "$CMD" | tr ' ' '\n' | grep -n -- '-lgcc' | tail -1 | cut -d: -f1)
    if [ -n "$OBJ_POS" ] && [ -n "$LIB_POS" ] && [ "$LIB_POS" -gt "$OBJ_POS" ]; then
        ok "gercek link komutunda -lgcc nesne dosyalarindan SONRA"
    else
        bad "gercek link komutunda -lgcc konumu yanlis (obj=$OBJ_POS lib=$LIB_POS)"
    fi
else
    echo "  [SKIP] $DB yok; once 'cmake --build --preset $PRESET' calistir"
fi

# ===========================================================================
echo
echo "=== I5: FLASH yuk goruntusu siniri ==="

# (a) Sinir icinde, BOS OLMAYAN .data -> link edilmeli ve RAM'e ulasmali.
cat > "$WORK/small.c" <<'EOF'
#include <stdint.h>
#include "common/console.h"
volatile uint32_t g_sentinel = 0xC0FFEE01u;   /* .data: sifir degil */
volatile uint32_t g_zero;                      /* .bss                */
int main(void)
{
    console_line("");
    console_kv_hex("SENTINEL", g_sentinel);
    console_kv_hex("BSS_ZERO", g_zero);
    console_kv_str("CHECK_DATA_INIT", (g_sentinel == 0xC0FFEE01u) ? "PASS" : "FAIL");
    console_kv_str("CHECK_BSS_ZERO",  (g_zero == 0u) ? "PASS" : "FAIL");
    console_line("===== END =====");
    for (;;) { }
}
EOF

if arm-none-eabi-gcc "${CFLAGS[@]}" -nostdlib -T"$LD" \
        "$WORK/small.c" "${PLATFORM_SRC[@]}" -lgcc \
        -o "$WORK/small.elf" >/dev/null 2>&1; then
    ok "sinir icindeki imaj (bos olmayan .data) link ediliyor"

    LMA=$(arm-none-eabi-objdump -h "$WORK/small.elf" | awk '/\.data/{getline_lma=$5; print $5; exit}')
    VMA=$(arm-none-eabi-objdump -h "$WORK/small.elf" | awk '/\.data/{print $4; exit}')
    if [ "$LMA" != "$VMA" ]; then
        ok ".data LMA ($LMA) != VMA ($VMA) - FLASH'ta duruyor, RAM'de calisiyor"
    else
        bad ".data LMA == VMA; yuk adresi FLASH'a atanmamis"
    fi
    case "$LMA" in
        1*) ok ".data LMA FLASH araliginda (0x$LMA)" ;;
        *)  bad ".data LMA FLASH araliginda degil (0x$LMA)" ;;
    esac

    if command -v qemu-system-arm >/dev/null 2>&1; then
        OUT=$(timeout 15 qemu-system-arm -M mps2-an505 -cpu cortex-m33 \
                -nographic -monitor none -serial stdio \
                -kernel "$WORK/small.elf" 2>/dev/null | tr -d '\r')
        echo "$OUT" | grep -q "CHECK_DATA_INIT] PASS" \
            && ok ".data calisma zamaninda dogru baslatildi" \
            || bad ".data baslatilmamis (startup kopyalama bozuk?)"
        echo "$OUT" | grep -q "CHECK_BSS_ZERO] PASS" \
            && ok ".bss calisma zamaninda sifirlanmis" \
            || bad ".bss sifirlanmamis"
    fi
else
    bad "sinir icindeki imaj link EDILEMEDI - linker script bozuk"
fi

# (b) Asil test: .text + .rodata FLASH'a SIGIYOR, .data RAM'e SIGIYOR,
#     ama .data'nin FLASH'taki KOPYASI eklenince FLASH tasiyor.
#     Eski "AT(__etext)" duzeninde bu durum SESSIZ kaliyordu.
#     ~508 KB salt-okunur veri + 8 KB baslatilmis veri.
cat > "$WORK/big.c" <<'EOF'
#include <stdint.h>
/* FLASH'i neredeyse dolduran salt-okunur veri (.rodata). */
const volatile uint32_t g_ro[127u * 1024u] = { 0xA5A5A5A5u };
/* RAM'e rahatca sigan, ama FLASH'ta da kopyasi olan baslatilmis veri. */
volatile uint32_t g_rw[2u * 1024u] = { 0x5A5A5A5Au };
int main(void) { g_rw[1] = g_ro[1]; for (;;) { } }
EOF

if arm-none-eabi-gcc "${CFLAGS[@]}" -nostdlib -T"$LD" \
        "$WORK/big.c" "${PLATFORM_SRC[@]}" -lgcc \
        -o "$WORK/big.elf" 2>"$WORK/big.err"; then
    bad "FLASH'i asan yuk goruntusu SESSIZCE link edildi (I5 regresyonu)"
    arm-none-eabi-objdump -h "$WORK/big.elf" | grep "\.data" | head -1
else
    if grep -qiE "region .?FLASH|FLASH overflow" "$WORK/big.err"; then
        ok "FLASH yuk goruntusu tasmasi link hatasi veriyor"
        echo "         $(grep -iE "region .?FLASH|FLASH overflow" "$WORK/big.err" | head -1 | sed 's|.*ld: ||')"
    else
        bad "link basarisiz ama sebep FLASH siniri degil:"
        head -3 "$WORK/big.err"
    fi
fi

# ===========================================================================
echo
echo "=== L10a: Break It - .global olmayan asm sembolu ==="

L10="$ROOT/lessons/10-linker-elf-memory/example"
L10_SRC=("$L10/main.c" "$L10/objects.c" "$L10/placement.c")
L10_FLAGS=("${CFLAGS[@]}" -ffunction-sections -fdata-sections -I"$L10")

if arm-none-eabi-gcc "${L10_FLAGS[@]}" -c "$L10/l10_asm_broken.S" \
        -o "$WORK/l10_broken.o" 2>/dev/null; then
    if arm-none-eabi-nm "$WORK/l10_broken.o" | grep -qE "^[0-9a-f]+ t asm_fn$"; then
        ok "asm_fn bozuk nesnede VAR ama LOCAL ('t')"
    else
        bad "asm_fn bozuk nesnede beklenen LOCAL sembol degil"
    fi
else
    bad "l10_asm_broken.S derlenemedi"
fi

if arm-none-eabi-gcc "${L10_FLAGS[@]}" -nostdlib -Wl,--gc-sections -T"$LD" \
        "${L10_SRC[@]}" "$L10/l10_asm.S" "${PLATFORM_SRC[@]}" -lgcc \
        -o "$WORK/l10_ok.elf" >/dev/null 2>&1; then
    ok "kontrol: .global asm_fn ile link BASARILI"
else
    bad "kontrol: dogru kaynakla link basarisiz - probe anlamsiz"
fi

if arm-none-eabi-gcc "${L10_FLAGS[@]}" -nostdlib -Wl,--gc-sections -T"$LD" \
        "${L10_SRC[@]}" "$L10/l10_asm_broken.S" "${PLATFORM_SRC[@]}" -lgcc \
        -o "$WORK/l10_broken.elf" 2>"$WORK/l10_broken.err"; then
    bad "beklenmedik: .global olmadan link BASARILI oldu"
else
    if grep -q "undefined reference to .asm_fn'" "$WORK/l10_broken.err"; then
        ok "link beklendigi gibi basarisiz: undefined reference to 'asm_fn'"
    else
        bad "link basarisiz ama beklenen tanilama yok:"
        head -3 "$WORK/l10_broken.err"
    fi
fi

# ===========================================================================
echo
echo "=== L10b: statik gozlemler (repo build'i uzerinden) ==="

L10B="$ROOT/build/$PRESET/lessons/10-linker-elf-memory"
L10E="$L10B/10-linker-elf-memory.elf"
L10M="$L10B/10-linker-elf-memory.map"
L10O="$L10B/CMakeFiles/10-linker-elf-memory.dir/example"
if [ -f "$L10E" ] && [ -f "$L10M" ]; then
    DISC="$(awk '/^Discarded input sections/{f=1;next} /^Memory Configuration/{f=0} f' "$L10M")"
    for s in .data.g_unused_data .text.unused_function; do
        echo "$DISC" | grep -q "^ $s" && ok "GC: $s atildi" || bad "GC: $s atilmadi"
    done
    # 'used' derleyiciye aittir; bu toolchain'de linker GC'sini engellemedi.
    echo "$DISC" | grep -q "^ .rodata.k_unused_used" \
        && ok "used != KEEP: .rodata.k_unused_used yine de atildi (olculen)" \
        || bad "used != KEEP gozlemi degisti: .rodata.k_unused_used korundu"

    awk '/^\.rodata /,/^\.data /' "$L10M" | grep -q "^ \.rodata\.l10_table" \
        && ok "ozel input section .rodata.l10_table -> .rodata cikis section'i" \
        || bad ".rodata.l10_table beklenen cikis section'inda degil"

    arm-none-eabi-readelf -r "$L10O/main.c.obj" | awk '/\.rel\.text/{f=1} /\.rel\.debug/{f=0} f' \
        | grep -q "R_ARM_THM_CALL.*text_fn" \
        && ok "main.o: BL text_fn -> R_ARM_THM_CALL" \
        || bad "main.o: text_fn icin R_ARM_THM_CALL yok"
    arm-none-eabi-readelf -r "$L10O/l10_asm.S.obj" | awk '/\.rel\.text/{f=1} /\.rel\.debug/{f=0} f' \
        | grep -q "R_ARM_THM_JUMP24.*text_fn" \
        && ok "l10_asm.o: B.W text_fn -> R_ARM_THM_JUMP24" \
        || bad "l10_asm.o: text_fn icin R_ARM_THM_JUMP24 yok"

    DATA_VMA=$(arm-none-eabi-objdump -h "$L10E" | awk '$2==".data"{print $4}')
    DATA_LMA=$(arm-none-eabi-objdump -h "$L10E" | awk '$2==".data"{print $5}')
    case "$DATA_VMA:$DATA_LMA" in
        3*:1*) ok ".data VMA RAM'de (0x$DATA_VMA), LMA FLASH'ta (0x$DATA_LMA)" ;;
        *)     bad ".data VMA/LMA beklenmedik: $DATA_VMA / $DATA_LMA" ;;
    esac

    # .data + .bss ayni LOAD segmentinde: FileSiz = .data, MemSiz = .data + .bss
    SEG=$(arm-none-eabi-readelf -l -W "$L10E" | awk '$1=="LOAD" && $3 ~ /^0x3/ {print $5, $6}')
    FS=$(( $(echo "$SEG" | awk '{print $1}') ))
    MS=$(( $(echo "$SEG" | awk '{print $2}') ))
    if [ "$MS" -gt "$FS" ]; then
        ok "RAM segmenti: FileSiz ($FS) < MemSiz ($MS) - .bss dosyada yer tutmuyor"
    else
        bad "RAM segmenti FileSiz/MemSiz beklenmedik: $SEG"
    fi
else
    echo "  [SKIP] $L10E yok; once 'cmake --build --preset $PRESET' calistir"
fi

# ===========================================================================
echo
echo "=== L10c: Exercise surucusu - gecersiz LMA asla fault uretmemeli ==="

# Ogrencinin locate.c'sini taklit eden, data_lma_of() icin VERILEN degeri
# donduren gecici bir kaynak. Surucu (locate_main.c) degistirilmez.
l10_lma_case() {
    local label="$1" expr="$2" expect="$3"
    cat > "$WORK/locate_adv.c" <<C
#include <stdint.h>
#include "l10.h"
#include "locate.h"
uint32_t flash_used_bytes(void) { return 0u; }
uint32_t ram_static_bytes(void) { return 0u; }
uint32_t data_lma_of(const void *p) { (void)p; return (uint32_t)($expr); }
int in_region(const void *p, uint32_t o, uint32_t l) { (void)p; (void)o; (void)l; return 0; }
C
    if ! arm-none-eabi-gcc "${L10_FLAGS[@]}" -nostdlib -Wl,--gc-sections -T"$LD" \
            "$L10/locate_main.c" "$L10/objects.c" "$L10/placement.c" \
            "$WORK/locate_adv.c" "${PLATFORM_SRC[@]}" -lgcc \
            -o "$WORK/locate_adv.elf" >/dev/null 2>&1; then
        bad "LMA $label: probe derlenemedi"
        return
    fi
    local out
    out=$(timeout 10 qemu-system-arm -M mps2-an505 -cpu cortex-m33 \
            -nographic -monitor none -serial stdio \
            -kernel "$WORK/locate_adv.elf" 2>/dev/null | tr -d '\r')
    if echo "$out" | grep -q "===== END =====" && \
       echo "$out" | grep -q "CHECK_LMA_HOLDS_INIT\] $expect"; then
        ok "LMA $label -> $expect, END'e ulasildi (fault yok)"
    else
        bad "LMA $label: beklenen '$expect' + END yok (fault?)"
    fi
}

if command -v qemu-system-arm >/dev/null 2>&1; then
    # Pozitif kontrol: gercek LMA (bu programda .data = yalnizca g_init).
    l10_lma_case "__etext (gercek LMA)"            '(uintptr_t)&__etext'                PASS
    # 32 bit tasma: "addr + 4" sarip araliga donerdi.
    l10_lma_case "0xFFFFFFFC"                      '0xFFFFFFFCu'                        FAIL
    l10_lma_case "0xFFFFFFFD"                      '0xFFFFFFFDu'                        FAIL
    l10_lma_case "0xFFFFFFFF"                      '0xFFFFFFFFu'                        FAIL
    # FLASH'in altinda.
    l10_lma_case "0x0FFFFFFC (FLASH alti)"         '0x0FFFFFFCu'                        FAIL
    # Haric bitise 4 byte'tan yakin / tam bitis.
    l10_lma_case "__flash_load_end - 2"            '(uintptr_t)&__flash_load_end - 2u'  FAIL
    l10_lma_case "__flash_load_end (haric bitis)"  '(uintptr_t)&__flash_load_end'       FAIL
    # Hizasiz.
    l10_lma_case "__etext + 2 (hizasiz)"           '(uintptr_t)&__etext + 2u'           FAIL
else
    echo "  [SKIP] qemu-system-arm yok"
fi

# ===========================================================================
echo
echo "=== L11a: Startup statik iliskiler (repo build'i uzerinden) ==="

L11B="$ROOT/build/$PRESET/lessons/11-startup"
L11E="$L11B/11-startup.elf"
if [ -f "$L11E" ] && [ -f "$L11B/11-startup.map" ]; then
    # vector[i] (little-endian word) okuyucu
    l11_vec() {  # $1 = elf, $2 = index
        arm-none-eabi-objcopy -O binary -j .vectors "$1" "$WORK/vec.bin" &&
        od -An -tx4 -j $(( $2 * 4 )) -N 4 "$WORK/vec.bin" | tr -d ' '
    }
    l11_sym() {  # $1 = elf, $2 = sembol -> readelf deger (Thumb bitli olabilir)
        arm-none-eabi-readelf -sW "$1" | awk -v n="$2" '$8==n {print $2; exit}'
    }
    V0=$(l11_vec "$L11E" 0); V1=$(l11_vec "$L11E" 1); V15=$(l11_vec "$L11E" 15)
    ST=$(arm-none-eabi-nm "$L11E" | awk '$3=="__stack_top"{print $1}')
    RH=$(l11_sym "$L11E" Reset_Handler)
    [ "$V0" = "$ST" ] && ok "vector[0] = __stack_top (0x$V0)" \
                      || bad "vector[0]=$V0 != __stack_top=$ST"
    [ "$V1" = "$RH" ] && ok "vector[1] = Reset_Handler sembol degeri (0x$V1, Thumb bitli)" \
                      || bad "vector[1]=$V1 != Reset_Handler=$RH"

    # Ayrilmis girisler (8, 9, 10, 13) handler DEGIL: tabloda sabit 0.
    # Gercek startup tablosu ve ders-yerel l11_vectors.c tablosu birlikte.
    RSV=""
    for e in "$L11E" "$L11B/11-solution-startup.elf"; do
        for i in 8 9 10 13; do RSV="$RSV$(l11_vec "$e" $i)"; done
    done
    [ "$RSV" = "$(printf '00000000%.0s' 1 2 3 4 5 6 7 8)" ] \
        && ok "ayrilmis girisler 8,9,10,13 = 0 (gercek + ders-yerel tablo)" \
        || bad "ayrilmis vektor girisleri sifir degil: $RSV"

    # Reset_Handler'in MANTIKSAL sirasi: geri dallanan dongu(ler) ->
    # bl SystemInit -> bl main -> wfi. Komut kaliplarina degil siraya bakilir.
    RHD=$(arm-none-eabi-objdump -d --no-show-raw-insn "$L11E" | awk '/<Reset_Handler>:/{f=1;next} f&&/^$/{exit} f')
    LOOP=$(echo "$RHD" | grep -nE "\sb[a-z.]*\s+[0-9a-f]+ <Reset_Handler\+0x[0-9a-f]+>" | head -1 | cut -d: -f1)
    SI=$(echo "$RHD" | grep -nE "\sbl\s+[0-9a-f]+ <SystemInit>" | head -1 | cut -d: -f1)
    MN=$(echo "$RHD" | grep -nE "\sbl\s+[0-9a-f]+ <main>" | head -1 | cut -d: -f1)
    WF=$(echo "$RHD" | grep -nE "\swfi" | head -1 | cut -d: -f1)
    if [ -n "$LOOP" ] && [ -n "$SI" ] && [ -n "$MN" ] && [ -n "$WF" ] && \
       [ "$LOOP" -lt "$SI" ] && [ "$SI" -lt "$MN" ] && [ "$MN" -lt "$WF" ]; then
        ok "Reset_Handler sirasi: dongu -> bl SystemInit -> bl main -> wfi"
    else
        bad "Reset_Handler mantiksal sirasi beklenmedik (dongu=$LOOP SI=$SI main=$MN wfi=$WF)"
    fi

    NMI=$(arm-none-eabi-nm "$L11E" | awk '$3=="NMI_Handler"{print $1" "$2}')
    DEF=$(arm-none-eabi-nm "$L11E" | awk '$3=="Default_Handler"{print $1}')
    [ "$NMI" = "$DEF W" ] && ok "NMI_Handler: WEAK alias, adresi Default_Handler ile ayni" \
                          || bad "NMI_Handler weak alias degil: '$NMI' / Default=$DEF"

    STK=$(arm-none-eabi-nm "$L11E" | awk '$3=="SysTick_Handler"{print $1" "$2}')
    STV=$(l11_sym "$L11E" SysTick_Handler)
    if [ "${STK#* }" = "T" ] && [ "${STK% *}" != "$DEF" ] && [ "$V15" = "$STV" ]; then
        ok "SysTick_Handler: guclu tanim weak alias'in yerini aldi, vector[15]'te"
    else
        bad "SysTick_Handler override beklenmedik: nm='$STK' vector[15]=$V15 sym=$STV"
    fi

    DISC="$(awk '/^Discarded input sections/{f=1;next} /^Memory Configuration/{f=0} f' "$L11B/11-startup.map")"
    if echo "$DISC" | grep -A1 "^ \.text\.SystemInit" | grep -q "startup_mps2_an505\.c\.obj" && \
       awk '/^Linker script and memory map/{f=1} f' "$L11B/11-startup.map" | \
         grep -A1 "^ \.text\.SystemInit" | grep -q "system_init\.c\.obj"; then
        ok "weak SystemInit (startup) atildi, guclu SystemInit (system_init.c) tutuldu"
    else
        bad "SystemInit weak/strong secimi map'te beklenen sekilde degil"
    fi

    for t in 11-startup-broken 11-exercise-startup 11-solution-startup; do
        M="$L11B/$t.map"
        if [ -f "$M" ] && ! grep -q "startup_mps2_an505" "$M"; then
            ok "$t: paylasilan startup nesnesi link EDILMEDI"
        else
            bad "$t: paylasilan startup nesnesi map'te var (veya map yok)"
        fi
        NV=$(awk '/^Linker script and memory map/{f=1} f' "$M" 2>/dev/null | grep -c "^ \.vectors ")
        if [ "$NV" = "1" ] && awk '/^Linker script and memory map/{f=1} f' "$M" | \
             grep -A1 "^ \.vectors " | grep -q "l11_vectors\.c\.obj"; then
            ok "$t: tek .vectors input section'i (l11_vectors.c)"
        else
            bad "$t: .vectors input section sayisi $NV (beklenen 1, l11_vectors.c)"
        fi
    done

    MISSING=""
    for t in 11-startup 11-startup-broken 11-exercise-startup 11-solution-startup; do
        for ext in elf bin map lst; do
            [ -s "$L11B/$t.$ext" ] || MISSING="$MISSING $t.$ext"
        done
    done
    [ -z "$MISSING" ] && ok "dort hedefin .elf/.bin/.map/.lst yan urunleri mevcut" \
                      || bad "eksik yan urunler:$MISSING"
else
    echo "  [SKIP] $L11E yok; once 'cmake --build --preset $PRESET' calistir"
fi

# ===========================================================================
echo
echo "=== L11b: QEMU maskeleme deneyi (ders-yerel startup) ==="

L11X="$ROOT/lessons/11-startup/example"
L11_SRC=("$L11X/l11_vectors.c" "$L11X/l11_reset.c" "$L11X/l11_main.c" "$L11X/fixtures.c"
         "$ROOT/platform/mps2-an505/src/system_init.c"
         "$ROOT/platform/mps2-an505/src/uart_cmsdk.c"
         "$ROOT/common/src/console.c" "$ROOT/common/src/tiny_libc.c")

l11_mask_case() {  # $1 etiket, $2 derleme tanimlari, $3 beklenen DATA, $4 beklenen BSS
    local label="$1" defs="$2" edata="$3" ebss="$4"
    # shellcheck disable=SC2086
    if ! arm-none-eabi-gcc "${CFLAGS[@]}" -ffunction-sections -fdata-sections \
            -I"$L11X" $defs -nostdlib -Wl,--gc-sections -T"$LD" \
            "${L11_SRC[@]}" -lgcc -o "$WORK/l11_mask.elf" >/dev/null 2>&1; then
        bad "$label: derlenemedi"
        return
    fi
    local out
    out=$(timeout 10 qemu-system-arm -M mps2-an505 -cpu cortex-m33 \
            -nographic -monitor none -serial stdio \
            -kernel "$WORK/l11_mask.elf" 2>/dev/null | tr -d '\r')
    if echo "$out" | grep -q "===== END =====" && \
       echo "$out" | grep -q "CHECK_DATA_INIT\] $edata" && \
       echo "$out" | grep -q "CHECK_BSS_ZERO\] $ebss"; then
        ok "$label -> data $edata, bss $ebss, END"
    else
        bad "$label: beklenen data $edata / bss $ebss / END yok"
    fi
}

if command -v qemu-system-arm >/dev/null 2>&1; then
    l11_mask_case "A tam startup + zehir"                 ""                                    PASS PASS
    l11_mask_case "B .bss yok + zehir"                    "-DL11_BROKEN_SKIP_BSS"               PASS FAIL
    l11_mask_case "C .bss yok + ZEHIR YOK (QEMU gozlemi)" "-DL11_BROKEN_SKIP_BSS -DL11_NO_POISON" PASS PASS
    l11_mask_case "D .data yok + zehir"                   "-DL11_SKIP_DATA"                     FAIL PASS
else
    echo "  [SKIP] qemu-system-arm yok"
fi

# ===========================================================================
echo
echo "---------------------------------------------"
echo " PASS: $pass    FAIL: $fail"
echo "---------------------------------------------"
[ "$fail" -eq 0 ] || exit 1
echo "Build probe'lari gecti."
