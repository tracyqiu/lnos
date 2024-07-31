section .text
bits 16


;
; int 10h ah=0Eh
; params: character, page
;
global x86_output_char
x86_output_char:
   ; create a new call frame
   push bp             ; save old call frame
   mov bp, sp          ; initialize new call frame

   ; save bx
   push bx

   ; [bp + 0] - old call frame
   ; [bp + 2] - return address (small memory model => 2 bytes)
   ; [bp + 4] - first argument (character)
   ; [bp + 6] - second argument (page)
   ; note: bytes are converted to words (you can't push a single byte on the stack)
   mov ah, 0Eh
   ; mov al, [bp + 4]
   ; mov bh, [bp + 6]
   mov al, dl
   mov bh, cl

   int 0x10

   ; restore bx
   pop bx

   ; restore old call frame
   mov sp, bp
   pop bp

   ret