; ft_strcmp - Versión simplificada y correcta
global ft_strcmp

	section .text
	ft_strcmp:
		push rbp
		mov rbp, rsp

	.loop:
		mov al, [rdi]
		cmp al, [rsi]
		jne .diff
		test al, al
		jz .equal
		inc rdi
		inc rsi
		jmp .loop

	.equal:
		xor eax, eax
		jmp .done

	.diff:
		movzx eax, byte [rdi]
		movzx ecx, byte [rsi]
		sub eax, ecx

	.done:
		pop rbp
		ret