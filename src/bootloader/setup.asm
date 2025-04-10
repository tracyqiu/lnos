bits 16
org 0x500

jmp start


%define ENDL 0x0A, 0x0D
%include "output.asm"
%include "mm.asm"


start:
   mov ax, cs
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov sp, 0xFFFF    ; set esp (stack pointer)

   mov si, msg
   call print_string

   ; test some cmd
   call get_input

   ; load kernel code to memory
   call load_kernel

   ; obtain the layout information of the system memory
   call query_memory

   ; switch to protected mode
   call switch_to_pm

   hlt


;
; Get user input, print to screen & run command
;
get_input:
   pusha
   mov di, msg_cmd

.keyloop
   mov ah, 0x00
   int 0x16          ; BIOS int get keystroke ah=00, character goes into al

   mov ah, 0x0E
   mov bh, 0x0
   cmp al, 0x0D      ; did user press 'enter' key?
   je .run_command
   int 0x10          ; if not, print input character to screen
   mov [di], al
   inc di
   jmp .keyloop

.run_command
   mov byte [di], 0  ; null terminate cmd from di
   mov al, [msg_cmd]
   cmp al, 'R'
   je .reboot
   cmp al, 'C'
   jne .not_found
   mov si, msg_continue
   call print_string
   popa
   ret

.not_found
   mov si, msg_reinput
   call print_string
   popa
   call get_input

.reboot
   popa
   jmp 0xffff:0x0



load_kernel:
   mov si, msg_loading_kernel
   call print_string

   ; 使用LBA模式读取扇区
   mov si, disk_address_packet
   mov word [si], 0x0010      ; 数据包大小 (16 bytes)
   mov word [si+2], 32        ; 要读取的扇区数
   mov word [si+4], 0x0000    ; 传输缓冲区偏移
   mov word [si+6], 0x1000    ; 传输缓冲区段地址
   mov dword [si+8], 5        ; 起始扇区LBA (0-based)
   mov dword [si+12], 0       ; 起始扇区高32位 (用于大硬盘)

   mov ah, 0x42               ; EXT Read Sectors From Drive
   mov dl, 0x80               ; floppy 0x00, hhd 0x80
   int 0x13
   jc kernel_load_error

   mov si, msg_hd_success
   call print_string
   ret

disk_address_packet:
   times 16 db 0


kernel_load_error:
   mov si, msg_hd_failed
   call print_string

   mov al, ah      ; error code in ah
   call print_hex  ; print error code

   jmp $


;
; prepare and switch to protected mode
; refer to x86汇编语言-从实模式到保护模式.pdf part 11
;
switch_to_pm:
   xchg bx, bx
   xchg bx, bx

   cli

   lgdt [gdt_descriptor]
   mov si, msg_gdt_loaded
   call print_string

   call enable_a20
   mov si, msg_a20_enabled
   call print_string

   ; switch to protected mode
   mov eax, cr0
   or eax, 1
   mov cr0, eax     ; PE位 = 1

   ; can not use BIOS interrupt after switch to protected mode
   ;mov si, msg_entering_protected_mode
   ;call print_string

   jmp CODE_SEG:protected_mode


;
; open A20
; refer to x86汇编语言-从实模式到保护模式.pdf part 11.5
;
enable_a20:
   in al, 0x92
   or al, 00000010b
   out 0x92, al
   ret


;
; GDT definition
; refer to x86汇编语言-从实模式到保护模式.pdf part 11.2
;
gdt_start:
   ; NULL描述符
   dd 0x0
   dd 0x0

   ; .text segment
   dw 0xFFFF         ; 段界限15-0位, 决定偏移量的最大/小值
   dw 0              ; 段基址15-0位
   db 0              ; 段基址23-16位
   db 10011010b      ; P=1存在，DPL=00特权级0，S=1代码段，TYPE=1010代码段可读可执行
   db 11001111b      ; G=1段界限单位4KB, G=0表示1B，D=1表示32位保护模式，L=0, AVL=0, 段界限19-16位会与15-0位组合
   db 0              ; 段基址24-31位

   ; .data segment
   dw 0xFFFF         ; 段限长0-15位
   dw 0              ; 段基址0-15位
   db 0              ; 段基址16-23位
   db 10010010b      ; P=1存在，DPL=00特权级0，S=1数据段，TYPE=0010数据段可读可写
   db 11001111b      ; G=1段界限单位4KB, G=0表示1B，D=1表示32位保护模式，L=0, AVL=0, 段界限19-16位会与15-0位组合
   db 0              ; 段基址24-31位

gdt_end:

gdt_descriptor:
   dw gdt_end - gdt_start - 1    ; GDT size
   dd gdt_start                  ; GDT address


CODE_SEG equ 0x08    ; code segment start
DATA_SEG equ 0x10    ; data segment start

msg:           db '-------------------------------', ENDL,\
                  'Kernel Booted, Welcome to LnOS!', ENDL,\
                  'C: load kernel', ENDL, \
                  'R: reboot', ENDL, \
                  '-------------------------------', ENDL, 0
msg_continue:  db ENDL, 'Load kernel', ENDL, 0
msg_reinput:   db ENDL, 'Oops! CMD not exist! :(', ENDL, 0
msg_cmd:       db ''
msg_hd_failed:       db 'HD read error! :(', ENDL, 0
msg_hd_success:      db 'HD read sucess! :)', ENDL, 0

msg_loading_kernel:  db 'Start loading kernel...', 13, 10, 0
msg_gdt_loaded:      db 'GDT loaded.', 13, 10, 0
msg_a20_enabled:     db 'A20 line enabled.', 13, 10, 0
msg_entering_protected_mode: db 'Entering protected mode...', 13, 10, 0


;;;
;;; enter bits 32
;;;

bits 32

%include "vga_print.asm"
%include "paging.asm"
%include "idt.asm"

;
; refer to x86汇编语言-从实模式到保护模式.pdf part 11.7
;
protected_mode:
   ; setup data segments
   mov ax, DATA_SEG
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax
   mov ss, ax
   mov esp, 0x90000     ; set esp (stack pointer)

   mov esi, protected_mode_msg
   mov edi, 0xB8000 + 80*42     ; output address
   call vga_print

   xchg bx, bx
   xchg bx, bx
   ;call setup_paging

   xchg bx, bx
   xchg bx, bx
   call setup_idt
   call setup_8259a

   ; test exception_handler
   ;xor eax, eax
   ;div eax

   ; jmp to kernel
   jmp CODE_SEG:0x10000


protected_mode_msg   db 'in 32 mode', 0x0D, 0x0A, 0


times 2048 - ($ - $$) db 0
