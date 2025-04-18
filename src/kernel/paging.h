#ifndef PAGING_H
#define PAGING_H

#include "stdint.h"

void init_paging();

void map_physical_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);

void unmap_physical_page(uint32_t virtual_addr);

uint32_t get_physical_address(uint32_t virtual_addr);

void remove_first_page_table_mapping();


#endif