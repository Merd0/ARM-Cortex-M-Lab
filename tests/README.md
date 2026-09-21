# tests/

Bu dizin **unit test** için değil. Gömülü tarafta unit test ayrı bir konudur
(host üzerinde derleyip çalıştırmak gerekir) ve bu repoda kapsam dışıdır.

Burada üç şey var:

```bash
./tests/smoke-test.sh          # davranışsal regresyon (preset: debug)
./tests/build-probes.sh        # link/linker sözleşme probe'ları
python3 tests/check-links.py   # Markdown göreli link kontrolü
```

Üçü de CI'da çalışır (`.github/workflows/ci.yml`).

## `smoke-test.sh` — hedef sınıflandırması

Bağımsız bir denetim, eski testin ciddi bir hatayı kaçırdığını gösterdi:
`-O2`/`-Os` altında Lesson 04 PSP çözümü **CHECK_REGION FAIL** veriyordu ama
test geçiyordu, çünkü yalnızca metin varlığına bakıyordu. Artık her hedef
türüne göre beklenen **davranış** doğrulanıyor:

| Hedef türü | Beklenen |
|---|---|
| Normal örnek | Doğru sonuç + tamamlanma işareti, hiç `FAIL` yok |
| Referans çözüm | Tüm `CHECK_*` satırları `PASS` |
| Bilerek bozuk | **Beyan edilen** yanlış sonuç ya da fault; tamamlanmamalı |
| Bitmemiş ödev | Eksik davranış; çözülmüş gibi raporlanmamalı |

`===== END =====` görünmesi **tek başına** hiçbir yerde başarı sayılmaz.

Kapsam: capability-probe (hedef kimliği, MPU/SAU bölgeleri, NVIC öncelik
biti uyumu), Lesson 02 (aritmetik + bayraklar + kasıtlı fault), Lesson 03
(ABI ihlalinin beyan edilen yanlış sonucu), Lesson 04 (üç aşama, push/pop
delta'sı, stack guard, PSP çözümü).

Ölçülen son durum: **87 PASS / 0 FAIL**, üç preset'te de
(Ubuntu 24.04 container, gcc 13.2.1, QEMU 8.2.2).

## `build-probes.sh` — link sözleşmeleri

İki denetim bulgusunu kalıcı olarak yakalar:

- **I4 — libgcc sıralaması:** `-lgcc` nesne dosyalarından önce gelirse
  `__aeabi_uldivmod` çözümlenmez. Probe hem yanlış sıranın başarısız hem
  doğru sıranın başarılı olduğunu, hem de repo'nun gerçek link komutunda
  kütüphanenin nesnelerden **sonra** durduğunu kontrol eder.
- **I5 — FLASH yük görüntüsü sınırı:** `.text` + `.rodata` FLASH'a sığarken
  `.data`'nın FLASH kopyası eklenince taşan bir imaj **link hatası**
  vermeli. Ayrıca sınır içindeki bir imajın `.data`'sının çalışma zamanında
  gerçekten RAM'e ulaştığı ve `.bss`'in sıfırlandığı QEMU'da doğrulanır.

Ölçülen son durum: **10 PASS / 0 FAIL**.

## Neden bu kadar az

Bilerek. Derslerin asıl doğrulaması GDB ile **elle** yapılır — çünkü öğrenme
hedefi budur. Otomatik test, altyapının bozulmadığını garantilemek için var;
öğrenmenin yerine geçmek için değil.

Firmware'ler sonsuz döngüyle bittiği için QEMU `timeout` ile kesilir
(`QEMU_TIMEOUT`, varsayılan 8 s). Test yalnızca kendi başlattığı süreçleri
sonlandırır.

---

[← Ana sayfa](../README.md)
