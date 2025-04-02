#include "print.h"
#include "stdint.h"

// vga address
#define VGA_STARTED_MEMORY_ADDR 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5
#define VGA_OFFSET_LOW 0x0f       // 当控制寄存器0x3d4设置为0xf时, 数据寄存器将保存低字节
#define VGA_OFFSET_HIGH 0x0e      // 当控制寄存器0x3d4设置为0xe时, 数据寄存器将保存高字节

// vga color
#define COLOR_BLACK 0x01
#define COLOR_BLUE 0x01
#define COLOR_GREEN 0x02
#define COLOR_CYAN 0x03
#define COLOR_RED 0x04
#define COLOR_MAGENTA 0x05
#define COLOR_BROWN 0x06
#define COLOR_LIGHT_GRAY 0x07
#define COLOR_DARK_GRAY 0x08
#define COLOR_LIGHT_BLUE 0x09
#define COLOR_LIGHT_GREEN 0x0A
#define COLOR_LIGHT_CYAN 0x0B
#define COLOR_LIGHT_RED 0x0C
#define COLOR_LIGHT_MAGENTA 0x0D
#define COLOR_YELLOW 0x0E
#define COLOR_WHITE 0x0F


////////////////////////////////////////////////////////////////////////////////

int16_t find_output_row();
void putc(char c, int16_t col, int16_t row);
void putint(int16_t i, int16_t col, int16_t row);
void scroll_screen(int16_t row);
void scroll();
int16_t get_cursor();
void set_cursor(int16_t offset);


////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------
void puts(const char* str) 
//------------------------------------------------------------------------------
{
   int16_t row = find_output_row();
   // putint(row + 1, 0, row);
   // int16_t col = 3;

   int16_t col = 0;

   while(*str)
   {
      if (*str == '\n') {
         col = 0;
         ++row;
         scroll_screen(row);
      }
      else if (*str == '\r') {
         col = 0;
      }
      else 
      {
         putc(*str, col, row);
         ++col;
         
         if (col >= SCREEN_WIDTH) {
            col = 0;
            ++row;
            scroll_screen(row);
         }
      }

      ++str;
   }

   set_cursor(row * SCREEN_WIDTH + col);
}


