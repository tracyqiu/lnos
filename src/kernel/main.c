#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"

//------------------------------------------------------------------------------
void cstart(void)
//------------------------------------------------------------------------------
{
   // clear_screen();
   puts("Welcome to roqiu's LnOS demo ^_^!\n");

   init_gdt();
   puts("GDT initialized.\n");

   init_idt();
   puts("IDT initialized.\n");
   asm volatile ("sti");

   init_keyboard();
   puts("Keyboard input is now enabled.\n");

   for (;;);
}
