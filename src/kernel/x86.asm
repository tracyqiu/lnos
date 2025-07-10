bits 32
section .text


global x86_inb
global x86_outb

%if 0   ; cause data reading error
global x86_inw
global x86_outw
global x86_inl
global x86_outl
%endif

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



%if 0
x86_outw:
   mov dx, [esp + 4]      ; port
   mov ax, [esp + 8]      ; value
   out dx, ax
   ret

x86_inw:
   mov dx, [esp + 4]      ; value
   xor eax, eax
   in ax, dx
   ret

x86_outl:
   mov dx, [esp + 4]      ; port
   mov eax, [esp + 8]      ; value
   out dx, eax
   ret

x86_inl:
   mov dx, [esp + 4]      ; value
   xor eax, eax
   in eax, dx
   ret
%endif


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

