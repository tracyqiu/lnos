;;;
;;; memory.asm: Query system address(memory) map
;;; refer to ACPI v6.3 part 15
;;;


bits 16

%include "output.asm"

query_memory:
   mov eax, 0xE820               ; eax - function code
   xor ebx, ebx                  ; ebx - pointer to continuation value
   mov di, 0x8000                ; es:di pointer to an Address Range Descriptor structure that the BIOS fills in
   mov ecx, 24                   ; ecx - size of Address Range Descriptor Structure (24)
   mov edx, SIGNATURE_SMAP       ; edx - signature
   int 0x15                      ; call interrupt
   jc .error                     ; if error then CF=1

   cmp eax, SIGNATURE_SMAP       ; BIOS should return 'SMAP' in eax
   jne .error


   test ebx, ebx                 ; ebx=0 means only one range of physical memory
   je .done

.next_entry:
   add di, 24                    ; next range of physical memory
   mov eax, 0xE820
   mov ecx, 24
   int 0x15
   jc .done

   test ebx, ebx                 ; ebx=0 means end of the range
   jne .next_entry

.done:
   mov dword [0x7000], 0x8000    ; save the address of memory mapping info to 0x7000
   mov dword [0x7004], edi       ; save size to 0x7004
   jmp .print_success_info

.error:
   mov dword [0x7000], 0
   mov dword [0x7004], 0

   mov si, msg_mm_failed
   call print_string

   ret

.print_success_info
   mov si, msg_mm_addr
   call print_string

   mov eax, [0x7000]
   call print_hex

   mov si, msg_mm_size
   call print_string

   mov eax, [0x7004]
   call print_hex

   mov si, msg_end
   call print_string
   ret



SIGNATURE_SMAP  equ 0x534D4150
msg_mm_addr:         db 'Obtain memory layout info success :) mm addr: 0x',0
msg_mm_size:         db ', mm size: 0x', 0
msg_end:             db 13, 10, 0
msg_mm_failed:       db 'Ops, memory detect error :(', 13, 10, 0
