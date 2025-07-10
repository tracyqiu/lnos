#include "test.h"
#include "print.h"
#include "memory.h"
#include "stdlib.h"
#include "paging.h"
#include "mm.h"
#include "task.h"
#include "schedule.h"
#include "disk.h"


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

   char buffer[128];
   sprintf(buffer, "%s, %d, %i, %uU, 0x%x, 0x%X, 0%o, %p\n", ":)", 123, -123, 4294967295U, 255, 255, 8, (void*)0x12345678);
   printf("sprintf result: %s\n", buffer);
   printf("=== End Printf Test ===\n");
   #endif
}

//------------------------------------------------------------------------------
void test_disk_rw()
//------------------------------------------------------------------------------
{
   // read MBR sector 0
   uint8_t boot_sector[512];
   if (disk_read(0, 1, boot_sector)) {
      char buffer[128];
      sprintf(buffer, "0x%x%x%x%x%x%x...%x%x%x%x",
         boot_sector[0], boot_sector[1], boot_sector[2],boot_sector[3], boot_sector[4], boot_sector[5],
         boot_sector[508], boot_sector[509], boot_sector[510], boot_sector[511]);
      if (boot_sector[0] == 0xEB && boot_sector[1] == 0x5C
         && boot_sector[510] == 0x55 && boot_sector[511] == 0xAA) {
         printf("Disk read MBR signature %s SUCCEED\n", buffer);
      } else {
         printf("Ops, disk read MBR signature is invalid :(\n");
      }
   } else {
      printf("Ops, disk read failed :(\n");
   }

   boot_sector[508] = 0x55;
   boot_sector[509] = 0xAA;
   if (disk_write(0, 1, boot_sector)) {
      uint8_t tmpbuf[512];
      disk_read(0, 1, tmpbuf);

      char buffer[128];
      sprintf(buffer, "0x%x%x%x%x%x%x...%x%x%x%x",
         tmpbuf[0], tmpbuf[1], tmpbuf[2],tmpbuf[3], tmpbuf[4], tmpbuf[5],
         tmpbuf[508], tmpbuf[509], tmpbuf[510], tmpbuf[511]);

      if (tmpbuf[0] == 0xEB && tmpbuf[1] == 0x5C
         && tmpbuf[508] == 0x55 && tmpbuf[509] == 0xAA
         && tmpbuf[510] == 0x55 && tmpbuf[511] == 0xAA) {
         printf("Disk write MBR signature %s SUCCEED\n", buffer);

         boot_sector[508] = 0x0;
         boot_sector[509] = 0x0;
         disk_write(0, 1, boot_sector);
      } else {
         printf("Ops, disk write MBR signature is invalid :(\n");
      }
   } else {
      printf("Ops, disk write MBR signature failed :(\n");
   }
}