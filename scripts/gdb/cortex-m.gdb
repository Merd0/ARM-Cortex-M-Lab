# ---------------------------------------------------------------------------
# Cortex-M GDB yardimcilari  (Armv8-M Main Extension / Cortex-M33)
#
# Kullanim:
#   gdb-multiarch -q build/debug/.../firmware.elf
#   (gdb) set architecture armv8-m.main
#   (gdb) target remote :3333
#   (gdb) source scripts/gdb/cortex-m.gdb
#
# Burada sihir yok: her komut altta hangi register'i okudugunu gosterir.
# Register adresleri: Armv8-M Architecture Reference Manual (Arm DDI 0553),
# Cortex-M33 Devices Generic User Guide (Arm 100235).
#
# TASARIM NOTU (onarim sonrasi)
#   cm-frame artik aktif $sp'den TAHMIN ETMEZ. Bir bagimsiz denetim, eski
#   surumun handler kendi stack'ini kullanirken alakasiz bellegi "exception
#   frame" diye cozdugunu ve stacking'in basarisiz oldugu durumlarda PC=0
#   gibi uydurma degerler bastigini gosterdi. Artik frame adresi ACIKCA
#   verilir ve on kosullar denetlenir.
# ---------------------------------------------------------------------------

set confirm off
set pagination off
set print pretty on

# NOT: "set arm force-mode thumb" YALNIZCA disassembly modunu zorlar;
# mimari secimi DEGILDIR ve ozel register'larin ($msp/$psp/$control/$psplim)
# gorunmesini saglamaz. Dogru yol baglanmadan once:
#     set architecture armv8-m.main
# Bu script onu senin yerine yapmaz, cunku hedef mimarisini secmek
# oturumun kararidir; ama eksikse asagidaki komutlar seni uyarir.

# --- RAM penceresi: frame adresi makullugu icin (bu platform) --------------
# MPS2-AN505 Secure internal SRAM: 0x30000000 .. 0x30008000
set $cm_ram_start = 0x30000000
set $cm_ram_end   = 0x30008000
# Yigilmis PC'nin makullugu icin kod penceresi (Secure code SRAM, 512K).
set $cm_code_start = 0x10000000
set $cm_code_end   = 0x10080000

define cm-connect
  target remote :3333
  printf "Baglandi. Once 'set architecture armv8-m.main' yaptigindan emin ol.\n"
  printf "Simdi: cm-regs / cm-mode / cm-stack / cm-fault\n"
end
document cm-connect
QEMU'nun GDB sunucusuna baglanir (localhost:3333).
end

# ---------------------------------------------------------------------------
define cm-regs
  printf "--- core registers ---\n"
  info registers r0 r1 r2 r3 r4 r5 r6 r7
  info registers r8 r9 r10 r11 r12
  info registers sp lr pc xpsr
  printf "--- special registers ---\n"
  printf "MSP     = 0x%08x\n", $msp
  printf "PSP     = 0x%08x\n", $psp
  printf "CONTROL = 0x%08x   (bit0 nPRIV, bit1 SPSEL)\n", $control
  printf "IPSR    = %d        (0 = Thread Mode, >0 = exception numarasi)\n", ($xpsr & 0x1ff)
end
document cm-regs
R0-R12, SP/LR/PC/xPSR ve ozel register'lari tek ekranda gosterir.
$msp/$psp/$control gorunmuyorsa 'set architecture armv8-m.main' eksiktir.
end

# ---------------------------------------------------------------------------
define cm-mode
  set $cm_ipsr = $xpsr & 0x1ff
  if $cm_ipsr == 0
    printf "Mode      : Thread Mode\n"
    if $control & 1
      printf "Privilege : Unprivileged (CONTROL.nPRIV = 1)\n"
    else
      printf "Privilege : Privileged (CONTROL.nPRIV = 0)\n"
    end
    if $control & 2
      printf "Stack     : PSP (0x%08x)\n", $psp
    else
      printf "Stack     : MSP (0x%08x)\n", $msp
    end
  else
    printf "Mode      : Handler Mode (exception %d)\n", $cm_ipsr
    printf "Privilege : Privileged (Handler Mode her zaman ayricaliklidir)\n"
    if $control & 1
      printf "            CONTROL.nPRIV = 1 -> KESILEN Thread baglami ayricaliksizdi\n"
    end
    printf "Stack     : MSP (0x%08x)  (Handler Mode MSP kullanir)\n", $msp
    printf "            PSP = 0x%08x   (kesilen Thread baglaminin stack'i olabilir)\n", $psp
  end
