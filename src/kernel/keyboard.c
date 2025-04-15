#include "keyboard.h"
#include "print.h"
#include "x86.h"
#include "isr.h"

static const char scancode_to_ascii[] = {
   0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
   '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
   0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
   0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
   '*', 0, ' ', 0
};

//------------------------------------------------------------------------------
static void keyboard_handler(registers_t* regs)
//------------------------------------------------------------------------------
{
   (void)regs;
   unsigned char scancode = x86_inb(0x60);

   if (scancode != '\0' && scancode < 0x80) {
      char c = scancode_to_ascii[scancode];
      if (c) {
         char buf[2] = {c, 0};
         puts(buf);
      }
   }
}

//------------------------------------------------------------------------------
void init_keyboard()
//------------------------------------------------------------------------------
{
   register_interrupt_handler(IRQ1, keyboard_handler);
}
