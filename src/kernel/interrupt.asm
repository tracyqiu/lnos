;;;
;;; ISR/IRQ handlers
;;;

bits 32
section .text

;
; void __attribute__((cdecl)) isr_handler(registers_t *r)
; void __attribute__((cdecl)) irq_handler(registers_t *r)
; defined in isr.c
;
extern isr_handler
extern irq_handler


; ISRs
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31


;
; We don't get information about which interrupt was caller when the handler is run, 
; so we will need to have a different handler for every interrupt.
;

; 0: Divide By Zero Exception
isr0:
   push byte 0     ; Placeholder, maintain stack structure consistency
   push byte 0     ; ISR/IRQ(Interrupt number)
   jmp general_exception_handler

; 1: Debug Exception
isr1:
   push byte 0
   push byte 1
   jmp general_exception_handler

; 2: Non Maskable Interrupt Exception
isr2:
   push byte 0
   push byte 2
   jmp general_exception_handler

; 3: Int 3 Exception
isr3:
   push byte 0
   push byte 3
   jmp general_exception_handler

; 4: INTO Exception
isr4:
   push byte 0
   push byte 4
   jmp general_exception_handler

; 5: Out of Bounds Exception
isr5:
   push byte 0
   push byte 5
   jmp general_exception_handler

; 6: Invalid Opcode Exception
isr6:
   push byte 0
   push byte 6
   jmp general_exception_handler

; 7: Coprocessor Not Available Exception
isr7:
   push byte 0
   push byte 7
   jmp general_exception_handler

; 8: Double Fault Exception (With Error Code!)
isr8:
   push byte 8
   jmp general_exception_handler

; 9: Coprocessor Segment Overrun Exception
isr9:
   push byte 0
   push byte 9
   jmp general_exception_handler

; 10: Bad TSS Exception (With Error Code!)
isr10:
   push byte 10
   jmp general_exception_handler

; 11: Segment Not Present Exception (With Error Code!)
isr11:
   push byte 11
   jmp general_exception_handler

; 12: Stack Fault Exception (With Error Code!)
isr12:
   push byte 12
   jmp general_exception_handler

; 13: General Protection Fault Exception (With Error Code!)
isr13:
   push byte 13
   jmp general_exception_handler

; 14: Page Fault Exception (With Error Code!)
isr14:
   push byte 14
   jmp general_exception_handler

; 15: Reserved Exception
isr15:
   push byte 0
   push byte 15
   jmp general_exception_handler

; 16: Floating Point Exception
isr16:
   push byte 0
   push byte 16
   jmp general_exception_handler

; 17: Alignment Check Exception
isr17:
   push byte 0
   push byte 17
   jmp general_exception_handler

; 18: Machine Check Exception
isr18:
   push byte 0
   push byte 18
   jmp general_exception_handler

; 19: Reserved
isr19:
   push byte 0
   push byte 19
   jmp general_exception_handler

; 20: Reserved
isr20:
   push byte 0
   push byte 20
   jmp general_exception_handler

; 21: Reserved
isr21:
   push byte 0
   push byte 21
   jmp general_exception_handler

; 22: Reserved
isr22:
   push byte 0
   push byte 22
   jmp general_exception_handler

; 23: Reserved
isr23:
   push byte 0
   push byte 23
   jmp general_exception_handler

; 24: Reserved
isr24:
   push byte 0
   push byte 24
   jmp general_exception_handler

; 25: Reserved
isr25:
   push byte 0
   push byte 25
   jmp general_exception_handler

; 26: Reserved
isr26:
   push byte 0
   push byte 26
   jmp general_exception_handler

; 27: Reserved
isr27:
   push byte 0
   push byte 27
   jmp general_exception_handler

; 28: Reserved
isr28:
   push byte 0
   push byte 28
   jmp general_exception_handler

; 29: Reserved
isr29:
   push byte 0
   push byte 29
   jmp general_exception_handler

; 30: Reserved
isr30:
   push byte 0
   push byte 30
   jmp general_exception_handler

; 31: Reserved
isr31:
   push byte 0
   push byte 31
   jmp general_exception_handler


; IRQs
global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

;
; IRQ handlers
; IRQ0-IRQ7 mapping to interrupt number 32-39 master PIC, 
; IRQ8-IRQ15 mapping to interrupt number 40-47 slaver PIC
;

; 32: System timer/clock interrupt
irq0:
   push byte 0
   push byte 32
   jmp general_irq_handler

; 33: Keyboard Interrupt
irq1:
   push byte 1
   push byte 33
   jmp general_irq_handler

; 34: Cascade connection, used to connect from PIC
irq2:
   push byte 2
   push byte 34
   jmp general_irq_handler

; 35: Serial ports COM2 and COM4
irq3:
   push byte 3
   push byte 35
   jmp general_irq_handler

; 36: Serial ports COM1 and COM3
irq4:
   push byte 4
   push byte 36
   jmp general_irq_handler

; 37: Parallel port LPT2 or sound card
irq5:
   push byte 5
   push byte 37
   jmp general_irq_handler

; 38: Floppy disk controller
irq6:
   push byte 6
   push byte 38
   jmp general_irq_handler

; 39: Parallel port LPT1
irq7:
   push byte 7
   push byte 39
   jmp general_irq_handler

; 40: Real-time Clock (RTC)
irq8:
   push byte 8
   push byte 40
   jmp general_irq_handler

; 41: Redirected IRQ2, usually used for ACPI
irq9:
   push byte 9
   push byte 41
   jmp general_irq_handler

; 42: Reserved, usually used for network cards or SCSI controllers
irq10: 
   push byte 10
   push byte 42
   jmp general_irq_handler

; 43: Reserved, usually used for SCSI or USB controllers
irq11:
   push byte 11
   push byte 43
   jmp general_irq_handler

; 44: PS/2 Mouse
irq12:
   push byte 12
   push byte 44
   jmp general_irq_handler

; 45: Math Coprocessor/FPU
irq13:
   push byte 13
   push byte 45
   jmp general_irq_handler

; 46: Master IDE channel
irq14:
   push byte 14
   push byte 46
   jmp general_irq_handler

; 47: Slave IDE channel
irq15:
   push byte 15
   push byte 47
   jmp general_irq_handler




; Some interrupts push an error code onto the stack but others don't, 
; so we will push a dummy error code for those which don't, so that we have a consistent stack for all of them.

general_exception_handler:
   ; 1. Save CPU state
   pusha          ; pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
   mov ax, ds      ; lower 16-bits of eax = ds.
   push eax        ; save the data segment descriptor
   mov ax, 0x10     ; kernel data segment descriptor
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   ; 2. Call C handler
   push esp        ; esp == handler_ptr* pointer
   cli
   call isr_handler
   pop eax         ; clear pointer afterwards

   ; 3. Restore state
   pop eax
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax
   popa
   add esp, 8      ; cleans up the pushed error code and pushed ISR number
   sti
   iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP



general_irq_handler:
   ; 1. Save CPU state
   pusha
   mov ax, ds
   push eax
   mov ax, 0x10
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   ; 2. Call C handler
   push esp
   cli
   call irq_handler      ; different than the ISR code
   pop ebx            ; different than the ISR code

   ; 3. Restore state
   pop ebx
   mov ds, bx
   mov es, bx
   mov fs, bx
   mov gs, bx
   popa
   add esp, 8
   sti
   iret

