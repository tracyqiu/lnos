#include "test.h"
#include "print.h"
#include "memory.h"
#include "stdlib.h"
#include "paging.h"
#include "mm.h"
#include "task.h"
#include "schedule.h"


//------------------------------------------------------------------------------
void test_allocate_physical_memory()
//------------------------------------------------------------------------------
{
   puts("Memory allocation test: ");
   void* mm0 = malloc_physical_memory(8192);
   void* mm1 = malloc_physical_memory(2048);
   void* mm2 = malloc_physical_memory(8192);
   if (mm1 && mm0 && mm2) {
      puts("addr0(8KB)=");
      char buf[32];
      puts(itoa((int32_t)mm0, buf, 16));

      puts(", addr1(2KB)=");
      puts(itoa((int32_t)mm1, buf, 16));
      puts(", addr2(8KB)=");
      puts(itoa((int32_t)mm2, buf, 16));
      puts(" >>> SUCCESS :)\n");
   }
   else {
      puts("FAILED :(\n");
   }
   free_physical_memory(mm2);
   free_physical_memory(mm1);
   free_physical_memory(mm0);
}

//------------------------------------------------------------------------------
void test_malloc_virtual_memory()
//------------------------------------------------------------------------------
{
   puts("Virtual mm allocation test: ");
   void* mm0 = malloc_virtual_memory(8192);
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


//------------------------------------------------------------------------------
void test_task_1()
//------------------------------------------------------------------------------
{
   puts("test_task_1...\n");
   int32_t count = 0;
   while(1) {
      if ((++count) % 1000 == 0) {
         puts("Task 1 running...\n");
         schedule_task();
      }
      for(int32_t i = 0; i < 100000; i++) {
         asm volatile("nop");
      }
   }
}

//------------------------------------------------------------------------------
void test_task_2()
//------------------------------------------------------------------------------
{
   puts("test_task_2...\n");
   int32_t count = 0;
   while(1) {
      if ((++count) % 1000 == 0) {
         puts("Task 2 running...\n");
         schedule_task();
      }
      for(int32_t i = 0; i < 100000; i++) {
         asm volatile("nop");
      }
   }
}

//------------------------------------------------------------------------------
void test_task_3()
//------------------------------------------------------------------------------
{
   puts("test_task_3...\n");
   int32_t count = 0;
   while(1) {
      if ((++count) % 1000 == 0) {
         puts("Task 3 running...\n");
         schedule_task();
      }
      for(int32_t i = 0; i < 100000; i++) {
         asm volatile("nop");
      }
   }
}

//------------------------------------------------------------------------------
void test_task_4()
//------------------------------------------------------------------------------
{
   puts("test_task_4...\n");
   int32_t count = 0;
   while(1) {
      if ((++count) % 1000 == 0) {
         puts("Task 4 running...\n");
         schedule_task();
      }
      for(int32_t i = 0; i < 100000; i++) {
         asm volatile("nop");
      }
   }
}

//------------------------------------------------------------------------------
void test_printf()
//------------------------------------------------------------------------------
{
   #ifdef DEF_DEBUG_TRACE
   printf("=== Start Printf Test ===\n");
   printf("String: %s\n", "Hello, OS! :)");
   printf("Integer: %d\n", 123);
   printf("Negative: %d\n", -123);
   printf("Unsigned: %u\n", 4294967295U);
   printf("Hex lower: %x\n", 255);
   printf("Hex upper: %X\n", 255);
   printf("Octal: %o\n", 8);
   printf("Character: %c\n", 'A');
   printf("Pointer: %p\n", (void*)0x12345678);
   printf("Percent: %%\n");
   printf("Multiple formats: %s, %d, %i, %uU, 0x%x, 0x%X, 0%o, %p\n", ":)", 123, -123, 4294967295U, 255, 255, 8, (void*)0x12345678);

   // char buffer[128];
   // sprintf(buffer, "%s, %d, %i, %uU, 0x%x, 0x%X, 0%o, %p\n", ":)", 123, -123, 4294967295U, 255, 255, 8, (void*)0x12345678);
   // printf("sprintf result: %s\n", buffer);
   printf("=== End Printf Test ===\n");
   #endif
}