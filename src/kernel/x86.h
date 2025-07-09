#ifndef X86_H
#define X86_H

#include "stdint.h"

uint8_t __attribute__((cdecl)) x86_inb(uint16_t port);

void __attribute__((cdecl)) x86_outb(uint16_t port, uint8_t value);

void __attribute__((cdecl)) x86_jmp_to_high_virtual_addr();


uint8_t __attribute__((cdecl)) x86_inw(uint16_t port);
void __attribute__((cdecl)) x86_outw(uint16_t port, uint16_t value);

uint8_t __attribute__((cdecl)) x86_inl(uint16_t port);
void __attribute__((cdecl)) x86_outl(uint16_t port, uint32_t value);

/*
// another way to define inline assembly functions
static inline void x86_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t x86_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void x86_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t x86_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
*/

static inline void io_wait(void)
{
    x86_outb(0x80, 0);
}

#endif //X86_H
