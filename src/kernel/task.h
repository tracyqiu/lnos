#ifndef TASK_h
#define TASK_h

#include "stdint.h"

typedef enum {
   TASK_STATE_INIT = 0,
   TASK_STATE_READY,
   TASK_STATE_RUNNING,
   TASK_STATE_BLOCKED,
   TASK_STATE_TERMINATED,
} task_state_t;

typedef enum {
   TASK_PRIORITY_LOW = 0,
   TASK_PRIORITY_NORMAL,
   TASK_PRIORITY_HIGH,
} task_priority_t;

typedef struct {
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
   uint32_t eip, cs, eflags, user_esp, ss, ds, fs, gs;
} __attribute__((packed)) tss_t;


typedef struct task {
   uint32_t pid;
   char name[32];

   tss_t context;

   task_state_t state;
   task_priority_t priority;

   uint32_t stack_size;
   void* stack_addr;

   uint32_t time_slice;
   uint32_t time_used;

   struct task* next;
} __attribute__((packed)) task_t;

typedef void* (*task_func_t)(void*);

task_t* create_task(task_func_t func, const char* name, const task_priority_t priority);

void destroy_task(task_t* task);


#endif