end
document cm-mode
Mode / privilege / aktif stack durumunu ozetler.
Handler Mode her zaman ayricaliklidir; CONTROL.nPRIV Thread Mode'un
ayricaligini tanimlar, mevcut handler'inkini degil.
Guvenlik durumu (Secure/Non-secure) burada gosterilmez; bu script tek
guvenlik durumu varsayar (bkz. ders 16).
end

# ---------------------------------------------------------------------------
define cm-stack
  printf "--- aktif SP: 0x%08x ---\n", $sp
  x/16wx $sp
end
document cm-stack
Aktif stack pointer'in gosterdigi yerden 16 word gosterir.
Bu bir exception frame COZUMU DEGILDIR; ham bellek dokumudur.
end

# ---------------------------------------------------------------------------
define cm-exc-return
  if $argc != 1
    printf "Kullanim: cm-exc-return <EXC_RETURN degeri>\n"
    printf "Ornek   : cm-exc-return 0xFFFFFFFD\n"
    printf "Not: handler'a girer girmez $lr EXC_RETURN'dur; ama herhangi bir\n"
    printf "     C cagrisindan SONRA $lr'nin hala onu tasidigi VARSAYILAMAZ.\n"
  else
    set $cm_er = (unsigned int)$arg0
    if ($cm_er & 0xFFFFFF80) != 0xFFFFFF80
      printf "0x%08x gecerli bir EXC_RETURN DEGIL: [31:7] RES1'dir, hepsi 1 olmali.\n", $cm_er
    else
      if ($cm_er & 0x2) != 0
        printf "UYARI: bit1 = 1. EXC_RETURN[1] ayrilmistir ve sifir olmali -> BICIMSIZ.\n"
      end
      if ((($cm_er >> 3) & 1) == 0) && ((($cm_er >> 2) & 1) == 1)
        printf "UYARI: Mode=0 + SPSEL=1. Handler Mode Main stack kullanir;\n"
        printf "       bu kombinasyon gecerli bir kodlama DEGILDIR.\n"
      end
      printf "EXC_RETURN = 0x%08x\n", $cm_er
      if $cm_er & 0x8
        printf "  bit3 Mode     = 1 -> Thread Mode'a donus\n"
      else
        printf "  bit3 Mode     = 0 -> Handler Mode'a donus\n"
      end
      if $cm_er & 0x4
        printf "  bit2 SPSEL    = 1 -> frame PSP uzerinde\n"
      else
        printf "  bit2 SPSEL    = 0 -> frame MSP uzerinde\n"
      end
      if $cm_er & 0x10
        printf "  bit4 FType    = 1 -> standart (8 word) frame\n"
      else
        printf "  bit4 FType    = 0 -> genisletilmis (FP) frame - cm-frame DESTEKLEMIYOR\n"
      end
      if ($cm_er & 0x40) == 0
        printf "  bit6 S        = 0 -> Non-secure kaynak (bu script tek durum varsayar)\n"
      end
    end
  end
end
document cm-exc-return
Bir EXC_RETURN degerini cozer. Degeri SEN verirsin; script canli $lr'den
tahmin etmez, cunku handler icinde yapilan her cagri $lr'yi ezebilir.
end

