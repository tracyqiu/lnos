#include "schedule.h"
#include "print.h"
#include "stdlib.h"
#include "ktypes.h"


extern void __attribute__((cdecl)) switch_task_context(tss_t* old_context, tss_t* new_context);

extern task_t* task_list;

extern task_t* task_lists[TASK_ENTRIES];

static task_t* current_task = NULL;
/*
// temporay solution to do some test
//------------------------------------------------------------------------------
static task_t* find_ready_task()
//------------------------------------------------------------------------------
{
   if (current_task)
   {
      for (uint32_t i = 0; i < TASK_ENTRIES; ++i) {
         if (task_lists[i] == NULL) break;
         if (current_task == task_lists[i] && i < TASK_ENTRIES-1 && task_lists[i+1] != NULL && task_lists[i+1]->state == TASK_STATE_READY) {
            return task_lists[i+1];
         }
      }
   }

   for (uint32_t i = 0; i < TASK_ENTRIES; ++i) {
      if (task_lists[i] == NULL) break;
      if (task_lists[i]->state == TASK_STATE_READY) {
         return task_lists[i];
      }
   }

   return NULL;
}
*/


//------------------------------------------------------------------------------
static task_t* find_ready_task()
//------------------------------------------------------------------------------
{
   if (!task_list) return NULL;

   if (current_task && current_task->next) {
      task_t* p = current_task->next;
      do {
         if (p->state == TASK_STATE_READY) {
            return p;
         }
         p = p->next;
      } while (p);
   }

   task_t* p = task_list;
   do {
      if (p->state == TASK_STATE_READY) {
         return p;
      }
      p = p->next;
   } while (p);

   return NULL;
}


//------------------------------------------------------------------------------
static void switch_to_task(task_t* task)
//------------------------------------------------------------------------------
{
   if (!task || task == current_task) return;

   task_t* old_task = current_task;
   if (old_task) {
      old_task->state = TASK_STATE_READY;
   }

   task->state = TASK_STATE_RUNNING;
   current_task = task;

   if (old_task) {
      switch_task_context(&old_task->context, &task->context);
   } else {
      asm volatile (
         "movl %0, %%esp\n"
         "popal\n"
         "iret\n"       // iret restore EIP, CS, EFLAGS, ESP, SS
         : : "r" (task->context.esp)
     );
   }
}


//------------------------------------------------------------------------------
void schedule_task()
//------------------------------------------------------------------------------
{
   task_t* next = find_ready_task();

   #if DEF_DEBUG_TRACE
   puts("find ready task: ");
   char buf[32];
   puts(itoa((int32_t)next, buf, 16));
   puts("\n");
   #endif

   if (next) {
      switch_to_task(next);
   }
}
