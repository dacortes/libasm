;global ft_write
;extern errno
;
;section .text
;	ft_write:
;		push rbp
;		mov rbp, rsp
;
;		mov rax, 1
;		syscall
;		cmp rax, 0
;		jl .error
;		jmp .done
;	.error:
;		push rax
;		neg eax
;		mov [errno], eax
;		pop rax
;		mov rax, -1
;	.done:
;		pop rbp
;		ret

global ft_write
extern __errno_location

section .text
ft_write:
    push rbp
    mov rbp, rsp
    
    ; Realizar la syscall
    mov rax, 1           ; syscall write
    syscall
    
    ; Verificar si hay error (rax < 0)
    cmp rax, 0
    jge .success
    
    ; Error: guardar el código de error
    mov r12, rax         ; Guardar valor negativo
    
    ; Llamar a __errno_location usando PLT (sintaxis NASM)
    call __errno_location wrt ..plt
    
    ; Configurar errno
    mov rcx, r12
    neg ecx              ; Convertir a positivo
    mov [rax], ecx       ; errno = código de error
    
    ; Retornar -1
    mov rax, -1
    jmp .done
    
.success:
    ; rax ya tiene los bytes escritos
    
.done:
    pop rbp
    ret

; Añadir sección .note.GNU-stack para evitar warning
section .note.GNU-stack noalloc noexec nowrite progbits