; ft_strcmp(char *s1, const char *s2)
global ft_strcmp

section .text
	ft_strcmp:
		push rbp
		mov rbp, rsp
	
	.loop:
		mov al, [rdi]
		cmp al, [rsi]
		jne .dif
		inc rdi
		inc rsi
		test al, al
		jnz .loop

		jmp .equal
		jmp .done

	.equal:
		xor eax, eax

	.dif:
		movzx eax, byte [rdi]
		movzx ecx, byte [rsi]
		ja .greater
		mov eax, -1
		jmp .done

	.greater:
		mov eax, 1
		jmp .done

	.done:
		pop rbp
		ret