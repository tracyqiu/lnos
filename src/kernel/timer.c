#include "timer.h"
#include "isr.h"
#include "x86.h"
#include "print.h"
#include "stdlib.h"
#include "schedule.h"


volatile uint32_t system_ticks = 0;
volatile uint32_t system_time_ms = 0;
static uint32_t cpu_frequency = 0;

//------------------------------------------------------------------------------
static void timer_handler(registers_t* regs)
//------------------------------------------------------------------------------
{
   (void)regs;
   system_ticks++;
   system_time_ms += TIMER_INTERVAL_MS;

   /* // if printed the tick will see it increase fast
   puts("Tick: ");
   char buf[32];
   puts(itoa(system_ticks, buf, 10));
   puts("\n");
   */

   if (system_ticks % 10 == 0)
   {
      schedule_task();
   }
}

//------------------------------------------------------------------------------
uint32_t get_tick_count(void)
//------------------------------------------------------------------------------
{
    return system_ticks;
}

//------------------------------------------------------------------------------
uint32_t get_system_time_ms(void)
//------------------------------------------------------------------------------
{
    return system_time_ms;
}

//------------------------------------------------------------------------------
void sleep_ms(uint32_t milliseconds)
//------------------------------------------------------------------------------
{
    uint32_t start_time = get_system_time_ms();
    uint32_t end_time = start_time + milliseconds;
    printf("Sleep for %u ms...\n", end_time);

    while (get_system_time_ms() < end_time) {
        // schedule();
        __asm__ volatile ("hlt"); // suspend the CPU until the next interrupt
    }
}

//------------------------------------------------------------------------------
bool is_timeout(uint32_t start_time, uint32_t timeout_ms)
//------------------------------------------------------------------------------
{
   uint32_t current_time = get_system_time_ms();
   return (current_time - start_time) >= timeout_ms;
}

//------------------------------------------------------------------------------
uint32_t timeout_after_ms(uint32_t milliseconds)
//------------------------------------------------------------------------------
{
   return get_system_time_ms() + milliseconds;
}

//------------------------------------------------------------------------------
uint64_t rdtsc(void)
//------------------------------------------------------------------------------
{
    uint32_t low, high;
    __asm__ volatile ("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

//------------------------------------------------------------------------------
uint32_t get_cpu_frequency(void)
//------------------------------------------------------------------------------
{
   if (cpu_frequency != 0) {
      return cpu_frequency;
   }

   // configure the PIT channel 2 for a single count to measure the CPU frequency
   x86_outb(PIT_COMMAND, PIT_CMD_CHANNEL2 | PIT_CMD_ACCESS_BOTH | PIT_CMD_MODE0);
   x86_outb(PIT_CHANNEL2, 0xFF);
   x86_outb(PIT_CHANNEL2, 0xFF);

   uint32_t start_tsc = rdtsc();

   // wait for a while
   for (volatile int i = 0; i < 1000000; i++) {
      __asm__ volatile ("nop");
   }

   uint32_t end_tsc = rdtsc();

   // it's a rough estimate
   // assuming the cycle lasts approximately 1ms.
   cpu_frequency =  (uint32_t)((end_tsc - start_tsc) * 1000);

   return cpu_frequency;
}

//------------------------------------------------------------------------------
void udelay(uint32_t microseconds)
//------------------------------------------------------------------------------
{
   if (cpu_frequency == 0) {
      // fall back to a simple loop delay
      volatile uint32_t count = microseconds * 1000;
      while (count--) {
         __asm__ volatile ("nop");
      }
      return;
   }

   uint32_t start = (uint32_t)rdtsc();
   uint32_t cycles = (cpu_frequency * microseconds) / 1000000;

   while (((uint32_t)rdtsc() - start) < cycles) {
      __asm__ volatile ("pause");
   }
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

   cpu_frequency = get_cpu_frequency();

   printf("Timer initialized. Frequency: %d Hz, CPU: %u MHz\n",
         frequency, cpu_frequency / 1000000);
}

