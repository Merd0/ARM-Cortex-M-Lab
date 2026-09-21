#!/usr/bin/env bash
# QEMU'da firmware calistirir (normal mod).
# Kullanim: ./scripts/run-qemu.sh build/debug/lessons/02-programmer-model/02-programmer-model.elf
# Cikis:    Ctrl-A sonra X
set -e
ELF="${1:?kullanim: run-qemu.sh <firmware.elf>}"
[ -f "$ELF" ] || { echo "ELF bulunamadi: $ELF"; exit 1; }
echo "QEMU: mps2-an505 / cortex-m33  ->  $ELF"
echo "(cikmak icin: Ctrl-A sonra X)"
echo
exec qemu-system-arm \
    -M mps2-an505 \
    -cpu cortex-m33 \
    -nographic \
    -monitor none \
    -serial stdio \
    -kernel "$ELF"
