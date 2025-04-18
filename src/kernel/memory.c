#include "memory.h"
#include "print.h"
#include "string.h"
#include "stdlib.h"


#define LOW_MEM   0x100000
#define PAGE_SIZE 4096

// Address Range Descriptor Structure
typedef struct {
   uint64_t base_addr;
   uint64_t length;                 // length in bytes
   uint32_t type;
   uint32_t extended_attributes;
} addr_range_entry_t;


typedef struct {
   uint32_t lower_mm_size;           // lower memory size(KB)
   uint32_t high_mm_size;           // higher memory size(KB)
   uint32_t map_size;
   addr_range_entry_t *map_addr;
} memory_info_t;

typedef enum {
   MEMORY_AVAILABLE      = 1,
   MEMORY_RESERVED       = 2,
   MEMORY_ACPI_RECLAIM   = 3,
   MEMORY_NVS            = 4,
   MEMORY_BADRAM         = 5,
} MEMORY_TYPE;


static memory_info_t memory_info = {
   .lower_mm_size = 0,
   .high_mm_size = 0,
   .map_size = 0,
   .map_addr = (addr_range_entry_t*)0x8000
};

static uint32_t* page_bitmap;
static uint32_t page_count;

static uint32_t heap_start;


//------------------------------------------------------------------------------
static void load_memory_info()
//------------------------------------------------------------------------------
{
   memory_info.map_addr = (addr_range_entry_t*)(*((uint32_t*)0x7000));
   memory_info.map_size = *((uint32_t*)0x7004);

   addr_range_entry_t *entry = memory_info.map_addr;
   for (uint32_t i = 0; i < memory_info.map_size / sizeof(addr_range_entry_t); i++) {
      if (entry[i].type == MEMORY_AVAILABLE) {
         if (entry[i].base_addr < LOW_MEM) {
            memory_info.lower_mm_size = (entry[i].base_addr + entry[i].length) >> 12;  // 1 KB = 1024 Bytes
         }
         else {
            memory_info.high_mm_size += entry[i].length >> 12;
         }
      }
   }
}

//------------------------------------------------------------------------------
static void set_page_bitmap_used(uint32_t *bitmap, uint32_t bit)
//------------------------------------------------------------------------------
{
   bitmap[bit / 32] |= (1 << (bit % 32));
}

//------------------------------------------------------------------------------
static void clear_page_bitmap(uint32_t *bitmap, uint32_t bit)
//------------------------------------------------------------------------------
{
   bitmap[bit / 32] &= ~(1 << (bit % 32));
}

//------------------------------------------------------------------------------
static uint32_t is_free_page(uint32_t *bitmap, uint32_t bit)
//------------------------------------------------------------------------------
{
   return ~(bitmap[bit / 32] & (1 << (bit % 32)));
}

//------------------------------------------------------------------------------
void init_memory()
//------------------------------------------------------------------------------
{
   load_memory_info();
   // puts("Loaded memory infor...\n");

   uint32_t available_mm_size = memory_info.high_mm_size * 1024;
   page_count = available_mm_size / PAGE_SIZE;

   // calculate how many bytes are needed to represent if these page is used or not, 1byte = 8bit
   uint32_t bitmap_bytes = page_count / 8;
   if (page_count % 8) bitmap_bytes++;
   // calculate the number of uint32_t required for the bitmap and round it up to the nearest integer
   uint32_t bitmap_uint32_count = (bitmap_bytes + 3) / 4;

   // put the page bitmap address behind idt, idt_addr_end is 0x102800
   page_bitmap = (uint32_t*)0x103000;
   // clear bitmap
   for (uint32_t i = 0; i < bitmap_uint32_count; i++) {
      page_bitmap[i] = 0;
   }

   // set the memory of kernel and page_bitmap as used
   uint32_t kernel_mm_end = (uint32_t)page_bitmap + bitmap_bytes;
   uint32_t kernel_page_count = (kernel_mm_end / PAGE_SIZE) + 1;
   for (uint32_t i = 0; i < kernel_page_count; i++) {
      set_page_bitmap_used(page_bitmap, i);
   }

   // set the memory of unavailable as used
   addr_range_entry_t *entry = memory_info.map_addr;
   for (uint32_t i = 0; i < memory_info.map_size / sizeof(addr_range_entry_t); i++) {
      if (entry[i].type != MEMORY_AVAILABLE) {
         uint64_t start_offset = entry[i].base_addr / PAGE_SIZE;
         // round it up to the nearest integer as a safety guarantee
         uint64_t end_offset = (entry[i].base_addr + entry[i].length + (PAGE_SIZE-1)) / PAGE_SIZE;

         for (uint64_t j = start_offset; j < end_offset && j < page_count; j++) {
            set_page_bitmap_used(page_bitmap, (uint32_t)j);
         }
      }
   }

   heap_start = kernel_mm_end;

   puts("Available memory: ");
   char buf[16];
   puts(itoa(available_mm_size / 1024, buf, 10));
   puts(" KB, total page count: ");
   puts(itoa(page_count, buf, 10));
   puts("\n");
   // char str[64];
   // strcat(str, itoa(page_count, buf, 10));
   // strcat(str, ".\n");
   // puts(str);
}

//------------------------------------------------------------------------------
void* get_free_page()
//------------------------------------------------------------------------------
{
   for (uint32_t i = 0; i < page_count; i++) {
      if (is_free_page(page_bitmap, i)) {
         set_page_bitmap_used(page_bitmap, i);
         return (void*)(i * PAGE_SIZE);
      }
   }
   return NULL;
}

