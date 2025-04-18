#ifndef MEMORY_H
#define MEMORY_H

#include "stdint.h"


void init_memory();

void* get_free_page();

void free_page(void* page_addr);

void* malloc_physical_memory(uint32_t size);

void free_physical_memory(void* addr);

void* malloc_aligned_physical_memory(uint32_t size, uint32_t alignment);

void free_aligned_physical_memory(void* aligned_addr);

uint32_t get_allocated_physical_size(void* physical_addr);


#endif