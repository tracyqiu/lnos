#include "paging.h"
#include "ktypes.h"
#include "memory.h"
#include "x86.h"
#include "gdt.h"
#include "idt.h"
#include "print.h"
#include "stdlib.h"
#include "string.h"


typedef uint32_t page_directory_t[PAGE_DIR_ENTRIES];
typedef uint32_t page_table_t[PAGE_TABLE_ENTRIES];

page_directory_t* page_directory_ptr = (page_directory_t*)PAGE_DIR_ADDR;
page_table_t* first_page_table_ptr = (page_table_t*)FIRST_PAGE_TABLE_ADDR;

extern void __attribute__((cdecl)) x86_jmp_to_high_virtual_addr();


typedef struct page_mapping {
    uint32_t virtual_addr;
    uint32_t physical_addr;
    struct page_mapping* next;
} page_mapping_t;

static page_mapping_t* page_mappings = NULL;


static uint32_t page_table_pool_bitmap[(PAGE_TABLE_POOL_SIZE + 31) / 32];
static uint32_t page_table_pool_initialized = 0;
extern uint32_t* page_bitmap; // defined in memory.c


//------------------------------------------------------------------------------
static int32_t add_page_mapping(uint32_t virtual_addr, uint32_t physical_addr)
//------------------------------------------------------------------------------
{
   page_mapping_t* mapping = (page_mapping_t*)malloc_physical_memory(sizeof(page_mapping_t));
    if (!mapping) {
        puts("ERROR: Failed to allocate memory for page mapping!\n");
        return 0;
    }
   mapping->virtual_addr = virtual_addr;
   mapping->physical_addr = physical_addr;
   mapping->next = page_mappings;
   page_mappings = mapping;

    return 1;
}

//------------------------------------------------------------------------------
static int32_t remove_page_mapping(uint32_t virtual_addr)
//------------------------------------------------------------------------------
{
   page_mapping_t* current = page_mappings;
   page_mapping_t* previous = NULL;

   while (current) {
      if (current->virtual_addr == virtual_addr) {
         if (previous) {
            previous->next = current->next;
         } else {
            page_mappings = current->next;
         }
         free_physical_memory((void*)(current));
         return 1;
      }
      previous = current;
      current = current->next;
   }
   return 0;
}

//------------------------------------------------------------------------------
static void init_page_table_pool()
//------------------------------------------------------------------------------
{
    if (page_table_pool_initialized) return;

    // clear the page table pool bitmap
    for (uint32_t i = 0; i < (PAGE_TABLE_POOL_SIZE + 31) / 32; i++) {
        page_table_pool_bitmap[i] = 0;
    }

    // set the page table pool bitmap to used
    uint32_t pool_start_page = PAGE_TABLE_POOL_START / PAGE_SIZE;
    for (uint32_t i = 0; i < PAGE_TABLE_POOL_SIZE; i++) {
        set_page_bitmap_used(page_bitmap, pool_start_page + i);
    }

    page_table_pool_initialized = 1;
}

//------------------------------------------------------------------------------
static void* allocate_page_table()
//------------------------------------------------------------------------------
{
    if (!page_table_pool_initialized) {
        init_page_table_pool();
    }

    // find a free page table slot in the pool
    for (uint32_t i = 0; i < PAGE_TABLE_POOL_SIZE; i++) {
        uint32_t word_idx = i / 32;
        uint32_t bit_idx = i % 32;

        if (!(page_table_pool_bitmap[word_idx] & (1 << bit_idx))) {
            // mark the page table slot as used
            page_table_pool_bitmap[word_idx] |= (1 << bit_idx);

            // calculate address and clear the page table memory
            void* page_table_addr = (void*)(PAGE_TABLE_POOL_START + i * PAGE_SIZE);
            memset(page_table_addr, 0, PAGE_SIZE);

            return page_table_addr;
        }
    }

    return NULL; // no free page table slots available
}