//------------------------------------------------------------------------------
void free_page(void* page_addr)
//------------------------------------------------------------------------------
{
   uint32_t page_offset = (uint32_t)page_addr / PAGE_SIZE;
   clear_page_bitmap(page_bitmap, page_offset);
}


////////////////////////////////////////////////////////////////////////////////


typedef struct heap_block {
   uint32_t size;
   uint8_t is_free;
   struct heap_block *next;
} heap_block_t;

static heap_block_t *heap_start_block_list = NULL;

//------------------------------------------------------------------------------
static void init_heap()
//------------------------------------------------------------------------------
{
   heap_start_block_list = (heap_block_t*)heap_start;
   heap_start_block_list->size = PAGE_SIZE - sizeof(heap_block_t);
   heap_start_block_list->is_free = 1;
   heap_start_block_list->next = NULL;
}

//------------------------------------------------------------------------------
static heap_block_t* kmalloc_on_page(uint32_t size)
//------------------------------------------------------------------------------
{
   // + (PAGE_SIZE - 1) : if less than one page then counted as one page.
   uint32_t pages_needed = (size + sizeof(heap_block_t) + (PAGE_SIZE - 1)) / PAGE_SIZE;

   uint32_t page_start = 0;
   uint32_t consecutive_free_page = 0;

   // try to find consecutive free pages to fit the needed size
   for (uint32_t i = 0; i < page_count; i++) {
      if (is_free_page(page_bitmap, i)) {
         if (0 == page_start) page_start = i;
         consecutive_free_page++;

         if (consecutive_free_page >= pages_needed) {
            for (uint32_t j = page_start; j < page_start + pages_needed; j++) {
               set_page_bitmap_used(page_bitmap, j);
            }

            // create a block
            heap_block_t* new_block = (heap_block_t*)(page_start * PAGE_SIZE);
            new_block->size = pages_needed * PAGE_SIZE - sizeof(heap_block_t);
            new_block->is_free = 1;
            new_block->next = NULL;

            // add new_block to list
            heap_block_t* current = heap_start_block_list;
            while (current->next) {
               current = current->next;
            }
            current->next = new_block;

            return new_block;
         }
      }
      else {
         page_start = 0;
         consecutive_free_page = 0;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------------
void* malloc_physical_memory(uint32_t size)
//------------------------------------------------------------------------------
{
   if (!heap_start_block_list) {
      init_heap();
   }

   heap_block_t *current = heap_start_block_list;
   heap_block_t *best_fit = NULL;
   uint32_t best_size = 0xFFFFFFFF;

   while (current) {
      if (current->is_free && current->size >= size) {
         if (current->size < best_size) {
            best_size = current->size;
            best_fit = current;
         }
      }
      current = current->next;
   }

   if (!best_fit) {
      best_fit = kmalloc_on_page(size);
   }

   // split memory of current founded page item
   // + 4 : 4 bytes available at least and leave some margin to serve as a safety guarantee.
   if (best_fit->size >= size + sizeof(heap_block_t) + 4) {
      heap_block_t *new_block = (heap_block_t*)((uint32_t)best_fit + sizeof(heap_block_t) + size);
      new_block->size = best_fit->size - size - sizeof(heap_block_t);
      new_block->is_free = 1;
      new_block->next = best_fit->next;

      best_fit->size = size;
      best_fit->next = new_block;
   }

   best_fit->is_free = 0;
   return (void*)((uint32_t)best_fit + sizeof(heap_block_t));
}

//------------------------------------------------------------------------------
void free_physical_memory(void* ptr)
//------------------------------------------------------------------------------
{
   if (!ptr) return;

   // mark block as free, the block address is moving forward from ptr by sizeof(heap_block_t)
   heap_block_t *block = (heap_block_t*)((uint32_t)ptr - sizeof(heap_block_t));
   block->is_free = 1;

   // merge free blocks
   heap_block_t *current = heap_start_block_list;
   while (current && current->next) {
      if (current->is_free && current->next->is_free) {
         current->size += sizeof(heap_block_t) + current->next->size;
         current->next = current->next->next;
      }
      else {
         current = current->next;
      }
   }
}

//------------------------------------------------------------------------------
void* malloc_aligned_physical_memory(uint32_t size, uint32_t alignment)
//------------------------------------------------------------------------------
{
   // align to multiples of 2
   if (alignment & (alignment - 1)) return NULL;

   // add sizeof(uint32_t) to store the physical_addr get from function malloc_physical_memory()
   uint32_t size_needed = size + alignment + sizeof(uint32_t);
   uint32_t physical_addr = (uint32_t)malloc_physical_memory(size_needed);
   if (!physical_addr) return NULL;

   // physical_addr will be placed before the aligned_addr, so + sizeof(uint32_t)
   uint32_t addr = physical_addr + sizeof(uint32_t);
   uint32_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);

   *((uint32_t*)(aligned_addr - sizeof(uint32_t))) = physical_addr;

   return (void*)aligned_addr;
}

//------------------------------------------------------------------------------
void free_aligned_physical_memory(void* aligned_addr)
//------------------------------------------------------------------------------
{
   if (!aligned_addr) return;

   void* physical_addr = (void*)((uint32_t)aligned_addr - sizeof(uint32_t));
   free_physical_memory(physical_addr);
}

//------------------------------------------------------------------------------
uint32_t get_allocated_physical_size(void* addr)
//------------------------------------------------------------------------------
{
   if (!addr) return 0;

   heap_block_t *block = (heap_block_t*)((uint32_t)addr - sizeof(heap_block_t));
   return block->size;
}


