#include "isr.h"
#include "idt.h"
#include "x86.h"
#include "print.h"


static handler_ptr interrupt_handlers[256];

// To print the message which defines every exception
static char *exception_messages[] = {
   "Division By Zero",
   "Debug",
   "Non Maskable Interrupt",
   "Breakpoint",
   "Into Detected Overflow",
   "Out of Bounds",
   "Invalid Opcode",
   "No Coprocessor",

   "Double Fault",
   "Coprocessor Segment Overrun",
   "Bad TSS",
   "Segment Not Present",
   "Stack Fault",
   "General Protection Fault",
   "Page Fault",
   "Unknown Interrupt",

   "Coprocessor Fault",
   "Alignment Check",
   "Machine Check",
   "Reserved",
   "Reserved",
   "Reserved",
   "Reserved",
   "Reserved",

   "Reserved",
   "Reserved",
   "Reserved",
   "Reserved",
   "Reserved",
   "Reserved",
   "Reserved",
   "Reserved"
};

//------------------------------------------------------------------------------
void register_interrupt_handler(uint8_t n, handler_ptr handler)
//------------------------------------------------------------------------------
{
   interrupt_handlers[n] = handler;
}

//------------------------------------------------------------------------------
void isr_handler(registers_t *regs)
//------------------------------------------------------------------------------
{
   puts("received interrupt: ");
   puts(exception_messages[regs->int_no]);
}

//------------------------------------------------------------------------------
void irq_handler(registers_t *regs)
//------------------------------------------------------------------------------
{
   if (interrupt_handlers[regs->int_no] != NULL) {
      handler_ptr handler = interrupt_handlers[regs->int_no];
      handler(regs);
   }

   // send EOI(end of interrupt) signal
   if (regs->int_no >= 40) {
      x86_outb(0xA0, 0x20);   // slaver
   }
   x86_outb(0x20, 0x20);      // master
}