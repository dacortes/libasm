; ft_strcpy(char *dest, const char *src)
global ft_strcpy

section .text
	ft_strcpy:
		push rbp
		mov rbp, rsp
		mov rcx, 0


		mov rax, rdi

;		pop rbp
;		ret

	.loop:
		inc rcx
		cmp byte[rdi + rcx], 0
		jz .break
		jmp .loop
	
	.break:
		pop rbp
		mov rax, rsi
		ret
