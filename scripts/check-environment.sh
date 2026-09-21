#!/usr/bin/env bash
# Gerekli araclarin varligini VE versiyonunu kontrol eder.
# Kullanim: ./scripts/check-environment.sh

missing=0

check() {
    local name="$1"; shift
    local version_cmd="$*"
    if command -v "$name" >/dev/null 2>&1; then
        local ver
        ver=$($version_cmd 2>&1 | head -1)
        printf "  [ OK ] %-24s %s\n" "$name" "$ver"
    else
        printf "  [MISS] %-24s bulunamadi\n" "$name"
        missing=$((missing + 1))
    fi
}

echo "=============================================="
echo " ARM Cortex-M Lab - Environment Check"
echo "=============================================="
echo
echo "Arm GNU Toolchain:"
check arm-none-eabi-gcc     arm-none-eabi-gcc --version
check arm-none-eabi-g++     arm-none-eabi-g++ --version
check arm-none-eabi-as      arm-none-eabi-as --version
check arm-none-eabi-ld      arm-none-eabi-ld --version
check arm-none-eabi-objdump arm-none-eabi-objdump --version
check arm-none-eabi-objcopy arm-none-eabi-objcopy --version
check arm-none-eabi-size    arm-none-eabi-size --version
check arm-none-eabi-nm      arm-none-eabi-nm --version

echo
echo "Debugger (birisi yeterli):"
if command -v arm-none-eabi-gdb >/dev/null 2>&1; then
    check arm-none-eabi-gdb arm-none-eabi-gdb --version
elif command -v gdb-multiarch >/dev/null 2>&1; then
    check gdb-multiarch gdb-multiarch --version
    echo "         (arm-none-eabi-gdb yok; gdb-multiarch kullanilacak)"
else
    echo "  [MISS] arm-none-eabi-gdb / gdb-multiarch   ikisi de yok"
    missing=$((missing + 1))
fi

echo
echo "Build sistemi:"
check cmake cmake --version
check ninja ninja --version

echo
echo "Simulator:"
check qemu-system-arm qemu-system-arm --version

echo
if command -v qemu-system-arm >/dev/null 2>&1; then
    if qemu-system-arm -machine help 2>/dev/null | grep -q "mps2-an505"; then
        echo "  [ OK ] QEMU mps2-an505 (Cortex-M33) makinesi mevcut"
    else
        echo "  [MISS] QEMU'da mps2-an505 makinesi YOK - QEMU surumunu kontrol et"
        missing=$((missing + 1))
    fi
fi

echo
echo "=============================================="
if [ "$missing" -eq 0 ]; then
    echo " Her sey hazir. Sonraki adim:"
    echo "   cmake --preset debug && cmake --build --preset debug"
    exit 0
else
    echo " Eksik arac sayisi: $missing"
    echo " Kurulum adimlari: docs/ENVIRONMENT.md"
    exit 1
fi
