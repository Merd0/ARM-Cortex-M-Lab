# Environment Setup

Hedef: fiziksel board olmadan Cortex-M33 firmware derleyip çalıştırıp debug
edebilmek.

---

## ⚠️ Doğrulama durumu — önce bunu oku

Bu repository'nin **tüm build / QEMU / GDB doğrulamaları Linux container
içinde yapıldı.** Somut olarak:

| Ortam | Durum |
|---|---|
| Ubuntu 24.04.4 LTS (x86_64) container | ✅ configure, build, QEMU çalıştırma, GDB breakpoint, smoke test — hepsi gerçekten çalıştırıldı |
| Windows 11 + PowerShell | ⬜ **TEST EDİLMEDİ** |
| WSL2 (Ubuntu) | ⬜ test edilmedi (Linux ile aynı olması bekleniyor) |
| macOS | ⬜ test edilmedi |

Bunun pratik sonuçları:

- `scripts/*.ps1` dosyaları yazıldı ama **Windows'ta bir kez bile
  çalıştırılmadı.** Çalışmazlarsa bu beklenen bir durumdur.
- `.vscode/tasks.json` ve `launch.json` Windows'ta denenmedi.
- Aşağıdaki Windows komutları resmi dokümantasyona ve standart paket
  isimlerine dayanır; bu makinede doğrulanmamıştır.
- `docs/TOOL_VERSIONS.md` içindeki Windows tablosu **bilerek boştur**.
  Kurulumu yapan kişi orayı kendi çıktılarıyla dolduracak.

Kısacası: Linux tarafı ölçüm, Windows tarafı talimat. İkisi karıştırılmıyor.

---

## Gereken beş şey

**Arm GNU Toolchain · CMake · Ninja · QEMU · bir editör (VS Code)**

Hepsi ücretsiz, kurulum ~15 dakika.

---

# Yol A — Windows'ta native kurulum

## A1. Arm GNU Toolchain

Resmi indirme sayfası:
<https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>

- Hedef: **`arm-none-eabi`** (AArch32 bare-metal)
- Windows için `.exe` installer en kolayı
- Kurulumda **"Add path to environment variable"** kutusunu işaretle

Alternatif olarak winget:

```powershell
winget install Arm.GnuArmEmbeddedToolchain
```

> winget paket adı Arm tarafından değiştirilebilir. Bulunamazsa
> `winget search arm-none-eabi` dene, olmadı resmi installer'ı kullan.

## A2. CMake, Ninja, Git

```powershell
winget install Kitware.CMake
winget install Ninja-build.Ninja
winget install Git.Git
```

## A3. QEMU

```powershell
winget install SoftwareFreedomConservancy.QEMU
```

winget sürümü sorun çıkarırsa resmi Windows binary'leri:
<https://www.qemu.org/download/#windows>

QEMU dizini (tipik olarak `C:\Program Files\qemu`) PATH'te değilse:

```powershell
[Environment]::SetEnvironmentVariable(
    "Path",
    [Environment]::GetEnvironmentVariable("Path", "User") + ";C:\Program Files\qemu",
    "User")
```

PATH değişikliğinden sonra **terminali kapat ve yeniden aç.**

## A4. VS Code

```powershell
winget install Microsoft.VisualStudioCode
```

Repo'yu açtığında VS Code önerilen eklentileri soracak
(`.vscode/extensions.json`): C/C++ ve CMake Tools.

## A5. Kurulumu doğrula

Yeni bir PowerShell penceresinde:

```powershell
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
cmake --version
ninja --version
qemu-system-arm --version
```

Beşi de sürüm basıyorsa hazırsın. Repo'daki script aynı şeyi yapar:

```powershell
.\scripts\check-environment.ps1
```

> Bu script Windows'ta test edilmedi. Hata verirse yukarıdaki beş komutu elle
> çalıştır — asıl kontrol odur.

## A6. Build ve çalıştır

```powershell
git clone <repo-url>
cd ARM-Cortex-M-Lab

cmake --preset debug
cmake --build --preset debug
```

Beklenen: her firmware için `--print-memory-usage` çıktısı ve `size` tablosu.

```powershell
cmake --build --preset debug --target run-02-programmer-model
```

