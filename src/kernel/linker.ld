ENTRY(c_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
phys = 0x10000;  /* kernel loaded address */

SECTIONS
{
  . = phys;

  .text : ALIGN(4K) {
    /* Make sure the entry point code is placed at the very beginning,
     * otherwise the execution entry point will become random when the assembly code file is added.
     **/
    *(.text.c_start)
    *(.text)
  }

  .rodata : ALIGN(4K) {
      *(.rodata)
  }

  .data : ALIGN(4K) {
    *(.data)
  }

  .bss : ALIGN(4K) {
    __bss_start = .;
    *(.bss)
    *(COMMON)
    __bss_end = .;
  }
}