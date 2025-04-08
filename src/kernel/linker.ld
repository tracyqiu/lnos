ENTRY(c_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
phys = 0x10000;  /* kernel loaded address */

SECTIONS
{
  . = phys;

  .text : {
    /* Make sure the entry point code is placed at the very beginning, 
     * otherwise the execution entry point will become random when the assembly code file is added. 
     **/
    *(.text.c_start) 
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