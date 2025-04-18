#ifndef X86_H
#define X86_H

#include "stdint.h"

// void x86_output_char(char c, uint8_t page);

uint8_t __attribute__((cdecl)) x86_inb(uint16_t port);

void __attribute__((cdecl)) x86_outb(uint16_t port, uint8_t value);

void __attribute__((cdecl)) x86_jmp_to_high_virtual_addr();

#endif //X86_H
