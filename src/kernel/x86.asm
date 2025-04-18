bits 32
section .text


global x86_inb
global x86_outb
global x86_jmp_to_high_virtual_addr

x86_outb:
   mov dx, [esp + 4]      ; port
   mov al, [esp + 8]      ; value
   out dx, al
   ret

x86_inb:
   mov dx, [esp + 4]      ; value
   xor eax, eax
   in al, dx
   ret



x86_jmp_to_high_virtual_addr:
   ; don't forget to change the return address
   pop ebx                 ; pop the return address which stores in esp to ebp
   add ebx, 0xC0000000

   lea eax, [.current_code_addr_offset]
   add eax, 0xC0000000
   jmp eax                 ; jmp to high address of code
   .current_code_addr_offset:

   add esp, 0xC0000000     ; update esp & ebp
   add ebp, 0xC0000000

   push ebx                ; push the new return address to new esp
   ret

