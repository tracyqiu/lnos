;;;
;;; extern void switch_task_context(tss_t* old_context, tss_t* new_context);
;;;

bits 32
section .text
global switch_task_context

switch_task_context:
   push ebp
   mov ebp, esp

   mov eax, [ebp + 8]      ; old_context
   mov edx, [ebp + 12]     ; new_context

   ; save old_context
   ; the sequence is the same with tss_t
   mov [eax + 0], edi
   mov [eax + 4], esi
   mov [eax + 8], ebp
   lea ecx, [ebp + 8]      ; esp
   mov [eax + 12], ecx
   mov [eax + 16], ebx
   mov [eax + 20], edx
   mov [eax + 24], ecx
   mov [eax + 28], eax

   mov ecx, [ebp + 4]      ; return address
   mov [eax + 32], ecx     ; eip

   mov ecx, cs
   mov [eax + 36], ecx

   pushfd                  ; EFLAGS
   pop ecx                 ; pop eflags to ecx
   mov [eax + 40], ecx

   mov ecx, esp
   mov [eax + 44], ecx

   mov ecx, ss
   mov [eax + 48], ecx

   mov ecx, ds
   mov [eax + 52], ecx

   mov ecx, fs
   mov [eax + 56], ecx

   mov ecx, gs
   mov [eax + 60], ecx


   ; switch to new_context

   ;mov ecx, [edx + 48]
   ;mov ss, ecx

   mov ecx, [edx + 52]
   mov ds, ecx

   mov ecx, [edx + 56]
   mov fs, ecx

   mov ecx, [edx + 60]
   mov gs, ecx

   mov edi, [edx + 0]
   mov esi, [edx + 4]
   mov ebp, [edx + 8]
   mov ebx, [edx + 16]
   mov ecx, [edx + 24]
   mov eax, [edx + 28]

   push dword [edx + 40]   ; EFLAGS
   popfd

   mov esp, [edx + 12]

   push dword [edx + 32]   ; eip
   ret


   ; test for check eip
%if 0
   mov ecx, [edx + 32]
   test ecx, ecx
   jz .error

   mov esp, [edx + 12]

   jmp ecx

.error
   mov eax, 0xEFFFFFFF
   hlt
   
%endif