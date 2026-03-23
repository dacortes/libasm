; -----------------------------------------------------------------------------
; ft_strcmp
; -----------------------------------------------------------------------------
; Compares two null-terminated strings byte by byte using unsigned semantics.
;
; Parameters:
;   rdi -> First string.
;   rsi -> Second string.
; Returns:
;   eax -> Zero when strings are equal, a positive value when s1 > s2, or a
;          negative value when s1 < s2.
; Notes:
;   The comparison stops at the first differing byte or at the null terminator.
; -----------------------------------------------------------------------------
global ft_strcmp

section .text
	ft_strcmp:
		push rbp
		mov rbp, rsp

	.loop:
		; Load and compare the current bytes from both strings.
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
		; Re-load the bytes as unsigned values to build the exact difference.
		movzx eax, byte [rdi]
		movzx ecx, byte [rsi]
		sub eax, ecx

	.done:
		pop rbp
		ret

section .note.GNU-stack noalloc noexec nowrite progbits