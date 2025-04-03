//#include "stdint.h"
#include "print.h"
#include "keyboard.h"


//__attribute__ ((section ("cstart"))) 
void cstart(void)
{
   // clear_screen();
   puts("Welcome to roqiu's LnOS demo ^_^!");

   asm volatile ("sti");
   // init_keyboard();
   puts("Keyboard input is now enabled.\n");

   for (;;);
}