#!/usr/bin/env bash
# QEMU'yu DURDURULMUS halde baslatir ve GDB baglantisi bekler (port 3333).
# Kullanim: ./scripts/debug-qemu.sh <firmware.elf> [port]
# Baska terminalde:
#   arm-none-eabi-gdb -x scripts/gdb/cortex-m.gdb <firmware.elf>
#   (gdb) cm-connect
set -e
ELF="${1:?kullanim: debug-qemu.sh <firmware.elf> [port]}"
PORT="${2:-3333}"
[ -f "$ELF" ] || { echo "ELF bulunamadi: $ELF"; exit 1; }
echo "QEMU GDB sunucusu: localhost:$PORT  (islemci RESET'te durduruldu)"
echo "Baglanmak icin:"
echo "  arm-none-eabi-gdb -x scripts/gdb/cortex-m.gdb $ELF"
echo "  (gdb) cm-connect"
echo
exec qemu-system-arm \
    -M mps2-an505 \
    -cpu cortex-m33 \
    -nographic \
    -monitor none \
    -serial stdio \
    -kernel "$ELF" \
    -S -gdb "tcp::$PORT"
