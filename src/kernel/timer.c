#include "timer.h"
#include "isr.h"
#include "x86.h"
#include "print.h"
#include "stdlib.h"


static uint32_t tick = 0;

//------------------------------------------------------------------------------
static void timer_handler(registers_t* regs)
//------------------------------------------------------------------------------
{
   (void)regs;
   tick++;

   /* // if printed the tick will see it increase fast
   puts("Tick: ");
   char buf[32];
   puts(itoa(tick, buf, 10));
   puts("\n");
   */
}

//------------------------------------------------------------------------------
// refer to https://wiki.osdev.org/Programmable_Interval_Timer
// refer to https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/index.html part of 'IRQs and the PIT'
void init_timer(uint32_t frequency)
//------------------------------------------------------------------------------
{
   register_interrupt_handler(IRQ0, timer_handler);

   // Get the PIT(Programmable Interval Timer) value: hardware clock at 1193180 Hz, The Intel 8253/8254 PIT configured it
   uint32_t divisor = 1193180 / frequency;

   /* 0x43:Mode/Command register. 0x36(00110110) means:
    * Bit 7-6: 00, Channel0 means IRQ0
    * Bits 5-4: 11, Access mode lowbyte/highbyte
    * Bit 3-1: 011, Operating mode 3(square wave generator)
    * Bits 0: BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
    * */
   x86_outb(0x43, 0x36);

   // Send the frequency divisor
   x86_outb(0x40, (uint8_t)(divisor & 0xFF));         // low byte
   x86_outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));  // high byte
}