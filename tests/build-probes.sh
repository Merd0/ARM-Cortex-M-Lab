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
echo "---------------------------------------------"
echo " PASS: $pass    FAIL: $fail"
echo "---------------------------------------------"
[ "$fail" -eq 0 ] || exit 1
echo "Build probe'lari gecti."
