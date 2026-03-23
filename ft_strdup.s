global ft_strdup
extern ft_strlen
extern ft_strcpy
extern malloc

section .text
	ft_strdup:
		push rbp
		mov rbp, rsp
		push rbx

		mov rbx, rdi
		call ft_strlen

		inc rax

		mov rdi, rax

		call malloc wrt ..plt
		test rax, rax
		jz .error

		mov rdi, rax
		mov rsi, rbx
		call ft_strcpy

		jmp .done

	.error:
		xor rax, rax

	.done:
		pop rbx
		pop rbp
		ret

section .note.GNU-stack noalloc noexec nowrite progbits