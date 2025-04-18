#include "mm.h"
#include "memory.h"
#include "paging.h"
#include "ktypes.h"

#define VIRTUAL_HEAP_START 0xD0000000
#define VIRTUAL_HEAP_SIZE  0x10000000

//------------------------------------------------------------------------------
// When allocating virtual memory, it is done on a page-by-page basis. If the memory requirement is less than one page, it will be allocated as one page.
// This is because the MMU(Memory Management Unit) performs address translation on a page-by-page basis and cannot achieve finer-grained mapping within a page.
void* malloc_virtual_memory(uint32_t size)
//------------------------------------------------------------------------------
{
   void* physical_addr = malloc_physical_memory(size);
   if (!physical_addr) return NULL;

   static uint32_t next_virtual_addr = VIRTUAL_HEAP_START;
   uint32_t virtual_addr = next_virtual_addr;
   // round it up to 4k to allocate page-by-page, == ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE
   next_virtual_addr += (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

   for (uint32_t i = 0; i < size; i += PAGE_SIZE) {
      uint32_t phy_addr = (uint32_t)physical_addr + i;
      uint32_t vir_addr = virtual_addr + i;

      map_physical_page(vir_addr, phy_addr, PAGE_PRESENT | PAGE_WRITE);
   }

   return (void*)virtual_addr;
}


//------------------------------------------------------------------------------
void free_virtual_memory(void* virtual_addr)
//------------------------------------------------------------------------------
{
   if (!virtual_addr) return;

   void* physical_addr = (void*)get_physical_address((uint32_t)virtual_addr);
   if (!physical_addr) return;

   uint32_t size = get_allocated_physical_size(physical_addr);
   free_physical_memory(physical_addr);

   for (uint32_t i = 0; i < size; i += PAGE_SIZE) {
      uint32_t vir_addr = (uint32_t)virtual_addr + i;

      unmap_physical_page(vir_addr);
   }
}