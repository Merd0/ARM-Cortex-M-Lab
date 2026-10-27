**Dil:** [English](TOOL_VERSIONS.md) | Türkçe

# Tool Versions

Neden onemli: derleyici surumu degisince uretilen assembly de degisir.
Bir derste "su komutu goreceksin" diyorsak, hangi surumle dogruladigimiz
yazili olmali.

## Bootstrap dogrulamasinin yapildigi ortam

> **Dikkat:** Asagidaki dogrulama **Linux (Ubuntu 24.04 x86_64)** uzerinde
> yapildi. Windows uzerinde ayni surumler kullanilsa bile derleyici
> ciktisinda kucuk farklar gorulebilir. Windows'ta kendi surumlerini
> asagidaki tabloya ekle.

| Arac | Surum | Kaynak |
|---|---|---|
| arm-none-eabi-gcc | 13.2.1 20231009 (paket 15:13.2.rel1-2) | Ubuntu 24.04 `gcc-arm-none-eabi` |
| arm-none-eabi-binutils (as/ld/objdump) | 2.42 | Ubuntu 24.04 |
| qemu-system-arm | 8.2.2 (Debian 1:8.2.2+ds-0ubuntu1.18) | Ubuntu 24.04 |
| cmake | 3.28.3 | Ubuntu 24.04 |
| ninja | 1.11.1 | Ubuntu 24.04 |
| gdb-multiarch | 15.1 | Ubuntu 24.04 |
| OS | Ubuntu 24.04.4 LTS, x86_64 | - |

`arm-none-eabi-gdb` bu ortamda paketle gelmedi; `gdb-multiarch` kullanildi.
Ikisi de Cortex-M remote debug icin yeterlidir.

## Kendi ortamini kaydet

```bash
./scripts/check-environment.sh | tee docs/tool-versions-local.txt
```

```powershell
.\scripts\check-environment.ps1 | Tee-Object docs\tool-versions-local.txt
```

## Windows yerel denetim ortamı — 2026-10-27

| Arac | Surum | Tarih |
|---|---|---|
| arm-none-eabi-gcc | xPack 15.2.1 20251203 | 2026-10-27 |
| qemu-system-arm | 11.1.0 (e470268ff4) | 2026-10-27 |
| cmake | 4.3.2 | 2026-10-27 |
| ninja | 1.13.2 | 2026-10-27 |
| arm-none-eabi-gdb | 16.3.90.20250906-git | 2026-10-27 |

## Vendored kutuphane surumleri

| Bilesen | Surum | Kaynak | Lisans |
|---|---|---|---|
| CMSIS-Core(M) | 6.3 | ARM-software/CMSIS_6, `main`, 2026-09-19 | Apache-2.0 |

Ayrinti ve guncelleme adimlari: [`external/README.md`](../external/README.tr.md).
