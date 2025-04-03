
%if 0

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


global keyboard_irq_handler
global setup_keyboard_irq

section .text

;
; 安装键盘的中断处理过程, IRQ1==0x21
;
setup_keyboard_irq:
   pushad
   
   mov eax, keyboard_irq_handler
   mov ebx, 0x102000 + 0x21 * 8
   mov word [ebx], ax
   mov word [ebx+2], 0x08
   mov byte [ebx+4], 0
   mov byte [ebx+5], 0x8E
   shr eax, 16
   mov word [ebx+6], ax
   
   popad
   ret


keyboard_irq_handler:
   pushad
   
   in al, 0x60          ; read keyboard scan code
   mov bl, al
   
   test bl, 0x80
   jnz .end_handler

   mov edi, 0xB8000 + 80*48
   mov ah, 0x07
   mov esi, key_pressed_msg
.print_msg:
   lodsb
   test al, al
   jz .print_scancode
   mov [edi], ax
   add edi, 2
   jmp .print_msg

.print_scancode
   movzx esi, bl
   cmp esi, scancode_to_ascii_len
   jae .end_handler
   mov al, [scancode_to_ascii + esi]
   test al, al
   jz .end_handler
   
   mov edi, [keyboard_cursor]
   mov [edi], ax
   add edi, 2           ; move keyboard_cursor to next character place
   mov [keyboard_cursor], edi

.end_handler:
   mov al, 0x20
   out 0x20, al         ; send EOI(end of interrupt) signal
   
   popad
   iret

section .data
key_pressed_msg      db "Key pressed! scancode:", 0
key_pressed_msg_len  22
keyboard_cursor      dd 0xB8000 + 80*48 + key_pressed_msg_len*2
scancode_to_ascii:
                     db 0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0
                     db 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0
                     db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`'
                     db 0, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 
                     db '*', 0, ' ', 0
scancode_to_ascii_len 58

%endif