# ---------------------------------------------------------------------------
define cm-frame
  if $argc != 2
    printf "Kullanim: cm-frame <frame_adresi> <EXC_RETURN>\n"
    printf "\n"
    printf "Ornek: cm-frame $psp 0xFFFFFFFD\n"
    printf "\n"
    printf "DESTEKLENEN EXC_RETURN KODLAMALARI (standart 8 word frame):\n"
    printf "  Secure     (ES=1, S=1):\n"
    printf "    0xFFFFFFF1  Handler Mode'a donus, frame MSP'de\n"
    printf "    0xFFFFFFF9  Thread  Mode'a donus, frame MSP'de\n"
    printf "    0xFFFFFFFD  Thread  Mode'a donus, frame PSP'de\n"
    printf "  Non-secure (ES=0, S=0):\n"
    printf "    0xFFFFFFB0  Handler Mode'a donus, frame MSP'de\n"
    printf "    0xFFFFFFB8  Thread  Mode'a donus, frame MSP'de\n"
    printf "    0xFFFFFFBC  Thread  Mode'a donus, frame PSP'de\n"
    printf "  DIKKAT: 0xFFFFFFB1 / B9 / BD DESTEKLENMEZ. Onlarda ES=1, S=0'dir\n"
    printf "  (Secure exception, Non-secure frame) - guvenlik durumlari arasi\n"
    printf "  bir gecis. Mimari olarak gecerli, bu komut cozmuyor.\n"
    printf "  Bu komut mimari olarak gecerli EXC_RETURN bicimlerinin yalnizca\n"
    printf "  bir ALT KUMESINI destekler: FType=1, DCRS=1, ES=S.\n"
    printf "  Bunlarin disindaki her deger ya BICIMSIZ ya DESTEKLENMIYOR.\n"
    printf "  Sadece 0xFFFFFFxx onekine bakmak YETMEZ: 0xFFFFFFFF (bit1=1),\n"
    printf "  0xFFFFFF7D (bit7=0) ve 0xFFFFFFF5 (Handler+PSP) GECERSIZDIR.\n"
    printf "\n"
    printf "NE YAPAR / NE YAPMAZ\n"
    printf "  Bu komut SINIRLI, AKLI BASINDA (sanity-checked) bir frame\n"
    printf "  cozucudur. Verilen adresin gercekten gecmiste yazilmis bir\n"
    printf "  exception frame'i oldugunu KANITLAYAMAZ - hicbir debugger\n"
    printf "  bunu yalnizca bir adresten kanitlayamaz. Yaptigi sey:\n"
    printf "  desteklenmeyen frame turlerini ve bariz sacmaliklari\n"
    printf "  REDDETMEK. Kontrollerden gecmesi 'gercektir' demek degildir.\n"
    printf "\n"
    printf "NEDEN IKI ARGUMAN?\n"
    printf "  1) Frame adresi: handler MSP uzerinde calisir, kesilen baglam\n"
    printf "     PSP'ye yigmis olabilir; ustelik handler'in kendi prolog'u\n"
    printf "     MSP'yi frame'in basindan uzaklastirir. Yani aktif $sp genel\n"
    printf "     olarak frame adresi DEGILDIR.\n"
    printf "  2) EXC_RETURN: frame TURUNU (standart / FP-genisletilmis,\n"
    printf "     hangi stack, hangi guvenlik durumu) yalnizca o soyler.\n"
    printf "\n"
    printf "Adresi bulmak icin:\n"
    printf "  handler'a GIRER GIRMEZ $lr'yi kaydet (EXC_RETURN).\n"
    printf "  cm-exc-return <lr> ile bit2'ye bak:\n"
    printf "    bit2=1 -> frame adresi giristeki PSP\n"
    printf "    bit2=0 -> frame adresi giristeki MSP\n"
    printf "  Handler icinde C cagrisi yapildiysa CANLI $lr artik\n"
    printf "  EXC_RETURN olmayabilir; kaydettigin degeri kullan.\n"
  else
    set $cm_fa = (unsigned int)$arg0
    set $cm_er = (unsigned int)$arg1
    set $cm_cfsr = *(unsigned int*)0xE000ED28
    set $cm_ok = 1

    # --- 1) EXC_RETURN BICIMI (mimari kodlama) ----------------------------
    # Kaynak: Armv8-M Exception Model User Guide (Arm 107706), EXC_RETURN
    # bit alanlari tablosu:
    #   [31:7] RES1  - HEPSI BIR olmali (0xFF onekiyle birlikte)
    #   [6]    S     - frame Secure stack'te mi
    #   [5]    DCRS  - varsayilan callee register yigmasi (1 = varsayilan)
    #   [4]    FType - 1 = standart frame, 0 = genisletilmis (FP) frame
    #   [3]    Mode  - 0 = Handler Mode'a donus, 1 = Thread Mode'a donus
    #   [2]    SPSEL - 0 = frame MSP'de, 1 = frame PSP'de
    #   [1]    Res   - SIFIR olmali
    #   [0]    ES    - exception Secure durumdan mi alindi
    #
    # "0xFFFFFFxx ise EXC_RETURN'dur" YANLISTIR: yalnizca oneke bakmak
    # 0xFFFFFFFF (bit1 = 1) veya 0xFFFFFF7D (bit7 = 0) gibi mimari olarak
    # gecersiz degerleri kabul eder.

    if ($cm_er & 0xFFFFFF80) != 0xFFFFFF80
      printf "REDDEDILDI [BICIMSIZ EXC_RETURN]: 0x%08x\n", $cm_er
      printf "  EXC_RETURN[31:7] RES1'dir: bu bitlerin HEPSI 1 olmali\n"
      printf "  (yani (deger & 0xFFFFFF80) == 0xFFFFFF80). Sizin degerinizde degil.\n"
      set $cm_ok = 0
    end
    if $cm_ok
      if ($cm_er & 0x2) != 0
        printf "REDDEDILDI [BICIMSIZ EXC_RETURN]: 0x%08x -> bit1 = 1.\n", $cm_er
        printf "  EXC_RETURN[1] ayrilmistir ve SIFIR olmak zorundadir.\n"
        set $cm_ok = 0
      end
    end
    if $cm_ok
      set $cm_mode  = ($cm_er >> 3) & 1
      set $cm_spsel = ($cm_er >> 2) & 1
      if ($cm_mode == 0) && ($cm_spsel == 1)
        printf "REDDEDILDI [BICIMSIZ EXC_RETURN]: 0x%08x -> Mode=0, SPSEL=1.\n", $cm_er
        printf "  Mode=0 Handler Mode'a donus demektir; Handler Mode her zaman\n"
        printf "  Main stack'i kullanir. Bu yuzden Handler + PSP diye bir gecerli\n"
        printf "  kodlama YOKTUR (Armv7-M tablosunda da 0xFFFFFFF5 bulunmaz).\n"
        printf "  NOT: bu deger Thread Mode'a donus DEGILDIR - oyle raporlanamaz.\n"
        set $cm_ok = 0
      end
    end

    # --- 2) MIMARI OLARAK GECERLI AMA BU KOMUTUN DESTEKLEMEDIGI BICIMLER ---
    if $cm_ok
      if ($cm_er & 0x10) == 0
        printf "REDDEDILDI [DESTEKLENMIYOR]: 0x%08x -> FType=0.\n", $cm_er
        printf "  Mimari olarak GECERLI bir deger, ama GENISLETILMIS (FP) frame'i\n"
        printf "  tanimliyor: 8 word degil, FP register'lari ve FPSCR de yiginda.\n"
        printf "  Bu komut yalnizca STANDART 8 word frame'i cozer.\n"
        printf "  (FPU ders 13'te acilacak; o zamana kadar bu yol kapali.)\n"
        set $cm_ok = 0
      end
    end
    if $cm_ok
      if ($cm_er & 0x20) == 0
        printf "REDDEDILDI [DESTEKLENMIYOR]: 0x%08x -> DCRS=0.\n", $cm_er
        printf "  Mimari olarak GECERLI, ama ek durum baglami var (callee\n"
        printf "  register'lari da yigilmis). Frame yerlesimi farklidir.\n"
        set $cm_ok = 0
      end
    end
    if $cm_ok
      set $cm_es = $cm_er & 1
      set $cm_s  = ($cm_er >> 6) & 1
      if $cm_es != $cm_s
        printf "REDDEDILDI [DESTEKLENMIYOR]: 0x%08x -> ES=%d, S=%d.\n", $cm_er, $cm_es, $cm_s
        printf "  Mimari olarak GECERLI, ama guvenlik durumlari arasi bir gecisi\n"
        printf "  tanimliyor. Bu script tek guvenlik durumu varsayar (ders 16).\n"
        set $cm_ok = 0
      end
    end

    # --- 3) stacking hatasi ------------------------------------------------
    if $cm_ok
      if ($cm_cfsr & 0x00100000) || ($cm_cfsr & 0x00001000) || ($cm_cfsr & 0x00000010)
        printf "REDDEDILDI [STACKING HATASI]: CFSR = 0x%08x\n", $cm_cfsr
        printf "  (UFSR.STKOF / BFSR.STKERR / MMFSR.MSTKERR)\n"
        printf "  Yigma isleminin KENDISI basarisiz oldugunda bellekte gecerli bir\n"
        printf "  frame olmayabilir. Buradan okunacak degerler uydurma olur.\n"
        printf "  NOT: bu bitler yapiskandir (sticky) - eski bir hatadan kalmis\n"
        printf "  olabilirler. Emin isen once CFSR'i temizleyip tekrar dene.\n"
        set $cm_ok = 0
      end
    end

    # --- 4) adres bicimi ---------------------------------------------------
    if $cm_ok
      if ($cm_fa & 0x7) != 0
        printf "REDDEDILDI [GECERSIZ FRAME ADRESI]: 0x%08x 8-byte hizali degil.\n", $cm_fa
        printf "  Cortex-M33 CCR[9] RES1: frame adresi 8'in kati olmali.\n"
        set $cm_ok = 0
      end
    end
    if $cm_ok
      if ($cm_fa < $cm_ram_start) || (($cm_fa + 32) > $cm_ram_end)
        printf "REDDEDILDI [GECERSIZ FRAME ADRESI]: 0x%08x RAM penceresi disinda\n", $cm_fa
        printf "  (0x%08x .. 0x%08x). Baska bir platformda calisiyorsan\n", $cm_ram_start, $cm_ram_end
        printf "  $cm_ram_start / $cm_ram_end degiskenlerini ayarla.\n"
        set $cm_ok = 0
      end
    end

    # --- 5) icerik makullugu (sanity) --------------------------------------
    # Bunlar KANIT DEGIL, elemedir: bariz sacmaliklari (ornegin sifirla dolu
    # bellek) frame diye sunmayi engeller.
    if $cm_ok
      set $cm_pc   = *(unsigned int*)($cm_fa + 24)
      set $cm_xpsr = *(unsigned int*)($cm_fa + 28)

      if ($cm_xpsr & 0x01000000) == 0
        printf "REDDEDILDI [GECERSIZ FRAME ICERIGI]: yigilmis xPSR = 0x%08x, T biti (bit24) SIFIR.\n", $cm_xpsr
        printf "  Cortex-M her zaman Thumb durumundadir; gecerli bir frame'de\n"
        printf "  bu bit 1 olmak zorundadir. Burasi buyuk olasilikla bir\n"
        printf "  exception frame'i degil (ornegin sifirla dolu bellek).\n"
        set $cm_ok = 0
      end
    end
    if $cm_ok
      if $cm_pc == 0
        printf "REDDEDILDI [GECERSIZ FRAME ICERIGI]: yigilmis PC = 0, gecerli donus adresi degil.\n"
        set $cm_ok = 0
      end
    end
    if $cm_ok
      if ($cm_pc & 1) != 0
        printf "REDDEDILDI [GECERSIZ FRAME ICERIGI]: yigilmis PC = 0x%08x, tek sayi.\n", $cm_pc
        printf "  Donanim yigilmis PC'nin bit0'ini temizler; tek olmamali.\n"
        set $cm_ok = 0
      end
    end
    if $cm_ok
      if ($cm_pc < $cm_code_start) || ($cm_pc >= $cm_code_end)
        printf "REDDEDILDI [GECERSIZ FRAME ICERIGI]: yigilmis PC = 0x%08x kod penceresi disinda\n", $cm_pc
        printf "  (0x%08x .. 0x%08x). Verdigin adres buyuk olasilikla bir\n", $cm_code_start, $cm_code_end
        printf "  exception frame'i degil (ornegin handler'in kendi yerel verisi).\n"
        printf "  Baska bir platformdaysan $cm_code_start / $cm_code_end ayarla.\n"
        set $cm_ok = 0
      end
    end

    if $cm_ok
      if (($cm_xpsr & 0x200) != 0) && (($cm_fa + 36) > $cm_ram_end)
        printf "REDDEDILDI [GECERSIZ FRAME ICERIGI]: alignment padding RAM penceresi disinda.\n"
        set $cm_ok = 0
      end
    end
    if $cm_ok
      if ((($cm_er & 8) != 0) && (($cm_xpsr & 0x1ff) != 0)) || ((($cm_er & 8) == 0) && (($cm_xpsr & 0x1ff) == 0))
        printf "REDDEDILDI [GECERSIZ FRAME ICERIGI]: EXC_RETURN Mode ve stacked IPSR uyusmuyor.\n"
        set $cm_ok = 0
      end
    end

    # --- 6) coz -------------------------------------------------------------
    if $cm_ok
      printf "--- exception frame @ 0x%08x (standart 8 word) ---\n", $cm_fa
      if $cm_er & 0x4
        printf "EXC_RETURN 0x%08x -> frame PSP uzerinde, Thread Mode'a donus\n", $cm_er
      else
        printf "EXC_RETURN 0x%08x -> frame MSP uzerinde\n", $cm_er
      end
      printf "R0         = 0x%08x\n", *(unsigned int*)($cm_fa + 0)
      printf "R1         = 0x%08x\n", *(unsigned int*)($cm_fa + 4)
      printf "R2         = 0x%08x\n", *(unsigned int*)($cm_fa + 8)
      printf "R3         = 0x%08x\n", *(unsigned int*)($cm_fa + 12)
      printf "R12        = 0x%08x\n", *(unsigned int*)($cm_fa + 16)
      printf "LR         = 0x%08x\n", *(unsigned int*)($cm_fa + 20)
      printf "stacked PC = 0x%08x\n", $cm_pc
      printf "xPSR       = 0x%08x\n", $cm_xpsr
      if $cm_xpsr & 0x200
        printf "  xPSR bit9 = 1 -> frame 8 byte hizalamasi icin 1 word doldurma icerir\n"
      end
      printf "\n"
      printf "SINIRLAR (oku):\n"
      printf "  * Bu cozum KANIT DEGILDIR. Kontroller yalnizca desteklenmeyen\n"
      printf "    turleri ve bariz sacmaliklari eler; verilen adresin gercekten\n"
      printf "    bir exception frame'i oldugunu GOSTERMEZ.\n"
      printf "  * 'stacked PC' kesilen baglamin donus adresidir. Hatali komutun\n"
      printf "    adresi olup olmadigi exception turune baglidir (precise/imprecise).\n"
      printf "  * Yalnizca standart 8 word frame, tek guvenlik durumu desteklenir.\n"
    end
  end
