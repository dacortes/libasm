; ft_strlen(const char *str)
global ft_strlen

section .text
	ft_strlen:
		mov rax, -1

	.loop:
		inc rax
		cmp byte[rdi + rax], 0
		jne .loop
		ret

section .note.GNU-stack noalloc noexec nowrite progbits
