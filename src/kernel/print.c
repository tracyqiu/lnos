#include "print.h"
#include "x86.h"
#include "stdlib.h"
#include "stdarg.h"

// vga address
#define VGA_STARTED_MEMORY_ADDR 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5
#define VGA_OFFSET_LOW 0x0F      // 当控制寄存器0x3d4设置为0xf时, 数据寄存器将保存低字节
#define VGA_OFFSET_HIGH 0x0E     // 当控制寄存器0x3d4设置为0xe时, 数据寄存器将保存高字节

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



//------------------------------------------------------------------------------
// refer to Writing VGA Driver https://dev.to/frosnerd/writing-my-own-vga-driver-22nn
// 格式1: asm(“汇编代码” : 输出部分 : 输入部分 : 改变的寄存器部分)。
// 格式2: __asm__(“汇编代码” : 输出部分 : 输入部分 : 改变的寄存器部分)。
static int16_t get_cursor() {
//------------------------------------------------------------------------------
   asm volatile ("out %%al, %%dx" : : "a"(VGA_OFFSET_HIGH), "d"(VGA_CTRL_REGISTER));

   unsigned char offset_high;
   asm volatile ("in %%dx, %%al" : "=a" (offset_high) : "d"(VGA_DATA_REGISTER));

   asm volatile ("out %%al, %%dx" : : "a"(VGA_OFFSET_LOW), "d"(VGA_CTRL_REGISTER));

   unsigned char offset_low;
   asm volatile ("in %%dx, %%al" : "=a" (offset_low) : "d"(VGA_DATA_REGISTER));

   return ((offset_high << 8) + offset_low) * 2;
}