//------------------------------------------------------------------------------
static void free_page_table(void* page_table_addr)
//------------------------------------------------------------------------------
{
    uint32_t addr = (uint32_t)page_table_addr;
    if (addr < PAGE_TABLE_POOL_START ||
        addr >= PAGE_TABLE_POOL_START + PAGE_TABLE_POOL_SIZE * PAGE_SIZE) {
        return;
    }

    // check if the address is aligned to PAGE_SIZE
    if (addr & (PAGE_SIZE - 1)) {
        return;
    }

    uint32_t index = (addr - PAGE_TABLE_POOL_START) / PAGE_SIZE;
    if (index >= PAGE_TABLE_POOL_SIZE) {
        return;
    }

    // clear the page table slots as free
    uint32_t word_idx = index / 32;
    uint32_t bit_idx = index % 32;

    page_table_pool_bitmap[word_idx] &= ~(1 << bit_idx);
}

//------------------------------------------------------------------------------
/*static*/ void get_page_table_pool_status(uint32_t* used, uint32_t* total)
//------------------------------------------------------------------------------
{
    *total = PAGE_TABLE_POOL_SIZE;
    *used = 0;

    for (uint32_t i = 0; i < PAGE_TABLE_POOL_SIZE; i++) {
        uint32_t word_idx = i / 32;
        uint32_t bit_idx = i % 32;

        if (page_table_pool_bitmap[word_idx] & (1 << bit_idx)) {
            (*used)++;
        }
    }
}

//------------------------------------------------------------------------------
void init_paging()
//------------------------------------------------------------------------------
{
   // step 1: init page table
   for (uint32_t i = 0; i < PAGE_DIR_ENTRIES; i++) {
      (*page_directory_ptr)[i] = NULL;
   }

   for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
      (*first_page_table_ptr)[i] = NULL;
   }

   (*page_directory_ptr)[0] = (uint32_t)first_page_table_ptr | PAGE_PRESENT | PAGE_WRITE;
   // prepare to map physical address 0-4M to high virtual address 0xC0000000, pde_index = 0xC0000000 >> 22 = 768
   (*page_directory_ptr)[768] = (*page_directory_ptr)[0];

   // map 4M virtual address[0xC0000000, 0xC0400000) to physical address[0x0, 0x400000)
   for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
      // map_physical_page(0xC0000000 + i * 0x1000, i * 0x1000, PAGE_PRESENT | PAGE_WRITE);
      (*first_page_table_ptr)[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITE;
   }

   // step 2: put page directory address to cr3 register
   asm volatile("mov %0, %%cr3" : : "r" (PAGE_DIR_ADDR));

   // step 3: enable paging
   uint32_t cr0;
   asm volatile (
       "mov %%cr0, %0\n\t"
       "or $0x80000000, %0\n\t"
       "mov %0, %%cr0"
       : "=r" (cr0)
   );

   // step 4: jmp to high address
   // uint32_t high_addr_offset = KERNEL_STARTED_VIRTUAL_ADDRESS;
   // asm volatile (
   //    "lea 1f, %%eax\n\t"        // calculate the address of label 1:, it's current code address offset
   //    "add %0, %%eax\n\t"
   //    "jmp *%%eax\n\t"           // jmp to high address
   //    "1:\n\t"
   //    "add %0, %%esp\n\t"        // update esp and ebp
   //    "add %0, %%ebp\n\t"
   //    : : "r" (high_addr_offset) : "eax"
   // );

   // try to implement jmp to high address by nasm
   x86_jmp_to_high_virtual_addr();
}

//------------------------------------------------------------------------------
void remove_first_page_table_mapping()
//------------------------------------------------------------------------------
{
   (*page_directory_ptr)[0] = NULL;
   asm volatile("mov %cr3, %eax; mov %eax, %cr3");
}

