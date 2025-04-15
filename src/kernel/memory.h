#ifndef MEMORY_H
#define MEMORY_H

#include "stdint.h"


void init_memory();

void* get_free_page();

void free_page(void* page_addr);

void* kernel_malloc(uint32_t size);

void kernel_free(void* ptr);

#endif