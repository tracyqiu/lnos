#include "stdio.h"
//#include "x86.h"

// vga address
#define VGA_STARTED_MEMORY_ADDR 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 160

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

int find_last_text_row();
void putc(char c, int col, int row);


void puts(const char* str)
{
    int row = find_last_text_row();
    int col = 0;
    while(*str)
    {
        putc(*str, col, row);
        ++str;
        ++col;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

int find_last_text_row() {
    unsigned short *output_addr = (unsigned short*)VGA_STARTED_MEMORY_ADDR;
    
    // find first empty row
    for (int row = SCREEN_HEIGHT - 1; row >= 0; row--) {
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            int offset = row * SCREEN_WIDTH + col;
            // if has valid character, return the next row
            if ((output_addr[offset] & 0xFF) != ' ' && (output_addr[offset] & 0xFF) != 0) {
                return row + 1;
            }
        }
    }
    
    return 1; // if not found, return the first row
}

void putc(char c, int col, int row)
{
    // x86_output_char(c,0);
    unsigned short *output_addr = (unsigned short*)VGA_STARTED_MEMORY_ADDR;
    int offset = row * SCREEN_WIDTH + col;
    output_addr[offset] = (unsigned short)c | ((unsigned short)COLOR_LIGHT_GRAY << 8);
}