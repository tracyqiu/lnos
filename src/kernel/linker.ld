ENTRY(c_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
phys = 0x10000;  /* kernel loaded address */

SECTIONS
{
  . = phys;

  .text : {
    *(.text) 
  }

  .rodata : {
      *(.rodata)
  }

  .data : { 
    *(.data) 
  }

  .bss : {
    *(.bss)
    *(COMMON)
  }
}