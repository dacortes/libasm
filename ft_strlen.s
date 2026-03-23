; -----------------------------------------------------------------------------
; ft_strlen
; -----------------------------------------------------------------------------
; Computes the length of a null-terminated string.
;
; Parameters:
;   rdi -> Pointer to the input string.
; Returns:
;   rax -> Number of bytes before the terminating null byte.
; Notes:
;   This routine performs a byte-by-byte scan and mirrors the behavior of
;   the C standard library strlen function for valid pointers.
; -----------------------------------------------------------------------------
; ft_strlen(const char *str)
global ft_strlen

section .text
	ft_strlen:
		; Start at -1 so the first increment positions the index at byte 0.
		mov rax, -1

	.loop:
		inc rax
		cmp byte[rdi + rax], 0
		jne .loop
		ret

section .note.GNU-stack noalloc noexec nowrite progbits
