org 0x7C00
bits 16

global _start
_start:
   mov ax, 2
   int 0x10

   ; setup data segments
   mov ax, 0
   mov ds, ax
   mov es, ax

   ; setup stack
   mov ss, ax
   mov sp, 0x7C00

   call load_setup
   jmp 0x500

   ; cli
   hlt


; Read 1 sector from disk
load_setup:
    mov ah, 0x02     ; function number:AH=0x02 means read hard disk sector
    mov al, 0x01     ; 1 sector to read
    mov ch, 0x00     ; cylinder
    mov cl, 0x02     ; started sector offset
    mov dh, 0x00     ; head
    mov dl, 0x80     ; driver number

    mov bx, 0x500    ; data should be after the bootloader

    int 0x13

    ret



times 510 - ($ - $$) db 0
db 0x55, 0xaa