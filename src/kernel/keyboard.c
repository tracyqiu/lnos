#include "keyboard.h"
#include "print.h"

extern void keyboard_handler_wrapper();
extern void setup_keyboard_idt();


// 中断门
typedef struct {
   unsigned short handler_addr_low;
   unsigned short code_seg;
   unsigned char always0;
   unsigned char flags;
   unsigned short handler_addr_high;
} __attribute__((packed)) idt_entry;

// idt_descriptor
struct idt_ptr{
   unsigned short size;
   unsigned int addr;
} __attribute__((packed));



static inline unsigned char inb(unsigned short port) {
   unsigned char ret;
   asm volatile ("in %%dx, %%al" : "=a" (ret) : "d" (port));
   return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
   asm volatile ("out %%al, %%dx" : : "a" (val), "d" (port));
}


void idt_set_gate(unsigned char num, unsigned int addr, unsigned short code_seg, unsigned char flags) {
   // idt_entry* idt = (idt_entry *)0x102000;
   // idt[num].handler_addr_low = (addr & 0xFFFF);
   // idt[num].handler_addr_high = (addr >> 16) & 0xFFFF;
   // idt[num].code_seg = code_seg;
   // idt[num].always0 = 0;
   // idt[num].flags = flags;
   // 使用更安全的方式访问IDT
   unsigned int idt_addr = 0x102000 + num * 8; // 每个IDT项占8字节

   // 使用直接内存操作而不是结构体
   *((unsigned short*)(idt_addr)) = (addr & 0xFFFF); 
   *((unsigned short*)(idt_addr + 2)) = code_seg;
   *((unsigned char*)(idt_addr + 4)) = 0; 
   *((unsigned char*)(idt_addr + 5)) = flags;
   *((unsigned short*)(idt_addr + 6)) = ((addr >> 16) & 0xFFFF);
}

// 键盘扫描码到ASCII的简单映射
const char scancode_to_ascii[] = {
   0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
   '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
   0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
   0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
   '*', 0, ' ', 0
};

void byte_to_hex(unsigned char byte, char *output) {
   const char hex_chars[] = "0123456789ABCDEF";
   output[0] = hex_chars[(byte >> 4) & 0xF];
   output[1] = hex_chars[byte & 0xF];
   output[2] = '\0';
}

// 键盘中断处理函数
void keyboard_handler() {
   unsigned char scancode = inb(0x60);
   
   puts("Key: ");
   char scancodeStr[3];
   byte_to_hex(scancode, scancodeStr);
   puts(scancodeStr);
   
   if (scancode != '\0' && scancode < 0x80) {
      char c = scancode_to_ascii[scancode];
      if (c) {
         char buf[2] = {c, 0};
         puts(buf);
      }
   }

   outb(0x20, 0x20); // send EOI(end of interrupt) signal
}

// 初始化键盘
void init_keyboard() {
   // idt_set_gate(0x21, (unsigned int)keyboard_handler_wrapper, 0x08, 0x8E);
   setup_keyboard_idt();
   puts("Keyboard idt_set_gate.\n");

   outb(0x21, inb(0x21) & ~0x02);  // 取消屏蔽IRQ1
   puts("Keyboard outb.\n");
   
   asm volatile ("sti");
   puts("Keyboard sti.\n");
}