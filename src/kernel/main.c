#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "keyboard.h"
#include "stdlib.h"
#include "timer.h"
#include "paging.h"
#include "mm.h"
#include "task.h"
#include "schedule.h"


//------------------------------------------------------------------------------
static void test_allocate_physical_memory()
//------------------------------------------------------------------------------
{
   puts("Memory allocation test: ");
   void* mm0 = malloc_physical_memory(9192);
   void* mm1 = malloc_physical_memory(2048);
   if (mm1 && mm0) {
      puts("addr0(8KB)=");
      char buf[32];
      puts(itoa((int32_t)mm0, buf, 16));

      puts(", addr1(2KB)=");
      puts(itoa((int32_t)mm1, buf, 16));
      puts(" >>> SUCCESS :)\n");
   }
   else {
      puts("FAILED :(\n");
   }
   free_physical_memory(mm1);
   free_physical_memory(mm0);
}

static void test_malloc_virtual_memory()
{
   puts("Virtual mm allocation test: ");
   void* mm0 = malloc_virtual_memory(9192);
   void* mm1 = malloc_virtual_memory(2048);
   if (mm1 && mm0) {
      puts("addr0(8KB)=");
      char buf[32];
      puts(itoa((int32_t)mm0, buf, 16));

      puts(", addr1(2KB)=");
      puts(itoa((int32_t)mm1, buf, 16));
      puts(" >>> SUCCESS :)\n");
   }
   else {
      puts("FAILED :(\n");
   }
   free_virtual_memory(mm1);
   free_virtual_memory(mm0);
}

static void test_task_1() {
   int32_t count = 0;
   while(1) {
      if ((++count) % 10 == 0) {
         puts("Task 1 running...\n");
      }
      for(int32_t i = 0; i < 100000; i++) {
         asm volatile("nop");
      }
   }
}

static void test_task_2() {
   int32_t count = 0;
   while(1) {
      if ((++count) % 1000 == 0) {
         puts("Task 2 running...\n");
      }
      for(int32_t i = 0; i < 100000; i++) {
         asm volatile("nop");
      }
   }
}

//------------------------------------------------------------------------------
// Specify the section name.
// Otherwise, combining it with the setting in the makefile will prevent other functions from being defined before this one.
__attribute__((section(".text.c_start"))) void cstart(void)
//------------------------------------------------------------------------------
{
   // clear_screen();
   puts("Welcome to roqiu's LnOS demo :)!\n");

   init_gdt();
   puts("GDT initialized.\n");

   init_idt();
   puts("IDT initialized.\n");

   init_memory();
   puts("Memory initialized.\n");
   test_allocate_physical_memory();

   init_timer(50);

   init_paging();
   puts("Virtual address initialized.\n");
   test_malloc_virtual_memory();

   (void)create_task((void*)test_task_1, "test_task_1", TASK_PRIORITY_NORMAL);
   (void)create_task((void*)test_task_2, "test_task_2", TASK_PRIORITY_NORMAL);

   asm volatile ("sti");

   init_keyboard();
   puts("Keyboard input is now enabled.\n");

   puts("Start task scheduler...\n");
   schedule_task();

   // TODO: fix the restart issue after remove mapping physical address 0-4M to virtual address 0-4M
   // remove_first_page_table_mapping();
   for (;;);
}