//------------------------------------------------------------------------------
void clear_screen() {
//------------------------------------------------------------------------------
   uint16_t *video_memory = (uint16_t*)VGA_STARTED_MEMORY_ADDR;
   uint16_t blank = (uint16_t)' ' | (COLOR_WHITE << 8);
   
   for (int16_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
      video_memory[i] = blank;
   }
   set_cursor(0);
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
void scroll_screen(int16_t row) {
//------------------------------------------------------------------------------
   if (row >= SCREEN_HEIGHT) {
      scroll();
      row = SCREEN_HEIGHT - 1;
   }
}


//------------------------------------------------------------------------------
void scroll() {
//------------------------------------------------------------------------------
   uint16_t *video_memory = (uint16_t*)VGA_STARTED_MEMORY_ADDR;
   
   //copy line[n+1] to line[n]
   for (int16_t i = 0; i < SCREEN_HEIGHT - 1; i++) {
      for (int16_t j = 0; j < SCREEN_WIDTH; j++) {
         video_memory[i * SCREEN_WIDTH + j] = video_memory[(i + 1) * SCREEN_WIDTH + j];
      }
   }
   
   // clear the last line
   int offset = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH;
   for (int16_t j = 0; j < SCREEN_WIDTH; j++) {
      video_memory[offset + j] = (uint16_t)' ' | (COLOR_WHITE << 8);
   }
   
   set_cursor(offset + 0);
}


//------------------------------------------------------------------------------
// refer to Writing VGA Driver https://dev.to/frosnerd/writing-my-own-vga-driver-22nn
// 格式1: asm(“汇编代码” : 输出部分 : 输入部分 : 改变的寄存器部分)。
// 格式2: __asm__(“汇编代码” : 输出部分 : 输入部分 : 改变的寄存器部分)。
int16_t get_cursor() {
//------------------------------------------------------------------------------
   asm volatile ("outb %%al, %%dx" : : "a"(VGA_OFFSET_HIGH), "d"(VGA_CTRL_REGISTER));
   
   unsigned char offset_high;
   asm volatile ("in %%dx, %%al" : "=a" (offset_high) : "d"(VGA_DATA_REGISTER));

   asm volatile ("outb %%al, %%dx" : : "a"(VGA_OFFSET_LOW), "d"(VGA_CTRL_REGISTER));

   unsigned char offset_low;
   asm volatile ("in %%dx, %%al" : "=a" (offset_low) : "d"(VGA_DATA_REGISTER));
   
   return ((offset_high << 8) + offset_low) * 2;
}

//------------------------------------------------------------------------------
// refer to Writing VGA Driver https://dev.to/frosnerd/writing-my-own-vga-driver-22nn
void set_cursor(int16_t offset) {
//------------------------------------------------------------------------------
   // 通过端口0x3D4, 0x3D5用于控制VGA光标
   asm volatile ("outb %%al, %%dx" : : "a"(VGA_OFFSET_HIGH), "d"(VGA_CTRL_REGISTER));
   asm volatile ("outb %%al, %%dx" : : "a"((offset >> 8) & 0xFF), "d"(VGA_DATA_REGISTER ));
   asm volatile ("outb %%al, %%dx" : : "a"(VGA_OFFSET_LOW), "d"(VGA_CTRL_REGISTER));
   asm volatile ("outb %%al, %%dx" : : "a"(offset & 0xFF), "d"(VGA_DATA_REGISTER ));
}

//------------------------------------------------------------------------------
int16_t find_output_row() {
//------------------------------------------------------------------------------
   uint16_t *output_vga_addr = (uint16_t*)VGA_STARTED_MEMORY_ADDR;
   
   // find first empty row
   for (int16_t row = SCREEN_HEIGHT - 1; row >= 0; row--) {
      for (int16_t col = 0; col < SCREEN_WIDTH; col++) {
         int16_t offset = row * SCREEN_WIDTH + col;
         // if has valid character, return the next row
         if ((output_vga_addr[offset] & 0xFF) != ' ' && (output_vga_addr[offset] & 0xFF) != 0) {
            // if current row is the last row, scroll screen
            // if call scroll after find_output_row returned will cause the last line contents missing and cannot out put in the last line
            if (row + 1 >= SCREEN_HEIGHT) {
               scroll();
               return SCREEN_HEIGHT - 1;
            }else {
               return row + 1;
            }
         }
      }
   }
   
   return 0; // if not found, return the first row
}

//------------------------------------------------------------------------------
void putc(char c, int16_t col, int16_t row)
//------------------------------------------------------------------------------
{
   uint16_t *output_vga_addr = (uint16_t*)VGA_STARTED_MEMORY_ADDR;
   int16_t offset = row * SCREEN_WIDTH + col;
   output_vga_addr[offset] = (uint16_t)c | ((uint16_t)COLOR_LIGHT_GRAY << 8);
}

//------------------------------------------------------------------------------
void putint(int16_t num, int16_t col, int16_t row) {
//------------------------------------------------------------------------------
   if (num < 0) {
      putc('-', col, row);
      col++;
      num = -num;
   }
   
   if (num == 0) {
      putc('0', col, row);
      return;
   }
   
   int temp = num;
   int num_count = 0;
   while (temp > 0) {
      num_count++;
      temp /= 10;
   }
   
   char buffer[16];
   int index = num_count - 1;
   
   while (num > 0) {
      buffer[index] = '0' + (num % 10);
      num /= 10;
      index--;
   }
   
   for (int i = 0; i < num_count; i++) {
      putc(buffer[i], col + i, row);
   }
}

