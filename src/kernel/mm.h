#ifndef MM_H
#define MM_H

#include "stdint.h"

void* malloc_virtual_memory(uint32_t size);

void free_virtual_memory(void* addr);

#endif