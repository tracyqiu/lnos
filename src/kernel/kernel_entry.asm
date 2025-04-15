bits 32
extern cstart  ; C函数入口

section .text

global c_start
c_start:
   ; 设置栈
   mov esp, kernel_stack_top

   call cstart

   jmp $


; 内核栈
section .bss
align 4
kernel_stack_bottom:
   resb 65536  ; 64KB栈空间
kernel_stack_top:
