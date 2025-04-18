;;;
;;; paging.asm: setup paging
;;;

bits 32

%include "vga_print.asm"

;
; Enable paging function
; refer to x86汇编语言-从实模式到保护模式.pdf part 16.3
;
;   PDT(31-22)       PTT(21-12)          物理偏移(11-0) 4K/页
;   0PDE     -->     0-0PTE      -->     4K
;                    0-1PTE      -->     4K
;                    ...         -->     ...
;                    0-1023PTE   -->     4K
;   1PDE     -->     1-0PTE      -->     4K
;                    1-1PTE      -->     4K
;                    ...         -->     ...
;                    1-1023PTE   -->     4K
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

   ; 将0-1023PTE的虚拟地址与物理地址做一一映射, 0-4MB <==> 0x0 - 0x400000(就包含了实模式下的1M空间)
   mov edi, 0x101000  ; 第一个页表地址0PDE, 包含0-1023PTE
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

