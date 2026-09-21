/*
 * MPS2-AN505 (Cortex-M33) Secure state linker script.
 *
 * Bellek yerlesimi QEMU mps2-an505 modelinden dogrulanmistir:
 *   0x10000000  ZBT SSRAM1 (Secure alias)      - 4 MB fiziksel
 *   0x30000000  IoT Kit internal SRAM (Secure) - 32 KB fiziksel
 *
 * FLASH uzunlugunu bilerek 512 KB ile sinirliyoruz: gercek bir MCU gibi
 * davranmasi ve "region overflow" hatalarinin anlamli olmasi icin.
 * Tam liste: docs/MEMORY_MAP.md
 */

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x10000000, LENGTH = 512K
  RAM   (rwx) : ORIGIN = 0x30000000, LENGTH = 32K
}

/* Stack ve heap icin ayrilan alan (RAM'in tepesinde) */
__stack_size = 4K;
__heap_size  = 0K;

ENTRY(Reset_Handler)

SECTIONS
{
  /* Vector table yuk goruntusunun en basinda olmali.

     Reset'teki Secure VTOR degeri MIMARI BIR SABIT DEGILDIR; hedefe ozeldir.
     Bu hedefte QEMU v8.2.2 hw/arm/mps2-tz.c icinde
         mmc->init_svtor = 0x10000000;
     olarak tanimlidir. Baska bir cihazda farklidir; startup kodu her halukarda
     VTOR'u acikca yazar (bkz. system_init.c). */
  .vectors : ALIGN(512)
  {
    __vectors_start = .;
    KEEP(*(.vectors))
    __vectors_end = .;
  } > FLASH

  .text :
  {
    *(.text)
    *(.text*)
    *(.glue_7) *(.glue_7t)
    . = ALIGN(4);
  } > FLASH

  .rodata :
  {
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  /* ARM exception/unwind tablolari (kullanmasak da toplanmali) */
  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
  .ARM.exidx   : {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
  } > FLASH

  . = ALIGN(4);

  /* Initialized data: FLASH'ta durur (LMA), RAM'de calisir (VMA).
     Reset_Handler bu bloku __etext -> __data_start kopyalar.

     "> RAM AT > FLASH" kullaniyoruz, "AT (__etext)" DEGIL.
     Fark kritik: acik bir AT(adres) yalnizca LMA'yi SOYLER, o baytlarin
     FLASH bolgesinden dusulmesini SAGLAMAZ. Eski surumde bu yuzden
     .text + .data yuk goruntusu 512 KB FLASH sinirini assa bile linker
     hata vermiyordu (bagimsiz denetim bulgusu I5).
     "AT > FLASH" ise yuk tahsisini gercekten FLASH'a yapar ve tasma
     "region FLASH overflowed" hatasina donusur. */
  .data : ALIGN(4)
  {
    . = ALIGN(4);
    __data_start = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    __data_end = .;
  } > RAM AT > FLASH

  /* .data'nin FLASH'taki kaynak adresi (LMA). Artik linker tarafindan
     turetiliyor; elle hesaplanmiyor. */
  __etext = LOADADDR(.data);

  /* Zero-initialized data: sadece RAM'de yer kaplar, dosyada yer kaplamaz. */
  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    __bss_start = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end = .;
  } > RAM

  /* Heap (su an 0) ve stack. Stack full-descending: tepeden asagi buyur. */
  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    __heap_start = .;
    . = . + __heap_size;
    __heap_end = .;
  } > RAM

  /* Stack RAM'in EN TEPESINDE durur ve asagi dogru buyur (full descending).
     __stack_top vector table'in [0] girdisine yazilir -> reset'te MSP.

     DIKKAT: buradaki ayirma yalnizca BIR MUHASEBE islemidir. Link zamaninda
     "stack icin 4 KB ayrildi" demek, calisma zamaninda tasmanin engellendigi
     anlamina GELMEZ. Asagidaki ASSERT yalnizca link anindaki yerlesimi
     denetler. Calisma zamani korumasi ayri mekanizmalardan gelir:
     MSPLIM/PSPLIM (ders 04) ve MPU (ders 06). */
  __stack_top   = ORIGIN(RAM) + LENGTH(RAM);
  __stack_limit = __stack_top - __stack_size;

  /* Stack .bss/.heap uzerine tasarsa link asamasinda hata ver. */
  ASSERT(__stack_limit >= __heap_end,
         "RAM overflow: stack bolgesi .bss/.heap ile cakisiyor")

  /* Ikinci bir emniyet kemeri: tam yuk goruntusu (text + rodata + .data'nin
     FLASH kopyasi) bildirilen FLASH bolgesine sigmali. "AT > FLASH" bunu
     zaten zorlar; bu ASSERT hata mesajini acik hale getirir. */
  __flash_load_end = LOADADDR(.data) + SIZEOF(.data);
  ASSERT(__flash_load_end <= ORIGIN(FLASH) + LENGTH(FLASH),
         "FLASH overflow: yuk goruntusu (.text + .rodata + .data LMA) FLASH'a sigmiyor")

  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.comment) }
}
