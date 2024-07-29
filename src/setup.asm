org 0x500
bits 16

%define ENDL 0x0A, 0x0D


start:
   mov ax, 2
   int 0x10

   ; setup data segments
   mov ax, 0
   mov ds, ax
   mov es, ax

   ; setup stack
   mov ss, ax
   mov sp, 0x500

   ; print message hello world
   mov si, msg
   call print

   jmp $



; Prints a string to screen
; Params:
;  - ds:si points to string
print:
   push si
   push ax
   push bx

   mov ah, 0x0E      ; call bios interrupt, refer to https://zh.wikipedia.org/wiki/INT_10H
   mov bh, 0         ; set page number to 0

.loop
   mov al, [si]      ; loads character in al
   cmp al, 0         ; verify if next character is null
   jz .done
   int 0x10

   inc si
   jmp .loop

.done
   pop bx
   pop ax
   pop si
   ret


msg: db 'loading...', ENDL, 0

times 510 - ($ - $$) db 0
db 0x55, 0xaa