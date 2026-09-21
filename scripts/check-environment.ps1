# Gerekli araclarin varligini VE versiyonunu kontrol eder (Windows / PowerShell).
# Kullanim: .\scripts\check-environment.ps1
#
# NOT: Bu script Windows uzerinde test edilmelidir. Sorun gorursen
# ayni kontrolleri docs/ENVIRONMENT.md icindeki komutlarla elle yapabilirsin.

$missing = 0

function Check-Tool {
    param([string]$Name, [string]$VersionArg = "--version")
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($null -ne $cmd) {
        $ver = (& $Name $VersionArg 2>&1 | Select-Object -First 1)
        "  [ OK ] {0,-24} {1}" -f $Name, $ver | Write-Host
        return $true
    }
    else {
        "  [MISS] {0,-24} bulunamadi" -f $Name | Write-Host -ForegroundColor Yellow
        $script:missing++
        return $false
    }
}

Write-Host "=============================================="
Write-Host " ARM Cortex-M Lab - Environment Check"
Write-Host "=============================================="
Write-Host ""
Write-Host "Arm GNU Toolchain:"
Check-Tool "arm-none-eabi-gcc"     | Out-Null
Check-Tool "arm-none-eabi-g++"     | Out-Null
Check-Tool "arm-none-eabi-as"      | Out-Null
Check-Tool "arm-none-eabi-ld"      | Out-Null
Check-Tool "arm-none-eabi-objdump" | Out-Null
Check-Tool "arm-none-eabi-objcopy" | Out-Null
Check-Tool "arm-none-eabi-size"    | Out-Null
Check-Tool "arm-none-eabi-nm"      | Out-Null

Write-Host ""
Write-Host "Debugger:"
if (Get-Command "arm-none-eabi-gdb" -ErrorAction SilentlyContinue) {
    Check-Tool "arm-none-eabi-gdb" | Out-Null
}
elseif (Get-Command "gdb-multiarch" -ErrorAction SilentlyContinue) {
    Check-Tool "gdb-multiarch" | Out-Null
    Write-Host "         (arm-none-eabi-gdb yok; gdb-multiarch kullanilacak)"
}
else {
    Write-Host "  [MISS] arm-none-eabi-gdb / gdb-multiarch   ikisi de yok" -ForegroundColor Yellow
    $missing++
}

Write-Host ""
Write-Host "Build sistemi:"
Check-Tool "cmake" | Out-Null
Check-Tool "ninja" | Out-Null

Write-Host ""
Write-Host "Simulator:"
if (Check-Tool "qemu-system-arm") {
    $machines = & qemu-system-arm -machine help 2>&1 | Out-String
    if ($machines -match "mps2-an505") {
        Write-Host "  [ OK ] QEMU mps2-an505 (Cortex-M33) makinesi mevcut"
    }
    else {
        Write-Host "  [MISS] QEMU'da mps2-an505 makinesi YOK" -ForegroundColor Yellow
        $missing++
    }
}

Write-Host ""
Write-Host "=============================================="
if ($missing -eq 0) {
    Write-Host " Her sey hazir. Sonraki adim:" -ForegroundColor Green
    Write-Host "   cmake --preset debug"
    Write-Host "   cmake --build --preset debug"
    exit 0
}
else {
    Write-Host " Eksik arac sayisi: $missing" -ForegroundColor Yellow
    Write-Host " Kurulum adimlari: docs/ENVIRONMENT.md"
    exit 1
}
