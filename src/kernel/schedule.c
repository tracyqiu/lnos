#include "schedule.h"


extern void switch_task_context(tss_t* old_context, tss_t* new_context);

extern task_t* task_list;

static task_t* current_task = NULL;


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

   if (next) {
      switch_to_task(next);
   }
}
