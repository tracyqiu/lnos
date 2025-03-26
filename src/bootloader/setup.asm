bits 16
org 0x500

jmp start


%define ENDL 0x0A, 0x0D
%include "output.asm"


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
   call setup_paging

   ; jmp to kernel
   jmp CODE_SEG:0x10000



;
; Prints a string to screen
; Params:
;  - edi points to output address
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
   mov ah, 0x07         ; black screen and light grey text
   
.print_loop:
   lodsb                ; loads next character in al
   test al, al          ; verify if al is null
   jz .print_done
   mov [edi], ax        ; put the character to be printed into the edi
   add edi, 2           ; each character takes two bytes
   jmp .print_loop
   
.print_done:
   popa
   ret

protected_mode_msg db "in 32 mode", 0



;
; Enable paging function
; refer to x86汇编语言-从实模式到保护模式.pdf part 16.3
;
;   PDT(31-22)       PTT(21-12)       物理偏移(11-0) 4K/页
;   0PDE     -->     0-0PTE    -->    0-0-0 4K
;                                     0-0-1 4K
;                                     ...
;                                     0-0-1023 4K
;                    0-1PTE    -->    0-1-0 4K
;                                     0-1-1 4K
;                                     ...
;                                     0-1-1023 4K
;                    ...
;                    0-1023PTE
;   1PDE     -->     1-0PTE    -->    1-0-0 4K
;                                     1-0-1 4K
;                                     ...
;                                     1-0-1023 4K
;                    1-1PTE
;                    ...
;                    1-1023PTE
;   ...              ...
;   1023PDE          ...
;
setup_paging:
    ; step 1: create page table
    ; 实模式下内存可访问0-1M的空间, 所以页表结构就放在紧邻1M之上的位置0x100000(1M=0xfffff), 避免冲突
    mov edi, 0x100000  ; 页目录起始地址
    xor eax, eax
    mov ecx, 1024      ; 页目录(PDT表)有1024项
    rep stosd          ; 将eax的值放到ES:DI中, 即初始化页表, stosd可使edi自增
    
    mov edi, 0x101000  ; 第一个页表地址
    mov ecx, 1024      ; 页表(PTT表)有1024项
    rep stosd
    
    ; 第一个页目录指向第一个页表
    mov dword [0x100000], 0x101003  ; 存在，可写
    
    ; 将0-0PTE的虚拟地址与物理地址做一一映射, 0-4MB <==> 0x0 - 0x400000(就包含了实模式下的1M空间)
    mov edi, 0x101000  ; 第一个页表地址0-0PTE
    mov eax, 0x3       ; 存在，可写
    mov ecx, 1024      ; 映射1024个页面（1024 * 4K == 4MB）
.init_page_loop:
    stosd
    add eax, 0x1000    ; 下一个页面 0x1000==4K
    loop .init_page_loop
    
    ; step 2: 页表地址写入cr3
    mov eax, 0x100000
    mov cr3, eax
    
    ; step 3: 设置cr的PE位以开启CPU分页模式
    mov eax, cr0
    or eax, 0x80000000 ; 第31位 PG位
    mov cr0, eax
    
    ret

times 2048 - ($ - $$) db 0