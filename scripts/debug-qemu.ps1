# QEMU'yu DURDURULMUS halde baslatir ve GDB baglantisi bekler.
# Kullanim: .\scripts\debug-qemu.ps1 <firmware.elf> [-Port 3333]
# Baska terminalde:
#   arm-none-eabi-gdb -x scripts\gdb\cortex-m.gdb <firmware.elf>
#   (gdb) cm-connect
param(
    [Parameter(Mandatory = $true)][string]$Elf,
    [int]$Port = 3333
)

if (-not (Test-Path $Elf)) {
    Write-Error "ELF bulunamadi: $Elf"
    exit 1
}

Write-Host "QEMU GDB sunucusu: localhost:$Port  (islemci RESET'te durduruldu)"
Write-Host "Baglanmak icin:"
Write-Host "  arm-none-eabi-gdb -x scripts\gdb\cortex-m.gdb $Elf"
Write-Host "  (gdb) cm-connect"
Write-Host ""

& qemu-system-arm `
    -M mps2-an505 `
    -cpu cortex-m33 `
    -nographic `
    -monitor none `
    -serial stdio `
    -kernel $Elf `
    -S -gdb "tcp::$Port"
