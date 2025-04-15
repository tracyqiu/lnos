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

   call find_highest_address

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
   call print_hex_32

   mov si, msg_mm_size
   call print_string

   mov eax, [0x7004]
   call print_hex_32

   mov si, msg_end
   call print_string

   mov si, msg_mm_highest_addr
   call print_string

   mov eax, [0x7008]
   call print_hex_32

   mov si, msg_end
   call print_string
   ret


;
; find the highest available memory address to reset the stack pointer in setup.asm
;
find_highest_address:
   ; calculate how many entries of Address Range Descriptor structure and save to ecx
   mov ecx, edi
   sub ecx, 0x8000
   mov eax, ecx
   xor edx, edx
   mov ecx, 24
   div ecx
   mov ecx, eax                  ; command loop checks and decrements the value of ecx

   xor edx, edx                  ; use edx to save the highest address
   mov esi, 0x8000

.find_highest_addr:
   cmp dword [esi + 16], MEMORY_AVAILABLE
   jne .next_range

   mov eax, [esi]                ; base address(address_start)
   mov ebx, [esi + 8]            ; length
   add eax, ebx                  ; address_end
   cmp eax, edx                  ; cmp the highest address
   jbe .next_range
   mov edx, eax

.next_range:
   add esi, 24
   loop .find_highest_addr

;.done
   mov dword [0x7008], edx       ; save highest available address to 0x7008
   ret




SIGNATURE_SMAP    equ 0x534D4150
MEMORY_AVAILABLE  equ 1
msg_mm_addr:         db 'Obtain memory layout info success :) mm addr: 0x',0
msg_mm_size:         db ', mm size: 0x', 0
msg_end:             db 13, 10, 0
msg_mm_highest_addr: db 'memory highest_addr: 0x', 0
msg_mm_failed:       db 'Ops, memory detect error :(', 13, 10, 0
