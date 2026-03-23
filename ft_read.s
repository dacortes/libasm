global ft_read
extern __errno_location

section .text
	ft_read:
		push rbp
		mov rbp, rsp
		push rbx
		push r12

		mov rax, 0
		syscall

		cmp rax, 0
		jl .error

		pop r12
		pop rbx
		pop rbp
		ret
	
	.error:
		mov r12, rax
		call __errno_location wrt ..plt
		mov rcx, r12
		neg ecx
		mov [rax], ecx
		mov rax, -1
		
		pop r12
		pop rbx
		pop rbp
		ret

section .note.GNU-stack noalloc noexec nowrite progbits