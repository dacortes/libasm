; ft_strlen(const char *str)
global ft_strlen

section .text

	ft_strlen:
		mov rax, -1				; count

	.loop:
		inc rax
		cmp byte [rdi + rax], 0	; compare char for char if diferent 0
		jne .loop				; if bot is 0

		ret						; return rax

; global ft_strlen

; section .text

; ft_strlen:
; 	test rdi, rdi				; rdi == NULL?
; 	jz .null_pointer			; if is null, jum
; 	xor rax, rax
	
; .loop:
; 	cmp byte [rdi + rax], 0
; 	je .done
; 	inc rax
; 	jmp .loop
	
; .done:
; 	ret

; .null_pointer:
; 	xor rax, rax
; 	ret