end
document cm-frame
Kullanim: cm-frame <frame_adresi> <EXC_RETURN>
EXC_RETURN mimari kodlamaya gore SIKI dogrulanir (Arm 107706, EXC_RETURN
bit alanlari): [31:7] RES1 hepsi bir, [1] sifir, Mode=0 iken SPSEL=1 olamaz.
Desteklenen bicimler yalnizca standart (FType=1), varsayilan callee yigmali
(DCRS=1), tek guvenlik durumlu (ES=S) frame'lerdir; pratikte alti deger:
Secure 0xFFFFFFF1 / 0xFFFFFFF9 / 0xFFFFFFFD ve Non-secure 0xFFFFFFB0 /
0xFFFFFFB8 / 0xFFFFFFBC. 0xFFFFFFB1 / B9 / BD DESTEKLENMEZ (ES=1, S=0).
Bu, mimari olarak gecerli bicimlerin bilincli olarak secilmis bir alt kumesidir.
SINIRLI ve akli basinda (sanity-checked) bir exception-frame cozucusudur;
bir dogrulayici DEGILDIR. Verilen adresin gercek bir frame oldugunu
kanitlayamaz. Reddettikleri: EXC_RETURN bicimi bozuk, FP-genisletilmis
(FType=0) frame, ek durum baglami (DCRS=0), guvenlik durumlari arasi gecis,
stacking hatasi bildiren CFSR, hizasiz adres, RAM penceresi disi adres,
yigilmis xPSR'de T biti 0, yigilmis PC 0 / tek / kod penceresi disi.
Red mesajlari kategorilidir: [BICIMSIZ EXC_RETURN], [DESTEKLENMIYOR],
[STACKING HATASI], [GECERSIZ FRAME ADRESI], [GECERSIZ FRAME ICERIGI].
Aktif $sp'den TAHMIN ETMEZ. Bu kontroller ELEMEDIR, KANIT DEGILDIR.
end

