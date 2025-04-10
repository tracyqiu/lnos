;;;
;;; idt.asm: setup idt and keyboard_irq_handler
;;;

bits 32

%include "vga_print.asm"

;
; refer to x86汇编语言-从实模式到保护模式.pdf part 17
;
setup_idt:
   ; step 1: initial Interrupt Descriptor Table
   mov edi, 0x102000  ; 放在紧邻页表的后面(页表中PDT占1024*4B, 第一个PTT占4K, 总共8K = 0x2000)
   xor eax, eax
   mov ecx, 256*8     ; IDT大小
   rep stosd

   ; 安装前20个异常中断处理过程（0-19）
   ; refer to x86汇编语言-从实模式到保护模式.pdf part 17.1.2 中断门格式
   mov edi, 0x102000
   mov eax, general_exception_handler
   and eax, 0xFFFF    ; 保存低16位
   mov ebx, general_exception_handler
   shr ebx, 16        ; 右移16位, 保存高16位
   xor esi, esi
.idt0:
   mov word [edi], ax         ; 处理程序在目标代码段内的偏移量15-00位
   mov word [edi+2], CODE_SEG ; 代码段选择子
   mov byte [edi+4], 0        ; 不使用
   mov byte [edi+5], 0x8E     ; P=1, DPL=00, S=0, TYPE=1110 (32位中断门)
   mov word [edi+6], bx       ; 处理程序在目标代码段内的偏移量31-16位
   add edi, 8                 ; 下一个IDT项
   inc esi
   cmp esi, 19
   jle .idt0

   ; 安装键盘的中断处理过程,
   ; 这个操作须在lidt [idt_descriptor]之前否则执行时会触发general_exception_handler异常
   call setup_keyboard_irq

   ; enable interrupt
   ; step 2: initial Interrupt Descriptor Table
   mov word [idt_descriptor], 256*8-1       ; IDT size
   mov dword [idt_descriptor+2], 0x102000   ; IDT address
   lidt [idt_descriptor]                    ; 加载中断描述符表寄存器IDTR

   ret


general_exception_handler:
   mov esi, exception_msg
   mov edi, 0xB8000 + 80*44     ; output address
   call vga_print

   hlt

;
; 设置8259A中断控制器
; refer to x86汇编语言-从实模式到保护模式.pdf part 17.3.4
; 主片的端口号是0x20 和0x21，从片的端口号是0xA0 和0xA1
; 中断向量0x20～0xFF（32～255）是用户可以自由分配的部分
;
setup_8259a:
   ; ICW1: 边沿触发/级联方式, 每次8259A 芯片接到ICW1 时，都意味着一个新的初始化过程开始了
   mov al, 0x11
   out 0x20, al
   out 0xA0, al

   ; ICW2: 中断向量表
   mov al, 0x20
   out 0x21, al       ; 主片范围0x20～0x27
   mov al, 0x28
   out 0xA1, al       ; 从片就从0x28开始

   ; ICW3: 从片级联到IR2
   mov al, 0x04
   out 0x21, al
   mov al, 0x02
   out 0xA1, al

   ; ICW4: 多片级联, 采用非自动结束方式
   mov al, 0x01
   out 0x21, al
   out 0xA1, al

   ; 设置中断屏蔽
   mov al, 0xFF       ; 暂时屏蔽所有中断，由内核开启
   ;out 0x21, al
   out 0xA1, al

   mov al, 0xFD       ; 只允许IRQ1 (键盘)
   out 0x21, al

   ret

idt_descriptor:
   dw 0    ; idt size, 16位表界限
   dd 0    ; idt address, 32位基地址



;
; 安装键盘的中断处理过程, IRQ1==0x21
;
setup_keyboard_irq:
   pushad

   mov eax, keyboard_irq_handler
   mov ebx, 0x102000 + 0x21 * 8
   mov word [ebx], ax
   mov word [ebx+2], 0x08
   mov byte [ebx+4], 0
   mov byte [ebx+5], 0x8E
   shr eax, 16
   mov word [ebx+6], ax

   popad
   ret


keyboard_irq_handler:
   pushad

   in al, 0x60          ; read keyboard scan code
   mov bl, al

   test bl, 0x80
   jnz .end_handler

   mov edi, 0xB8000 + 80*48
   mov ah, 0x07
   mov esi, key_pressed_msg
.print_msg:
   lodsb
   test al, al
   jz .print_scancode
   mov [edi], ax
   add edi, 2
   jmp .print_msg

.print_scancode
   movzx esi, bl
   cmp esi, scancode_to_ascii_len
   jae .end_handler
   mov al, [scancode_to_ascii + esi]
   test al, al
   jz .end_handler

   mov edi, [keyboard_cursor]
   mov [edi], ax
   add edi, 2           ; move keyboard_cursor to next character place
   mov [keyboard_cursor], edi

.end_handler:
   mov al, 0x20
   out 0x20, al         ; send EOI(end of interrupt) signal

   popad
   iret



CODE_SEG equ 0x08    ; code segment start

exception_msg        db  '********Exception encountered********',0

key_pressed_msg      db "Key pressed! scancode:", 0
key_pressed_msg_len  equ $ - key_pressed_msg - 1
keyboard_cursor      dd 0xB8000 + 80*48 + key_pressed_msg_len*2
scancode_to_ascii:
                     db 0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0
                     db 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0
                     db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`'
                     db 0, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
                     db '*', 0, ' ', 0
scancode_to_ascii_len equ $ - scancode_to_ascii - 1
