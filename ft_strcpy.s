; ft_strcpy(char *dest, const char *src)
global ft_strcpy

section .text
	ft_strcpy:
		push rbp
		mov rbp, rsp
		mov rax, rdi
		xor rcx, rcx

	.loop:
		mov dl, [rsi + rcx]
		mov [rdi + rcx], dl
		cmp dl, 0
		je .done
		inc rcx
		jmp .loop

	.done:
		pop rbp
		ret

section .note.GNU-stack noalloc noexec nowrite progbits