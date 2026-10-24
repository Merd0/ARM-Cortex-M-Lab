MEMORY {
  FLASH (rx) : ORIGIN = 0x00080000, LENGTH = 128K
  RAM (rwx) : ORIGIN = 0x20004000, LENGTH = 16K
}
ENTRY(NS_Reset_Handler)
SECTIONS {
  .vectors : ALIGN(512) { KEEP(*(.vectors)) } > FLASH
  .text : { *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
  .ARM.extab : { *(.ARM.extab*) } > FLASH
  .ARM.exidx : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } > FLASH
  .data : ALIGN(4) {
    __data_start = .; *(.data*) . = ALIGN(4); __data_end = .;
  } > RAM AT > FLASH
  __etext = LOADADDR(.data);
  .bss (NOLOAD) : ALIGN(4) {
    __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .;
  } > RAM
  __stack_top = ORIGIN(RAM)+LENGTH(RAM);
  __stack_limit = __stack_top-4K;
  ASSERT(__bss_end <= __stack_limit, "Non-secure stack overlaps data")
  ASSERT(LOADADDR(.data)+SIZEOF(.data) <= ORIGIN(FLASH)+LENGTH(FLASH), "NS FLASH overflow")
  /DISCARD/ : { *(.note.GNU-stack) *(.comment) }
}
