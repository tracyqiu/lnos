#include "task.h"
#include "mm.h"
#include "string.h"
#include "ktypes.h"
#include "print.h"
#include "stdlib.h"

static uint32_t next_pid = 1;
task_t* task_list = NULL;
task_t* task_lists[TASK_ENTRIES] = {0}; // temporay solution to do some test


//------------------------------------------------------------------------------
static task_t* add_task(task_t* task)
//------------------------------------------------------------------------------
{
   if (!task_list) {
      task_list = task;
   } else {
      task_t* p = task_list;
      while (p->next != NULL) {
         p = p->next;
         char buf[32];
         puts("p addr: ");
         puts(itoa((uint32_t)p, buf, 16));
         puts("\n");
      }
      p->next = task;
      task->next = NULL;
   }

   return task_list;
}

//------------------------------------------------------------------------------
task_t* create_task(task_func_t func, const char* name, const task_priority_t priority)
//------------------------------------------------------------------------------
{
   task_t* task = (task_t*)malloc_virtual_memory(sizeof(task_t));
   if (!task) {
      puts("!!! Failed to allocate memory for task");
      return NULL;
   }

   memset(task, 0, sizeof(task_t));
   task->pid = next_pid++;
   strncpy(task->name, name, 31);
   task->name[31] = '\0';
   task->state = TASK_STATE_INIT;
   task->priority = priority;
   task->time_slice = 20 + priority * 10;
   task->time_used = 0;
   task->next = NULL;

   task->stack_size = PAGE_SIZE;
   task->stack_addr = malloc_virtual_memory(task->stack_size);
   if (!task->stack_addr) {
      puts("!!! Failed to allocate stack for task");
      free_virtual_memory((void*)task);
      return NULL;
   }

   memset(&task->context, 0, sizeof(tss_t));
   uint32_t* stack_top = (uint32_t*)((uint8_t*)task->stack_addr + task->stack_size);

   /** init register
   * bottom of the stack
   * ...
   * top of the stack
   */
   // keep consistent with the sequence of command iret
   *(--stack_top) = 0x10;                 // SS
   --stack_top;
   *stack_top = (uint32_t)stack_top;      // ESP
   *(--stack_top) = 0x202;                // EFLAGS (IF=1 means allow interruption)
   *(--stack_top) = 0x08;                 // CS
   *(--stack_top) = (uint32_t)func;       // EIP

   // keep consistent with the sequence of command popad
   *(--stack_top) = 0;                    // EAX
   *(--stack_top) = 0;                    // ECX
   *(--stack_top) = 0;                    // EDX
   *(--stack_top) = 0;                    // EBX
   --stack_top;
   *stack_top = (uint32_t)stack_top;      // ESP
   *(--stack_top) = (uint32_t)((uint8_t*)task->stack_addr + task->stack_size); // EBP
   *(--stack_top) = 0;                    // ESI
   *(--stack_top) = 0;                    // EDI

   task->context.esp = (uint32_t)stack_top;
   task->context.cs = 0x80;
   task->context.ss = 0x10;
   task->context.ds = 0x10;
   task->context.fs = 0x10;
   task->context.gs = 0x10;
   task->context.eflags = 0x202;

   task->context.eip = (uint32_t)func;

   char buf[32];
   puts("Created task: ");
   puts(task->name);
   puts(" (PID: ");
   puts(itoa(task->pid, buf, 10));
   puts("), address: ");
   puts(itoa((uint32_t)task, buf, 16));
   puts("\n");

   add_task(task);
   task_lists[task->pid - 1] = task;

   task->state = TASK_STATE_READY;
   return task;
}

//------------------------------------------------------------------------------
void destroy_task(task_t* task)
//------------------------------------------------------------------------------
{
   if (!task) return;

   if (task_list == task) {
      task_list = task->next;
   } else {
      task_t* p = task_list;
      while (p && p->next != task) p = p->next;
      if (p) p->next = task->next;
   }

   free_virtual_memory(task->stack_addr);
   free_virtual_memory(task);

   // if (current_task == task){
   //    current_task = NULL;
   //    schedule();
   // }
}
