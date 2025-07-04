#ifndef PRINT_H_
#define PRINT_H_

#include "stdint.h"

int32_t puts(const char* str);

void clear_screen();

int32_t printf(const char* format, ...);

int32_t sprintf(char* buffer, const char* format, ...);

#endif //PRINT_H_