QEMU'dan çıkış: **Ctrl-A**, sonra **X**.

> Ctrl-A bazı terminallerde yutulur. Windows Terminal kullanıyorsan çalışır.

## A7. Debug

İki terminal gerekir.

Terminal 1 — QEMU durdurulmuş halde GDB bekler:

```powershell
cmake --build --preset debug --target debug-02-programmer-model
```

Terminal 2 — GDB bağlanır:

```powershell
arm-none-eabi-gdb build\debug\lessons\02-programmer-model\02-programmer-model.elf
```

GDB içinde:

```gdb
set architecture armv8-m.main
target remote :3333
source scripts/gdb/cortex-m.gdb
cm-mode
b main
continue
```

VS Code'dan: **F5** → "Attach to QEMU (lesson 02)". Bu yol da Windows'ta
denenmedi; çalışmazsa yukarıdaki iki terminal yöntemi her zaman çalışır —
ve öğretici olan da zaten odur.

---

# Yol B — WSL2 (doğrulanmış ortama en yakın)

Windows tarafında bir şey çalışmazsa, doğrulamanın yapıldığı ortamın aynısını
kurmanın yolu budur.

```powershell
wsl --install -d Ubuntu-24.04
```

WSL içinde:

```bash
sudo apt-get update
sudo apt-get install -y \
    gcc-arm-none-eabi binutils-arm-none-eabi \
    qemu-system-arm cmake ninja-build gdb-multiarch git
```

Doğrula ve çalıştır:

```bash
./scripts/check-environment.sh
cmake --preset debug
cmake --build --preset debug
./tests/smoke-test.sh
```

Bu komut dizisi **bu container'da gerçekten çalıştırıldı: 26 PASS / 0 FAIL.**

## WSL notları

- `arm-none-eabi-gdb` Ubuntu depolarında yok; `gdb-multiarch` kullanılır.
  Bu repo için farkı yoktur — tüm GDB doğrulamaları onunla yapıldı:
  ```bash
  gdb-multiarch -q build/debug/lessons/02-programmer-model/02-programmer-model.elf
  (gdb) set architecture armv8-m.main
  (gdb) target remote :3333
  ```
- Kod Windows diskinde (`/mnt/c/...`) değil, WSL dosya sisteminde (`~/`)
  dursun. `/mnt/c` üzerindeki build belirgin şekilde yavaştır.
- VS Code + "WSL" eklentisi ile WSL klasörünü doğrudan açabilirsin:
  Windows'ta editör, Linux'ta toolchain.

---

## Sorun giderme

| Belirti | Olası neden | Çözüm |
|---|---|---|
| `arm-none-eabi-gcc: command not found` | PATH'e eklenmemiş | Terminali yeniden aç; olmazsa kurulum `bin` dizinini PATH'e ekle |
| `CMAKE_C_COMPILER not found` | Toolchain PATH'te değil | `cmake -DARM_TOOLCHAIN_BIN=C:/path/to/bin --preset debug` |
| `Ninja generator not found` | Ninja kurulu değil | `winget install Ninja-build.Ninja` |
| `unsupported machine type` | QEMU çok eski | `qemu-system-arm -M help \| findstr mps2` → `mps2-an505` listede olmalı |
| QEMU açılıyor, çıktı yok | `-serial stdio` eksik | Repo'nun `run-*` hedeflerini kullan |
| GDB `Remote connection closed` | QEMU başlamadan bağlanıldı | Terminal 1'in beklediğinden emin ol |
| GDB'de register'lar saçma | Mimari ayarlanmamış | `set architecture armv8-m.main` |
| `.ps1 cannot be loaded` | Execution policy | `powershell -ExecutionPolicy Bypass -File scripts\check-environment.ps1` |

---

## Kurulumu bitirince

`docs/TOOL_VERSIONS.md` içindeki Windows tablosunu kendi çıktılarınla doldur.
Bu repo'nun kuralı: **ölçmediğin şeyi yazma.** O tablo şu an boş, çünkü henüz
kimse Windows'ta ölçmedi.

---

[← Ana sayfa](../README.md) · [Araç sürümleri](TOOL_VERSIONS.md) · [Bilinen sınırlar](LIMITATIONS.md)
