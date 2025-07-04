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
#include "test.h"


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

   test_printf();

   (void)create_task((void*)test_task_1, "test_task_1", TASK_PRIORITY_NORMAL);
   (void)create_task((void*)test_task_2, "test_task_2", TASK_PRIORITY_NORMAL);
   (void)create_task((void*)test_task_3, "test_task_3", TASK_PRIORITY_NORMAL);
   (void)create_task((void*)test_task_4, "test_task_4", TASK_PRIORITY_NORMAL);

   asm volatile ("sti");

   init_keyboard();
   puts("Keyboard input is now enabled.\n");

   puts("Start task scheduler...\n");
   schedule_task();


   // TODO: fix the restart issue after remove mapping physical address 0-4M to virtual address 0-4M
   // remove_first_page_table_mapping();
   for (;;);
}


