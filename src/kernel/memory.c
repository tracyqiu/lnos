#include "memory.h"
#include "print.h"
#include "string.h"
#include "stdlib.h"
#include "ktypes.h"


#define LOW_MEM   0x100000

// Address Range Descriptor Structure
typedef struct {
   uint64_t base_addr;
   uint64_t length;                 // length in bytes
   uint32_t type;
   uint32_t extended_attributes;
} addr_range_entry_t;


typedef struct {
   uint32_t lower_mm_size;          // lower memory size(KB)
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
static uint32_t page_count;         // the total page count, include all memory
static uint32_t max_memory_addr;
static uint32_t heap_start;
static uint32_t heap_start_page_idx;

//------------------------------------------------------------------------------
static void debug_raw_memory_data()
//------------------------------------------------------------------------------
{
   #if DEF_DEBUG_TRACE
   puts("=== START RAW MEMORY DEBUG ===\n");

   uint32_t* raw_data = (uint32_t*)0x8000;
   uint32_t len = *((uint32_t*)0x7004);

   puts("Memory map size: ");
   char buf[16];
   puts(itoa(len, buf, 10));
   puts(" bytes\n");

   uint32_t max_entries = len / 4;
   puts("Raw data (hex):\n");
   // print 4 bytes each time
   for (uint32_t i = 0; (i < max_entries) && ((i * 4) < len); i++) {
      puts("Offset ");
      puts(itoa(i * 4, buf, 10));
      puts(": ");
      puts(itoa(raw_data[i], buf, 16));
      puts("\n");
   }

   puts("=== END RAW MEMORY DEBUG ===\n");
   #endif
}

//------------------------------------------------------------------------------
static void load_memory_info()
//------------------------------------------------------------------------------
{
   debug_raw_memory_data();

   memory_info.map_addr = (addr_range_entry_t*)(*((uint32_t*)0x7000));
   memory_info.map_size = *((uint32_t*)0x7004);

   addr_range_entry_t *entry = memory_info.map_addr;
   uint32_t len = memory_info.map_size / sizeof(addr_range_entry_t);
   for (uint32_t i = 0; i < len; i++) {
      uint64_t end_addr = entry[i].base_addr + entry[i].length;
      if (end_addr > max_memory_addr) {
         max_memory_addr = (uint32_t)end_addr;
      }

      if (entry[i].length > 0) {
         char buf[16];
         puts("memory addr: ");
         puts(itoa(entry[i].base_addr, buf, 16));
         puts(" length: ");
         puts(itoa(entry[i].length >> 10, buf, 10));
         puts("KB");
      }

      if (entry[i].type == MEMORY_AVAILABLE) {
         if (entry[i].base_addr < LOW_MEM) {
            memory_info.lower_mm_size = (entry[i].base_addr + entry[i].length) >> 10;  // 1 KB = 1024 Bytes
         }
         else {
            memory_info.high_mm_size += entry[i].length >> 10;

            if (entry[i].length > 0) puts(" available");
         }
      }
      if (entry[i].length > 0) puts(" \n");
   }
}

//------------------------------------------------------------------------------
static void set_page_bitmap_used(uint32_t *bitmap, uint32_t bit)
//------------------------------------------------------------------------------
{
   if (bit >= page_count) return;
   bitmap[bit / 32] |= (1 << (bit % 32));
}

//------------------------------------------------------------------------------
static void clear_page_bitmap(uint32_t *bitmap, uint32_t bit)
//------------------------------------------------------------------------------
{
   if (bit >= page_count) return;
   bitmap[bit / 32] &= ~(1 << (bit % 32));
}

//------------------------------------------------------------------------------
static uint32_t is_free_page(uint32_t *bitmap, uint32_t bit)
//------------------------------------------------------------------------------
{
   if (bit >= page_count) return 0;
   return !(bitmap[bit / 32] & (1 << (bit % 32)));
}

//------------------------------------------------------------------------------
static uint32_t is_page_available(uint32_t page_index)
//------------------------------------------------------------------------------
{
   uint32_t addr = page_index * PAGE_SIZE;
   addr_range_entry_t* entry = memory_info.map_addr;
   uint32_t len = memory_info.map_size / sizeof(addr_range_entry_t);

   for (uint32_t i = 0; i < len; i++) {
      if (entry[i].type == MEMORY_AVAILABLE
         && addr >= entry[i].base_addr
         && addr < (entry[i].base_addr + entry[i].length)) {
         return 1;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
void init_memory()
//------------------------------------------------------------------------------
{
   load_memory_info();
   // puts("Loaded memory infor...\n");

   page_count = (max_memory_addr + PAGE_SIZE - 1) / PAGE_SIZE;
   // calculate how many bytes are needed to represent if these page is used or not and round it up, 1byte = 8bit
   uint32_t bitmap_bytes = (page_count + 7) / 8;
   // calculate the number of uint32_t required for the bitmap and round it up to the nearest integer
   uint32_t bitmap_uint32_count = (bitmap_bytes + 3) / 4;

   // put the page bitmap address behind idt, idt_addr_end is 0x102800
   page_bitmap = (uint32_t*)0x103000;
   // set all memory as used
   for (uint32_t i = 0; i < bitmap_uint32_count; i++) {
      page_bitmap[i] = 0xFFFFFFFF;
   }

   // set memory of available as free
   addr_range_entry_t* entry = memory_info.map_addr;
   uint32_t len = memory_info.map_size / sizeof(addr_range_entry_t);
   for (uint32_t i = 0; i < len; i++) {
      if (entry[i].type == MEMORY_AVAILABLE) {
         uint32_t start_page_idx = entry[i].base_addr / PAGE_SIZE;
         uint32_t end_page_idx = (entry[i].base_addr + entry[i].length) / PAGE_SIZE;

         for (uint32_t j = start_page_idx; j < end_page_idx; j++) {
            clear_page_bitmap(page_bitmap, j);
         }
      }
   }

   // // set the memory of unavailable as used
   // addr_range_entry_t *entry = memory_info.map_addr;
   // for (uint32_t i = 0; i < memory_info.map_size / sizeof(addr_range_entry_t); i++) {
   //    if (entry[i].type != MEMORY_AVAILABLE) {
   //       uint64_t start_offset = entry[i].base_addr / PAGE_SIZE;
   //       // round it up to the nearest integer as a safety guarantee
   //       uint64_t end_offset = (entry[i].base_addr + entry[i].length + (PAGE_SIZE-1)) / PAGE_SIZE;

   //       for (uint64_t j = start_offset; j < end_offset && j < page_count; j++) {
   //          set_page_bitmap_used(page_bitmap, (uint32_t)j);
   //       }
   //    }
   // }

   // set the memory of kernel and page_bitmap as used
   uint32_t kernel_mm_end = (uint32_t)page_bitmap + bitmap_bytes;
   uint32_t kernel_page_count = (kernel_mm_end / PAGE_SIZE) + 1;
   for (uint32_t i = 0; i < kernel_page_count; i++) {
      set_page_bitmap_used(page_bitmap, i);
   }

   heap_start = kernel_page_count * PAGE_SIZE;
   heap_start_page_idx = kernel_page_count;

   uint32_t available_pages = 0;
   for (uint32_t i = 0; i < page_count; i++) {
      if (is_free_page(page_bitmap, i)) available_pages++;
   }

   puts("Available memory: ");
   char buf[16];
   puts(itoa((available_pages * PAGE_SIZE) / 1024, buf, 10));
   puts(" KB, total page count: ");
   puts(itoa(page_count, buf, 10));
   puts(", kernel pages: ");
   puts(itoa(kernel_page_count, buf, 10));
   puts(", available pages: ");
   puts(itoa(available_pages, buf, 10));
   puts(", bitmap uint32 count: ");
   puts(itoa(bitmap_uint32_count, buf, 10));
   puts("\n");
}

//------------------------------------------------------------------------------
void* get_free_page()
//------------------------------------------------------------------------------
{
   for (uint32_t i = 0; i < page_count; i++) {
      if (is_free_page(page_bitmap, i)) { //  && is_page_available(i)
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
   if (page_offset < page_count && is_page_available(page_offset)) {
      clear_page_bitmap(page_bitmap, page_offset);
   }
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
   for (uint32_t i = heap_start_page_idx; i < page_count; i++) {
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
            if (heap_start_block_list != new_block) {
               heap_block_t* current = heap_start_block_list;
               while (current->next) {
                  current = current->next;
               }
               current->next = new_block;
            }
            else {
               heap_start_block_list = new_block;
            }

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