//------------------------------------------------------------------------------
// refer to Writing VGA Driver https://dev.to/frosnerd/writing-my-own-vga-driver-22nn
static void set_cursor(int16_t offset) {
//------------------------------------------------------------------------------
   // 通过端口0x3D4, 0x3D5用于控制VGA光标
   // asm volatile ("outb %%al, %%dx" : : "a"(VGA_OFFSET_HIGH), "d"(VGA_CTRL_REGISTER));
   // asm volatile ("outb %%al, %%dx" : : "a"((offset >> 8) & 0xFF), "d"(VGA_DATA_REGISTER ));
   // asm volatile ("outb %%al, %%dx" : : "a"(VGA_OFFSET_LOW), "d"(VGA_CTRL_REGISTER));
   // asm volatile ("outb %%al, %%dx" : : "a"(offset & 0xFF), "d"(VGA_DATA_REGISTER ));

   x86_outb(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
   x86_outb(VGA_DATA_REGISTER, (uint8_t)((offset >> 8) & 0xFF));
   x86_outb(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
   x86_outb(VGA_DATA_REGISTER, (uint8_t)(offset & 0xFF));
}

//------------------------------------------------------------------------------
static void scroll() {
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
static void scroll_screen(int16_t* row) {
//------------------------------------------------------------------------------
   if (*row >= SCREEN_HEIGHT) {
      scroll();
      *row = SCREEN_HEIGHT - 1;
   }
}

//------------------------------------------------------------------------------
/*static*/ int16_t find_output_row() {
//------------------------------------------------------------------------------
   uint16_t *output_vga_addr = (uint16_t*)VGA_STARTED_MEMORY_ADDR;

   // find first empty row
   for (int16_t row = SCREEN_HEIGHT - 1; row >= 0; row--) {
      for (int16_t col = 0; col < SCREEN_WIDTH; col++) {
         int16_t offset = row * SCREEN_WIDTH + col;
         // if has valid character, return the next row
         if ((output_vga_addr[offset] & 0xFF) != ' ' && (output_vga_addr[offset] & 0xFF) != 0) {
            // if current row is the last row, scroll screen
            if (row + 1 >= SCREEN_HEIGHT) {
               scroll();
               return SCREEN_HEIGHT - 1;
            }
            else {
               return row + 1;
            }
         }
      }
   }

   return 0; // if not found, return the first row
}

//------------------------------------------------------------------------------
static void putc(char c, int16_t col, int16_t row)
//------------------------------------------------------------------------------
{
   uint16_t *output_vga_addr = (uint16_t*)VGA_STARTED_MEMORY_ADDR;
   int16_t offset = row * SCREEN_WIDTH + col;
   output_vga_addr[offset] = (uint16_t)c | ((uint16_t)COLOR_LIGHT_GRAY << 8);
}

//------------------------------------------------------------------------------
/*static*/ void putint(int16_t num, int16_t col, int16_t row) {
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


//------------------------------------------------------------------------------
static int32_t uint_to_string(uint32_t num, char* buffer, int32_t base, bool uppercase)
//------------------------------------------------------------------------------
{
   if (num == 0) {
      buffer[0] = '0';
      buffer[1] = '\0';
      return 1;
   }

   char digits[] = "0123456789abcdef";
   char upper_digits[] = "0123456789ABCDEF";
   char* digit_chars = uppercase ? upper_digits : digits;

   int32_t idx = 0;
   uint32_t temp_num = num;

   while (temp_num > 0) {
      temp_num /= base;
      idx++;
   }

   buffer[idx] = '\0';
   int32_t pos = idx - 1;

   while (num > 0) {
      buffer[pos--] = digit_chars[num % base];
      num /= base;
   }

   return idx;
}

//------------------------------------------------------------------------------
static int32_t int_to_string(int32_t num, char* buffer, int32_t base, bool uppercase)
//------------------------------------------------------------------------------
{
   if (num == 0) {
      buffer[0] = '0';
      buffer[1] = '\0';
      return 1;
   }

   bool is_negative = false;
   uint32_t unum;

   if (num < 0 && base == 10) {
      is_negative = true;
      unum = (uint32_t)(-num);
   } else {
      unum = (uint32_t)num;
   }

   char digits[] = "0123456789abcdef";
   char upper_digits[] = "0123456789ABCDEF";
   char* digit_chars = uppercase ? upper_digits : digits;

   char temp[32];
   int32_t index = 0;

   while (unum > 0) {
      temp[index++] = digit_chars[unum % base];
      unum /= base;
   }

   int32_t final_index = 0;
   if (is_negative) {
      buffer[final_index++] = '-';
   }

   // reverse
   for (int32_t i = index - 1; i >= 0; i--) {
      buffer[final_index++] = temp[i];
   }

   buffer[final_index] = '\0';
   return final_index;
}


////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
int32_t puts(const char* str)
//------------------------------------------------------------------------------
{
   int16_t offset = get_cursor();
   int16_t row = offset / (2 * SCREEN_WIDTH);
   int16_t col = (offset / 2) % SCREEN_WIDTH;

   /* deprecated because the output is now printed at the cursor position
   int16_t row = find_output_row();
   // putint(row, 0, row);
   // int16_t col = 3;

   int16_t col = 0;
   */

   int32_t count = 0;
   while(*str)
   {
      if (*str == '\n') {
         col = 0;
         ++row;
         scroll_screen(&row);
      }
      else if (*str == '\r') {
         col = 0;
      }
      else if (*str == '\t') {
         col = (col + 4) & ~3;
         if (col >= SCREEN_WIDTH) {
            col = 0;
            ++row;
            scroll_screen(&row);
         }
      }
      else if (*str == '\b') {
         if (col > 0) {
            --col;
            putc(' ', col, row);
         }
         else if (row > 0) {
            --row;
            col = SCREEN_WIDTH - 1;
            putc(' ', col, row);
         }
      }
      else
      {
         putc(*str, col, row);
         ++col;

         if (col >= SCREEN_WIDTH) {
            col = 0;
            ++row;
            scroll_screen(&row);
         }
      }

      ++count;
      ++str;
   }

   set_cursor(row * SCREEN_WIDTH + col);
   return count; // return the number of characters printed
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


//------------------------------------------------------------------------------
int32_t printf(const char* format, ...)
//------------------------------------------------------------------------------
{
   va_list args;
   va_start(args, format);

   const char* p = format;
   int32_t chars_printed = 0;
   char buffer[64];

   while (*p) {
      if (*p == '%' && *(p + 1)) {
         p++;  // skip '%'

         switch (*p) {
            case 'c': {
               char c = (char)va_arg(args, int32_t);
               char arr[2] = {c, '\0'};
               puts(arr);
               chars_printed++;
               break;
            }
            case 's': {
               char* str = va_arg(args, char*);
               int32_t len = puts(str);
               chars_printed += len;
               break;
            }
            case 'd':
            case 'i': {
               int32_t num = va_arg(args, int32_t);
               int32_t len = int_to_string(num, buffer, 10, false);
               puts(buffer);
               chars_printed += len;
               break;
            }
            case 'u': {
               uint32_t num = va_arg(args, uint32_t);
               int32_t len = uint_to_string(num, buffer, 10, false);
               puts(buffer);
               chars_printed += len;
               break;
            }
            case 'x': {
               uint32_t num = va_arg(args, uint32_t);
               int32_t len = uint_to_string(num, buffer, 16, false);
               puts(buffer);
               chars_printed += len;
               break;
            }
            case 'X': {
               uint32_t num = va_arg(args, uint32_t);
               int32_t len = uint_to_string(num, buffer, 16, true);
               puts(buffer);
               chars_printed += len;
               break;
            }
            case 'o': {
               uint32_t num = va_arg(args, uint32_t);
               int32_t len = uint_to_string(num, buffer, 8, false);
               puts(buffer);
               chars_printed += len;
               break;
            }
            case 'p': {
               // pointer
               void* ptr = va_arg(args, void*);
               puts("0x");
               int32_t len = uint_to_string((uint32_t)ptr, buffer, 16, false);
               puts(buffer);
               chars_printed += len + 2;
               break;
            }
            case '%': {
               // character '%'
               char arr[2] = {'%', '\0'};
               puts(arr);
               chars_printed++;
               break;
            }
            default: {
               // unknown format specifier, print '%' and the character
               char arr[3] = {'%', *p, '\0'};
               puts(arr);
               chars_printed += 2;
               break;
            }
         }
      }
      else {
         char arr[2] = {*p, '\0'};
         puts(arr);
         chars_printed++;
      }

      p++;
   }

   va_end(args);
   return chars_printed; // return the number of characters printed
}

//------------------------------------------------------------------------------
int32_t sprintf(char* buffer, const char* format, ...)
//------------------------------------------------------------------------------
{
   va_list args;
   va_start(args, format);

   const char* p = format;
   char tmpbuf[64];
   int32_t chars_written = 0;

   while (*p) {
      if (*p == '%' && *(p + 1)) {
         p++;

         switch (*p) {
            case 'c': {
               char c = (char)va_arg(args, int32_t);
               buffer[chars_written++] = c;
               break;
            }
            case 's': {
               char* str = va_arg(args, char*);
               if (!str) str = "(null)";
               while (*str) {
                  buffer[chars_written++] = *str++;
               }
               break;
            }
            case 'd':
            case 'i': {
               int32_t num = va_arg(args, int32_t);
               int32_t len = int_to_string(num, tmpbuf, 10, false);
               for (int32_t i = 0; i < len; i++) {
                  buffer[chars_written++] = tmpbuf[i];
               }
               break;
            }
            // TODO: issue >> cause reboot even if not been called
            // case 'u': {
            //    uint32_t num = va_arg(args, uint32_t);
            //    int32_t len = uint_to_string(num, tmpbuf, 10, false);
            //    for (int32_t i = 0; i < len; i++) {
            //       if (tmpbuf[i] == '\0') break;
            //       buffer[chars_written++] = tmpbuf[i];
            //    }
            //    break;
            // }
            // case 'x': {
            //    uint32_t num = va_arg(args, uint32_t);
            //    int32_t len = uint_to_string(num, tmpbuf, 16, false);
            //    for (int32_t i = 0; i < len; i++) {
            //       buffer[chars_written++] = tmpbuf[i];
            //    }
            //    break;
            // }
            // case 'X': {
            //    uint32_t num = va_arg(args, uint32_t);
            //    int32_t len = uint_to_string(num, tmpbuf, 16, true);
            //    for (int32_t i = 0; i < len; i++) {
            //       buffer[chars_written++] = tmpbuf[i];
            //    }
            //    break;
            // }
            // case 'o': {
            //    uint32_t num = va_arg(args, uint32_t);
            //    int32_t len = uint_to_string(num, tmpbuf, 8, false);
            //    for (int32_t i = 0; i < len; i++) {
            //       buffer[chars_written++] = tmpbuf[i];
            //    }
            //    break;
            // }
            // case 'p': {
            //    void* ptr = va_arg(args, void*);
            //    buffer[chars_written++] = '0';
            //    buffer[chars_written++] = 'x';
            //    int32_t len = uint_to_string((uint32_t)ptr, tmpbuf, 16, false);
            //    for (int32_t i = 0; i < len; i++) {
            //       buffer[chars_written++] = tmpbuf[i];
            //    }
            //    break;
            // }
            case '%': {
               buffer[chars_written++] = '%';
               break;
            }
            default: {
               buffer[chars_written++] = '%';
               buffer[chars_written++] = *p;
               break;
            }
         }
      }
      else {
         buffer[chars_written++] = *p;
      }

      p++;
   }

   buffer[chars_written] = '\0';
   va_end(args);
   return chars_written;
}
