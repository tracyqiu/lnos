;;;
;;; output.asm: print string
;;;

%ifndef OUTPUT_ASM
%define OUTPUT_ASM

;
; Prints a string to screen
; Params:
;  - ds:si points to string
;
print_string:
   ; push si
   ; push ax
   ; push bx
   pusha

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
   ; pop bx
   ; pop ax
   ; pop si
   popa
   ret

;
; print hex to screen
; Params：
;  - al point to hex
;
print_hex:
   pusha
   mov cx, 2       ; print 2 hex numbers (8 bit)
   mov bh, al      ; save original hex numbers to bh
.loop:
   mov al, bh
   shr al, 4       ; shift the value of al 4 bits to the right, to put the higher 4 bit to the lower 4 bit position
   and al, 0x0F    ; lower 4 bit valid
   add al, '0'     ; convert to ASCII
   cmp al, '9'
   jle .print
   add al, 7       ; 'A'-'9'-1
.print:
   mov ah, 0x0E    ; call bios interrupt
   int 0x10

   mov al, bh
   and al, 0x0F    ; lower 4 bit valid
   add al, '0'     ; convert to ASCII
   cmp al, '9'
   jle .print2
   add al, 7       ; 'A'-'9'-1
.print2:
   mov ah, 0x0E
   int 0x10

   popa
   ret


%endif
