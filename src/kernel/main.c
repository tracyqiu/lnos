#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "keyboard.h"
#include "stdlib.h"
#include "timer.h"


//------------------------------------------------------------------------------
static void test_allocate_memory()
//------------------------------------------------------------------------------
{
   puts("Memory allocation test: ");
   void* mm0 = kernel_malloc(9192);
   void* mm1 = kernel_malloc(2048);
   if (mm1 && mm0) {
      puts("addr0(8KB)=");
      char buf[32];
      puts(itoa((int32_t)mm0, buf, 16));

      puts(", addr1(2KB)=");
      puts(itoa((int32_t)mm1, buf, 16));
      puts(" >>> SUCCESS :)\n");
   }
   else {
      puts("FAILED :(\n");
   }
   kernel_free(mm1);
   kernel_free(mm0);
}


//------------------------------------------------------------------------------
// Specify the section name.
// Otherwise, combining it with the setting in the makefile will prevent other functions from being defined before this one.
__attribute__((section(".text.c_start"))) void cstart(void)
//------------------------------------------------------------------------------
{
   // clear_screen();
   puts("Welcome to roqiu's LnOS demo :)!\n");

   init_gdt();
   puts("GDT initialized.\n");

   init_idt();
   puts("IDT initialized.\n");

   init_memory();
   puts("Memory initialized.\n");
   test_allocate_memory();

   asm volatile ("sti");

   init_timer(50);

   init_keyboard();
   puts("Keyboard input is now enabled.\n");

   for (;;);
}


