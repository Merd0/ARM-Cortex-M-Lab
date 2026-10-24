/* Reuse the audited memory allocation and FLASH LMA rules. */
INCLUDE mps2-an505-secure.ld
__data_load_start = LOADADDR(.data);
ASSERT(__data_load_start != ADDR(.data), "LMA must differ from VMA")
ASSERT(SIZEOF(.vectors) == 64, "core vector table must have sixteen entries")
