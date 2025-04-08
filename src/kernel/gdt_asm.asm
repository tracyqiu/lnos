;;;
;;; void __attribute__((cdecl)) load_gdt(uint32_t descriptor, uint16_t code_segment, uint16_t data_segment);
;;;

bits 32
section .text
global load_gdt

load_gdt:
   ; make new call frame
   push ebp             ; save old call frame
   mov ebp, esp         ; initialize new call frame

   mov eax, [ebp + 8]
   mov ecx, [ebp + 12]
   mov edx, [ebp + 16]

   ;cli
   ; load gdt
   lgdt [eax]

   push ecx
   push .reload_cs
   retf                 ; update cs and eip

.reload_cs
   ; reload code/data segment
   mov ax, dx
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax
   mov ss, ax

   ;sti
   ; restore old call frame
   mov esp, ebp
   pop ebp
   ret

