; -----------------------------------------------------------------------------
; ft_strdup
; -----------------------------------------------------------------------------
; Allocates a new buffer large enough to hold a copy of the input string, then
; duplicates the contents into the new allocation.
;
; Parameters:
;   rdi -> Source string to duplicate.
; Returns:
;   rax -> Pointer to the duplicated string, or NULL when allocation fails.
; Notes:
;   The implementation delegates the length calculation to ft_strlen, requests
;   memory with malloc, and copies the bytes through ft_strcpy.
; -----------------------------------------------------------------------------
global ft_strdup
extern ft_strlen
; ft_strlen computes the number of bytes required for the source string.
extern ft_strcpy
; ft_strcpy copies the source bytes into the allocated destination buffer.
extern malloc
; malloc reserves heap memory for the duplicated string.

section .text
	ft_strdup:
		push rbp
		mov rbp, rsp

		; Preserve the source pointer because ft_strlen consumes rdi.
		mov rbx, rdi
		call ft_strlen

		; Reserve one extra byte for the null terminator.
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
		pop rbp
		ret

section .note.GNU-stack noalloc noexec nowrite progbits