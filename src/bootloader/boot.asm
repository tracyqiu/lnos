org 0x7C00
bits 16

jmp _start

%define ENDL 0x0D, 0x0A 
%include "output.asm"

_start:
   ; clear screen
;   mov ax, 2
;   int 0x10

   ; setup data segments
   xor ax, ax
   mov ds, ax
   mov es, ax
   ; setup stack
   mov ss, ax
   mov sp, 0x7C00

   mov si, msg_hd_start_read
   call print_string
;   xchg bx, bx   ; bochs break point
;   xchg bx, bx

   ; reset HD 
   mov ah, 0
   int 13h
   jc disk_error

   call load_setup_org

   ; cli
   ; hlt


; Read 4 sector from disk
load_setup_org:
   mov bx, 0x500   ; the address is the same with setup org address
   
   mov ah, 0x02    ; function number:AH=0x02 means read hard disk sector
   mov al, 0x04    ; 1 sector to read
   mov ch, 0x00    ; cylinder
   mov cl, 0x02    ; started sector offset
   mov dh, 0x00    ; head
   mov dl, 0x80    ; floppy 0x00, hhd 0x80
   int 13h
   jc disk_error

   mov si, msg_hd_success
   call print_string
   
   jmp 0x0:0x500

%if 0
; deprecated
load_setup_no_org:
   mov bx, 0x500   ; data want to loaded to
   mov es, bx
   mov bx, 0
   call load_hd
   jmp 0x500:0x0

; Read 1 sector from disk
load_hd:
   mov ah, 0x02    ; function number:AH=0x02 means read hard disk sector
   mov al, 0x01    ; 1 sector to read
   mov ch, 0x00    ; cylinder
   mov cl, 0x02    ; started sector offset
   mov dh, 0x00    ; head
   mov dl, 0x80    ; floppy 0x00, hhd 0x80

   int 13h
   jc disk_error

   mov si, msg_hd_success
   call print_string
   ret
%endif

disk_error:
   mov si, msg_hd_failed
   call print_string
   jmp $


msg_hd_start_read:  db 'Start read HD...', ENDL, 0
msg_hd_failed:      db 'HD error! :(', ENDL, 0
msg_hd_success:     db 'load setup.asm sucess! :)', ENDL, 0


times 510 - ($ - $$) db 0
db 0x55, 0xaa