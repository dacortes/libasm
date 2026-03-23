; -----------------------------------------------------------------------------
; ft_strcpy
; -----------------------------------------------------------------------------
; Copies a null-terminated string from src to dest, including the final
; terminator, and returns the destination pointer.
;
; Parameters:
;   rdi -> Destination buffer.
;   rsi -> Source string.
; Returns:
;   rax -> Original destination pointer.
; Notes:
;   The destination buffer must be large enough to hold the copied string.
; -----------------------------------------------------------------------------
; ft_strcpy(char *dest, const char *src)
global ft_strcpy

section .text
	ft_strcpy:
		push rbp
		mov rbp, rsp
		; Keep the original destination pointer for the return value.
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