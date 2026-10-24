MEMORY {
  FLASH (rx) : ORIGIN = 0x10000000, LENGTH = 448K
  NSC (rx) : ORIGIN = 0x10070000, LENGTH = 4K
  RAM (rwx) : ORIGIN = 0x30000000, LENGTH = 16K
}
ENTRY(Reset_Handler)
SECTIONS {
  .vectors : ALIGN(512) { KEEP(*(.vectors)) } > FLASH
  .text : { *(.text*) *(.rodata*) *(.glue_7*) . = ALIGN(4); } > FLASH
  .ARM.extab : { *(.ARM.extab*) } > FLASH
  .ARM.exidx : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } > FLASH
  .gnu.sgstubs : ALIGN(32) { *(.gnu.sgstubs*) . = ALIGN(32); } > NSC
  __nsc_start = ADDR(.gnu.sgstubs);
  __nsc_end = ADDR(.gnu.sgstubs) + SIZEOF(.gnu.sgstubs);
  .data : ALIGN(4) {
    __data_start = .; KEEP(*(.secret)) *(.data*) . = ALIGN(4); __data_end = .;
  } > RAM AT > FLASH
  __etext = LOADADDR(.data);
  .bss (NOLOAD) : ALIGN(4) {
    __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .;
  } > RAM
  __stack_top = ORIGIN(RAM) + LENGTH(RAM);
  __stack_limit = __stack_top - 4K;
  ASSERT(__bss_end <= __stack_limit, "Secure stack overlaps data")
  ASSERT(__nsc_end > __nsc_start && __nsc_end <= ORIGIN(NSC)+LENGTH(NSC), "NSC bounds")
  ASSERT(LOADADDR(.data)+SIZEOF(.data) <= ORIGIN(FLASH)+LENGTH(FLASH), "Secure FLASH overflow")
  /DISCARD/ : { *(.note.GNU-stack) *(.comment) }
}
