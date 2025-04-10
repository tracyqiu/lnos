;;;
;;; vga_print.asm: print string by VGA
;;;

%ifndef VGA_ASM
%define VGA_ASM

bits 32

;
; Prints a string to screen
; Params:
;  - edi points to output address(deprecated because the output is now printed at the cursor position)
;  - esi points to string
;
; equals to:
;   %if 0
;   ; clean screen
;   mov edi, 0xB8000     ; vga started address
;   mov ecx, 2000        ; count 80*25
;   mov ax, 0x0720       ; empty black screen and white text
;   rep stosw
;   %endif
;   mov edi, 0xB8000
;   mov eax, 0x0F4E0F49  ; text "IN", 'I' in ASCII is 73(0x49)
;   mov [edi], eax
;   mov eax, 0x0F320F33  ; text "32"
;   mov [edi+4], eax
;   mov eax, 0x0F4F0F4D  ; text "MO"
;   mov [edi+8], eax
;   mov eax, 0x0F450F44  ; text "DE"
;   mov [edi+12], eax
;
vga_print:
   pusha

   call get_cursor
   ; calculate character's position
   mov edi, 0xB8000
   add edi, eax
   add edi, eax         ; every character use 16 bit, include character and color

   mov ah, 0x07         ; black screen and light grey text

.print_loop:
   lodsb                ; loads next character in al
   test al, al          ; verify if al is null
   jz .print_done

   cmp al, 0x0D
   je .handle_cr
   cmp al, 0x0A
   je .handle_lf

   mov [edi], ax        ; put the character to be printed into the edi
   add edi, 2           ; each character takes two bytes
   jmp .print_loop

.handle_lf
   add edi, SCREEN_WIDTH * 2
   jmp .print_loop

;
; int16_t offset = get_cursor();
; int16_t row = offset / (2 * SCREEN_WIDTH);
; EAX = The dividend, in ECX = divisor.
; EAX = EAX / ECX, the result is stored in EAX while the remainder is in EDX.
; EAX = EAX * ECX, The multiplier is stored in ECX, and the result is stored in EDX:EAX. EAX stores the result while the EDX stores the overflow part.
;
.handle_cr
   mov eax, edi
   sub eax, VGA_STARTED_ADDR
   mov ecx, SCREEN_WIDTH * 2
   xor edx, edx
   div ecx                       ; row
   mul ecx                       ; offset = row * col_width * 2
   mov edi, VGA_STARTED_ADDR
   add edi, eax
   jmp .print_loop


.print_done:
   call set_cursor

   popa
   ret


;
; get current cursor
; return:
;  - eax: cursor offset (row*80 + col)
;
get_cursor:
   ; vga offset high
   mov dx, 0x3D4
   mov al, 0x0E
   out dx, al

   mov dx, 0x3D5
   in al, dx
   mov ah, al

   ; vga offset low
   mov dx, 0x3D4
   mov al, 0x0F
   out dx, al

   mov dx, 0x3D5
   in al, dx

   ret



;
; set cursor
; return:
;  - edi: cursor position (0xB8000 + offset)
;
set_cursor:
   sub edi, 0xB8000
   shr edi, 1

   mov dx, 0x3D4
   mov al, 0x0E
   out dx, al

   mov dx, 0x3D5
   mov ax, di
   shr ax, 8
   out dx, al

   mov dx, 0x3D4
   mov al, 0x0F
   out dx, al

   mov dx, 0x3D5
   mov ax, di
   out dx, al

   ret



VGA_STARTED_ADDR  equ 0x0B8000
SCREEN_WIDTH      equ 80
SCREEN_HEIGHT     equ 25

%endif
