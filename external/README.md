# external/ — vendored third-party sources

Buradaki hiçbir dosya elle değiştirilmez. Değiştirilirse "resmi kaynak"
olma özelliğini kaybeder ve güncellenemez hale gelir.

---

## CMSIS-Core (Arm)

| | |
|---|---|
| Kaynak | https://github.com/ARM-software/CMSIS_6 — `CMSIS/Core/Include` |
| Sürüm başlığı | CMSIS-Core(M) **6.3** (`__CM_CMSIS_VERSION_MAIN = 6`, `_SUB = 3`) |
| Alındığı ref | `main` (hareketli dal) |
| İndirme tarihi | 2026-09-19 |
| Bütünlük kaydı | [`CMSIS/SHA256SUMS.txt`](CMSIS/SHA256SUMS.txt) |
| Lisans | Apache-2.0 — [`CMSIS/LICENSE`](CMSIS/LICENSE) |

Bu repository'nin **production / baseline core API'si** budur. Startup,
system init, platform driver'ları, `tools/` ve projeler bunu kullanır.
Giriş noktası CMSIS device header'ıdır:
[`platform/mps2-an505/include/platform/mps2_an505.h`](../platform/mps2-an505/include/platform/mps2_an505.h).

### Provenance — ne doğrulandı, ne doğrulanamadı

Denetim, kaydın **değişmez bir yukarı akış revizyonu** içermediğini haklı
olarak işaret etti. 2026-09-20'de şunu doğruladım:

| Karşılaştırma | Sonuç |
|---|---|
| Vendored `core_cm33.h` ↔ `main` (2026-09-20) | **Birebir aynı** (sha256 `7adc886f…`, diff 0 satır) |
| Vendored `core_cm33.h` ↔ tag `v6.3.0` | **Farklı** — 37 satır; `main`'de tag sonrası eklenen `SCB_SFSR_*` tanımları var |

Yani snapshot **v6.3.0 sonrası bir `main` durumudur**, yayımlanmış bir tag'e
karşılık gelmez.

**Değişmez bir commit SHA'sı kaydedilemedi.** Dosyalar `raw.githubusercontent.com`
üzerinden dal adıyla çekildi; o isteğin hangi commit'e denk geldiği bilgisi
saklanmadı ve `main` o tarihten sonra ilerlemiş olabilir. Uydurma bir hash
yazmıyorum.

Bunun yerine elde edilebilen kanıt kaydedildi: her vendored dosyanın sha256
özeti [`CMSIS/SHA256SUMS.txt`](CMSIS/SHA256SUMS.txt) içinde. Bu, **yerel
bütünlüğü** garanti eder (dosyalar değiştirilmediyse doğrular) ama yukarı akış
kimliğini kanıtlamaz.

Doğrulamak için:

```bash
cd external/CMSIS && sha256sum -c SHA256SUMS.txt
```

**Bir sonraki güncellemede yapılacak:** bir tag'den (`v6.x.y`) veya bilinen
bir commit SHA'sından çekip o referansı buraya yazmak.

### Neden vendored (submodule değil)

- Build ağ erişimine bağımlı olmasın; `git clone` + `cmake` yeter.
- CI'da ek bir checkout adımı gerekmesin.
- Sürüm bu dosyada yazılı olarak sabit kalsın.

Maliyeti: güncelleme elle yapılır. Aşağıdaki adımlar bunun içindir.

### Kopyalanan dosyalar

Tüm CMSIS değil, yalnızca Cortex-M33 için gerekli olanlar (~490 KB):

```text
CMSIS/Core/Include/
    core_cm33.h              Cortex-M33 core peripheral access layer
    cmsis_version.h
    cmsis_compiler.h
    cmsis_gcc.h  cmsis_clang.h  cmsis_armclang.h  cmsis_iccarm.h
    m-profile/
        cmsis_gcc_m.h  cmsis_clang_m.h  cmsis_armclang_m.h  cmsis_iccarm_m.h
        armv8m_mpu.h  armv7m_mpu.h  armv7m_cachel1.h
        armv8m_pmu.h  armv81m_pac.h
CMSIS/LICENSE
```

A-profile ve R-profile dosyaları ile diğer çekirdeklerin (`core_cm0.h`,
`core_cm55.h`, …) header'ları bilerek alınmadı. M55 portu eklenirse
`core_cm55.h` de buraya eklenir.

### Güncelleme

```bash
curl -L -o /tmp/cmsis.tar.gz \
  https://codeload.github.com/ARM-software/CMSIS_6/tar.gz/refs/heads/main
tar xzf /tmp/cmsis.tar.gz -C /tmp
cp -r /tmp/CMSIS_6-main/CMSIS/Core/Include/m-profile \
      external/CMSIS/Core/Include/
cp /tmp/CMSIS_6-main/CMSIS/Core/Include/{core_cm33.h,cmsis_*.h} \
      external/CMSIS/Core/Include/
cp /tmp/CMSIS_6-main/LICENSE external/CMSIS/LICENSE
```

Sonrasında **mutlaka**:

```bash
./tests/smoke-test.sh
cd external/CMSIS && sha256sum -c SHA256SUMS.txt   # eski kayit artik gecersiz
```

ve bu dosyadaki sürüm/ref satırının + `SHA256SUMS.txt`'in güncellenmesi.
Mümkünse dal yerine **bir tag veya commit SHA'sı** kullan.

### Build entegrasyonu

[`external/CMakeLists.txt`](CMakeLists.txt) tek bir INTERFACE hedef tanımlar:

```cmake
add_library(cmsis-core INTERFACE)
target_include_directories(cmsis-core SYSTEM INTERFACE ...)
```

`SYSTEM` önemli: üçüncü taraf header'lardan gelen uyarılar repo'nun kendi
`-Wall -Wextra -Wshadow` uyarılarını boğmasın diye.

---

## Lisans notu

CMSIS Apache-2.0 ile lisanslıdır ve bu repository MIT'dir. İkisi uyumludur;
`external/CMSIS/LICENSE` olduğu gibi korunur ve bu dizindeki koda
uygulanmaya devam eder.

---

[← Ana sayfa](../README.md) · [Mimari](../docs/ARCHITECTURE.md)
