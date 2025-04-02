bits 32
extern keyboard_handler 

global keyboard_handler_wrapper
global setup_keyboard_idt

section .text


keyboard_handler_wrapper:
   pusha
   mov ax, ds
   push eax
   mov ax, 0x10
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   cld                       ; 清除方向标志
   push esp
   call keyboard_handler
   pop ebx

   ; 3. Restore state
   pop ebx
   mov ds, bx
   mov es, bx
   mov fs, bx
   mov gs, bx
   popa
   add esp, 8
   iret


setup_keyboard_idt:
   mov eax, keyboard_handler_wrapper
   mov ebx, 0x102000 + 0x21 * 8 
   mov word [ebx], ax
   mov word [ebx+2], 0x08
   mov byte [ebx+4], 0
   mov byte [ebx+5], 0x8E 
   shr eax, 16
   mov word [ebx+6], ax
   ret