# ---------------------------------------------------------------------------
define cm-fault
  set $cm_cfsr = *(unsigned int*)0xE000ED28
  set $cm_hfsr = *(unsigned int*)0xE000ED2C
  printf "--- fault status registers ---\n"
  printf "CFSR  = 0x%08x  (MMFSR=0x%02x BFSR=0x%02x UFSR=0x%04x)\n", \
         $cm_cfsr, ($cm_cfsr & 0xff), (($cm_cfsr >> 8) & 0xff), (($cm_cfsr >> 16) & 0xffff)

  printf "HFSR  = 0x%08x", $cm_hfsr
  if $cm_hfsr & 0x40000000
    printf "  FORCED=1 -> baska bir fault HardFault'a yukseltildi (escalation)\n"
  else
    printf "  FORCED=0 -> escalation YOK\n"
  end
  if $cm_hfsr & 0x2
    printf "        VECTTBL=1 -> vector table okuma hatasi\n"
  end

  # MMFAR / BFAR yalnizca ilgili VALID biti 1 iken anlamlidir.
  if $cm_cfsr & 0x80
    printf "MMFAR = 0x%08x  (MMARVALID=1 -> adres GECERLI)\n", *(unsigned int*)0xE000ED34
  else
    printf "MMFAR = 0x%08x  (MMARVALID=0 -> icerik ANLAMSIZ, kullanma)\n", *(unsigned int*)0xE000ED34
  end
  if $cm_cfsr & 0x8000
    printf "BFAR  = 0x%08x  (BFARVALID=1 -> adres GECERLI)\n", *(unsigned int*)0xE000ED38
  else
    printf "BFAR  = 0x%08x  (BFARVALID=0 -> icerik ANLAMSIZ, kullanma)\n", *(unsigned int*)0xE000ED38
  end

  # Set olan CFSR bitlerini ISIMLERIYLE listele (Armv8-M; bit konumlari
  # vendored CMSIS core_cm33.h ile ayni). Ders 08 triage adimi 4.
  printf "SET   :"
  if $cm_cfsr & 0x1
    printf " MMFSR.IACCVIOL"
  end
  if $cm_cfsr & 0x2
    printf " MMFSR.DACCVIOL"
  end
  if $cm_cfsr & 0x8
    printf " MMFSR.MUNSTKERR"
  end
  if $cm_cfsr & 0x10
    printf " MMFSR.MSTKERR"
  end
  if $cm_cfsr & 0x20
    printf " MMFSR.MLSPERR"
  end
  if $cm_cfsr & 0x80
    printf " MMFSR.MMARVALID"
  end
  if $cm_cfsr & 0x100
    printf " BFSR.IBUSERR"
  end
  if $cm_cfsr & 0x200
    printf " BFSR.PRECISERR"
  end
  if $cm_cfsr & 0x400
    # Genel Armv8-M/CMSIS adi IMPRECISERR. Cortex-M33 icin dogrulanmis
    # bir alan olarak SUNULMUYOR (bkz. lessons/08-faults/README.md).
    printf " BFSR.bit2(generic-IMPRECISERR,M33:dogrulanmadi)"
  end
  if $cm_cfsr & 0x800
    printf " BFSR.UNSTKERR"
  end
  if $cm_cfsr & 0x1000
    printf " BFSR.STKERR"
  end
  if $cm_cfsr & 0x2000
    printf " BFSR.LSPERR"
  end
  if $cm_cfsr & 0x8000
    printf " BFSR.BFARVALID"
  end
  if $cm_cfsr & 0x10000
    printf " UFSR.UNDEFINSTR"
  end
  if $cm_cfsr & 0x20000
    printf " UFSR.INVSTATE"
  end
  if $cm_cfsr & 0x40000
    printf " UFSR.INVPC"
  end
  if $cm_cfsr & 0x80000
    printf " UFSR.NOCP"
  end
  if $cm_cfsr & 0x100000
    printf " UFSR.STKOF"
  end
  if $cm_cfsr & 0x1000000
    printf " UFSR.UNALIGNED"
  end
  if $cm_cfsr & 0x2000000
    printf " UFSR.DIVBYZERO"
  end
  if $cm_cfsr == 0
    printf " (hic)"
  end
  printf "\n"

  if ($cm_cfsr & 0x00100000) || ($cm_cfsr & 0x00001000) || ($cm_cfsr & 0x00000010)
    printf "UYARI: STACKING hatasi bildirilmis (STKOF/STKERR/MSTKERR).\n"
    printf "       Bellekte gecerli bir exception frame olmayabilir.\n"
  end
  if ($cm_cfsr & 0x00000800) || ($cm_cfsr & 0x00000008)
    printf "NOT: UNSTACKING hatasi (UNSTKERR/MUNSTKERR) - donus sirasinda olustu.\n"
  end
  printf "NOT: CFSR/HFSR bitleri yapiskandir (sticky); 1 yazilarak temizlenir.\n"
end
document cm-fault
CFSR / HFSR / MMFAR / BFAR degerlerini basar.
FORCED bitini KOSULLU yorumlar, MMFAR/BFAR icin gecerlilik bitlerini
raporlar, set olan CFSR bitlerini isimleriyle listeler ve bir
stacking/unstacking hatasi varsa uyarir.
end

# ---------------------------------------------------------------------------
define cm-vectors
  set $vt = *(unsigned int*)0xE000ED08
  printf "--- vector table (VTOR = 0x%08x) ---\n", $vt
  printf "[0]  Initial MSP  = 0x%08x   (adres DEGIL, SP baslangic degeri)\n", *(unsigned int*)($vt + 0)
  printf "[1]  Reset        = 0x%08x\n", *(unsigned int*)($vt + 4)
  printf "[2]  NMI          = 0x%08x\n", *(unsigned int*)($vt + 8)
  printf "[3]  HardFault    = 0x%08x\n", *(unsigned int*)($vt + 12)
  printf "[4]  MemManage    = 0x%08x\n", *(unsigned int*)($vt + 16)
  printf "[5]  BusFault     = 0x%08x\n", *(unsigned int*)($vt + 20)
  printf "[6]  UsageFault   = 0x%08x\n", *(unsigned int*)($vt + 24)
  printf "[7]  SecureFault  = 0x%08x\n", *(unsigned int*)($vt + 28)
  printf "[11] SVCall       = 0x%08x\n", *(unsigned int*)($vt + 44)
  printf "[14] PendSV       = 0x%08x\n", *(unsigned int*)($vt + 56)
  printf "[15] SysTick      = 0x%08x\n", *(unsigned int*)($vt + 60)
  printf "Not: handler girdilerinin bit0'i 1'dir (Thumb). [0] bir handler\n"
  printf "     adresi degildir; ayrilmis (reserved) girdiler 0 olabilir.\n"
end
document cm-vectors
VTOR'u okur ve vector table girdilerini listeler.
end

# ---------------------------------------------------------------------------
define cm-mpu
  printf "MPU_TYPE = 0x%08x  (region sayisi = %d)\n", \
         *(unsigned int*)0xE000ED90, ((*(unsigned int*)0xE000ED90) >> 8) & 0xff
  printf "MPU_CTRL = 0x%08x  (bit0 ENABLE, bit2 PRIVDEFENA)\n", *(unsigned int*)0xE000ED94
end
document cm-mpu
MPU tip ve kontrol register'larini gosterir.
end

printf "Cortex-M GDB yardimcilari yuklendi.\n"
printf "  cm-connect  cm-regs  cm-mode  cm-stack  cm-fault  cm-vectors  cm-mpu\n"
printf "  cm-frame <adres> <EXC_RETURN>    cm-exc-return <deger>\n"
printf "Hedef mimarisi: 'set architecture armv8-m.main' (ozel register'lar icin gerekli)\n"
