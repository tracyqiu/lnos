#include "paging.h"
#include "ktypes.h"
#include "memory.h"
#include "x86.h"
#include "gdt.h"
#include "idt.h"


#define PAGE_DIR_ENTRIES      1024
#define PAGE_TABLE_ENTRIES    1024

#define PAGE_DIR_ADDR         0x200000
#define FIRST_PAGE_TABLE_ADDR 0x201000



typedef uint32_t page_directory_t[PAGE_DIR_ENTRIES];
typedef uint32_t page_table_t[PAGE_TABLE_ENTRIES];

page_directory_t* page_directory_ptr = (page_directory_t*)PAGE_DIR_ADDR;
page_table_t* first_page_table_ptr = (page_table_t*)FIRST_PAGE_TABLE_ADDR;

extern void __attribute__((cdecl)) x86_jmp_to_high_virtual_addr();


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
   uint32_t pde_index = virtual_addr >> 22;           // bit 31-22
   uint32_t pte_index = (virtual_addr >> 12) & 0x3FF; // bit 21-12, 0b1111111111 == 0x3FF

   if (!((*page_directory_ptr)[pde_index] & PAGE_PRESENT)) {
      // MMU(Memory Management Unit) performs address translation on a page-by-page basis
      page_table_t* new_pde = (page_table_t*)malloc_aligned_physical_memory(sizeof(page_table_t), PAGE_SIZE);
      for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
         (*new_pde)[i] = NULL;
      }

      (*page_directory_ptr)[pde_index] = (uint32_t)new_pde | PAGE_PRESENT | PAGE_WRITE;
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
   uint32_t pde_index = virtual_addr >> 22;
   uint32_t pte_index = (virtual_addr >> 12) & 0x3FF;

   if ((*page_directory_ptr)[pde_index] & PAGE_PRESENT) {
      uint32_t pde_addr = (*page_directory_ptr)[pde_index] & 0xFFFFF000;
      page_table_t* pt = (page_table_t*) pde_addr;
      (*pt)[pte_index] = NULL;

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