//------------------------------------------------------------------------------
void map_physical_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags)
//------------------------------------------------------------------------------
{
   add_page_mapping(virtual_addr, physical_addr);

   uint32_t pde_index = virtual_addr >> 22;           // bit 31-22
   uint32_t pte_index = (virtual_addr >> 12) & 0x3FF; // bit 21-12, 0b1111111111 == 0x3FF

   if (!((*page_directory_ptr)[pde_index] & PAGE_PRESENT)) {
      // MMU(Memory Management Unit) performs address translation on a page-by-page basis
      // page_table_t* new_pde = (page_table_t*)malloc_physical_memory(sizeof(page_table_t));
      // for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
      //    (*new_pde)[i] = NULL;
      // }
      page_table_t* new_pde = (page_table_t*)allocate_page_table();
      if (!new_pde) {
         puts("ERROR: Page table pool exhausted!\n");
         return;
      }

      (*page_directory_ptr)[pde_index] = (uint32_t)new_pde | PAGE_PRESENT | PAGE_WRITE;

      #if DEF_DEBUG_TRACE
      uint32_t used, total;
      get_page_table_pool_status(&used, &total);

      puts("Allocated page table ");
      char buf[16];
      puts(itoa(used, buf, 10));
      puts("/");
      puts(itoa(total, buf, 10));
      puts(" for PDE[");
      puts(itoa(pde_index, buf, 10));
      puts("]\n");
      #endif
   }

   uint32_t pde_addr = (*page_directory_ptr)[pde_index] & 0xFFFFF000;
   page_table_t* pt = (page_table_t*)pde_addr;

   (*pt)[pte_index] = (physical_addr & 0xFFFFF000) | flags;

   // update TLB
   asm volatile("invlpg (%0)" : : "r" (virtual_addr) : "memory");
}

//------------------------------------------------------------------------------
void unmap_physical_page(uint32_t virtual_addr)
//------------------------------------------------------------------------------
{
   remove_page_mapping(virtual_addr);

   uint32_t pde_index = virtual_addr >> 22;
   uint32_t pte_index = (virtual_addr >> 12) & 0x3FF;

   if ((*page_directory_ptr)[pde_index] & PAGE_PRESENT) {
      uint32_t pde_addr = (*page_directory_ptr)[pde_index] & 0xFFFFF000;
      page_table_t* pt = (page_table_t*) pde_addr;
      (*pt)[pte_index] = NULL;
      free_page_table((void*)virtual_addr);

      // update TLB
      asm volatile("invlpg (%0)" : : "r" (virtual_addr) : "memory");
   }
}

//------------------------------------------------------------------------------
uint32_t get_physical_address(uint32_t virtual_addr)
//------------------------------------------------------------------------------
{
   uint32_t pde_index = virtual_addr >> 22;
   uint32_t pte_index = (virtual_addr >> 12) & 0x3FF;

   if (!((*page_directory_ptr)[pde_index] & PAGE_PRESENT)) {
      return NULL;
   }

   uint32_t pde_addr = (*page_directory_ptr)[pde_index] & 0xFFFFF000;
   page_table_t* pt = (page_table_t*)pde_addr;

   if (!((*pt)[pte_index] & PAGE_PRESENT)){
      return NULL;
   }

   uint32_t offset = virtual_addr & 0xFFF;  // bit 11-0
   return ((*pt)[pte_index] & 0xFFFFF000) | offset;
}

//------------------------------------------------------------------------------
uint32_t get_physical_address_from_mapping(uint32_t virtual_addr)
//------------------------------------------------------------------------------
{
    page_mapping_t* current = page_mappings;
    while (current) {
        if (current->virtual_addr == virtual_addr) {
            return current->physical_addr;
        }
        current = current->next;
    }

    uint32_t page_base = virtual_addr & 0xFFFFF000;
    current = page_mappings;
    while (current) {
        if ((current->virtual_addr & 0xFFFFF000) == page_base) {
            uint32_t offset = virtual_addr - current->virtual_addr;
            return current->physical_addr + offset;
        }
        current = current->next;
    }

    return NULL;
}