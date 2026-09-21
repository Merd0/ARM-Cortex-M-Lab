# QEMU'da firmware calistirir (normal mod).
# Kullanim: .\scripts\run-qemu.ps1 build\debug\lessons\02-programmer-model\02-programmer-model.elf
# Cikis:    Ctrl-A sonra X
param(
    [Parameter(Mandatory = $true)][string]$Elf
)

if (-not (Test-Path $Elf)) {
    Write-Error "ELF bulunamadi: $Elf"
    exit 1
}

Write-Host "QEMU: mps2-an505 / cortex-m33  ->  $Elf"
Write-Host "(cikmak icin: Ctrl-A sonra X)"
Write-Host ""

& qemu-system-arm `
    -M mps2-an505 `
    -cpu cortex-m33 `
    -nographic `
    -monitor none `
    -serial stdio `
    -kernel $Elf
