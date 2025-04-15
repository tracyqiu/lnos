#include "idt.h"
#include "isr.h"
#include "x86.h"

#define KERNEL_SEGMENT_SELECTOR 0x08

// interrupt gate
typedef struct {
   uint16_t base_low;               // Lower 16 bits of handler function address
   uint16_t segment_selector;       // Kernel segment selector
   uint8_t always0;                 // Reserved
   /* First byte
    * Bit 7: P=1, interrupt is present
    * Bits 6-5: DPL=00, privilege level of caller (0=kernel..3=user)
    * Bit 4: S=0, set to 0 for interrupt gates
    * Bits 3-0: TYPE 1110 = decimal 14 = "32 bit interrupt gate"
    * */
   uint8_t flags;
   uint16_t base_high;              // Higher 16 bits of handler function address
} __attribute__((packed)) idt_entry_t;

// register IDTR
struct idt_descriptor_t {
   uint16_t limit;                  // IDT size
   uint32_t base;                   // IDT address
} __attribute__((packed));


static idt_entry_t idt[256];
static struct idt_descriptor_t lidt = { sizeof(idt_entry_t) * 256 - 1, (uint32_t)&idt };


//------------------------------------------------------------------------------
static void load_idt()
//------------------------------------------------------------------------------
{
   // Don't make the mistake of loading &idt -- always load &lidt
   asm volatile("lidt (%0)" : : "r" (&lidt));
}

//------------------------------------------------------------------------------
static void set_idt_gate(uint8_t num, uint32_t base)
//------------------------------------------------------------------------------
{
   idt[num].base_low = base & 0xFFFF;
   idt[num].segment_selector = KERNEL_SEGMENT_SELECTOR;
   idt[num].always0 = 0;
   idt[num].flags = 0x8E;  // 10001110b, P=1, DPL=00, S=0, TYPE=1110 (32位中断门)
   idt[num].base_high = (base >> 16) & 0xFFFF;
}

//------------------------------------------------------------------------------
void init_idt()
//------------------------------------------------------------------------------
{
   set_idt_gate(0, (uint32_t) isr0);
   set_idt_gate(1, (uint32_t) isr1);
   set_idt_gate(2, (uint32_t) isr2);
   set_idt_gate(3, (uint32_t) isr3);
   set_idt_gate(4, (uint32_t) isr4);
   set_idt_gate(5, (uint32_t) isr5);
   set_idt_gate(6, (uint32_t) isr6);
   set_idt_gate(7, (uint32_t) isr7);
   set_idt_gate(8, (uint32_t) isr8);
   set_idt_gate(9, (uint32_t) isr9);
   set_idt_gate(10, (uint32_t) isr10);
   set_idt_gate(11, (uint32_t) isr11);
   set_idt_gate(12, (uint32_t) isr12);
   set_idt_gate(13, (uint32_t) isr13);
   set_idt_gate(14, (uint32_t) isr14);
   set_idt_gate(15, (uint32_t) isr15);
   set_idt_gate(16, (uint32_t) isr16);
   set_idt_gate(17, (uint32_t) isr17);
   set_idt_gate(18, (uint32_t) isr18);
   set_idt_gate(19, (uint32_t) isr19);
   set_idt_gate(20, (uint32_t) isr20);
   set_idt_gate(21, (uint32_t) isr21);
   set_idt_gate(22, (uint32_t) isr22);
   set_idt_gate(23, (uint32_t) isr23);
   set_idt_gate(24, (uint32_t) isr24);
   set_idt_gate(25, (uint32_t) isr25);
   set_idt_gate(26, (uint32_t) isr26);
   set_idt_gate(27, (uint32_t) isr27);
   set_idt_gate(28, (uint32_t) isr28);
   set_idt_gate(29, (uint32_t) isr29);
   set_idt_gate(30, (uint32_t) isr30);
   set_idt_gate(31, (uint32_t) isr31);

   // setup_8259a(remap the PIC, master PIC: 0x20-0x27, slave PIC: 0x28-0x2F)
   x86_outb(0x20, 0x11);
   x86_outb(0xA0, 0x11);
   x86_outb(0x21, 0x20);
   x86_outb(0xA1, 0x28);
   x86_outb(0x21, 0x04);
   x86_outb(0xA1, 0x02);
   x86_outb(0x21, 0x01);
   x86_outb(0xA1, 0x01);
   x86_outb(0x21, 0x0);
   x86_outb(0xA1, 0x0);

   set_idt_gate(32, (uint32_t)irq0);
   set_idt_gate(33, (uint32_t)irq1);
   set_idt_gate(34, (uint32_t)irq2);
   set_idt_gate(35, (uint32_t)irq3);
   set_idt_gate(36, (uint32_t)irq4);
   set_idt_gate(37, (uint32_t)irq5);
   set_idt_gate(38, (uint32_t)irq6);
   set_idt_gate(39, (uint32_t)irq7);
   set_idt_gate(40, (uint32_t)irq8);
   set_idt_gate(41, (uint32_t)irq9);
   set_idt_gate(42, (uint32_t)irq10);
   set_idt_gate(43, (uint32_t)irq11);
   set_idt_gate(44, (uint32_t)irq12);
   set_idt_gate(45, (uint32_t)irq13);
   set_idt_gate(46, (uint32_t)irq14);
   set_idt_gate(47, (uint32_t)irq15);

   load_idt();
}
