; -----------------------------------------------------------------------------
; ft_read
; -----------------------------------------------------------------------------
; Invokes the Linux read system call and reproduces libc-style errno handling
; on failure.
;
; Parameters:
;   rdi -> File descriptor.
;   rsi -> Destination buffer.
;   rdx -> Maximum number of bytes to read.
; Returns:
;   rax -> Number of bytes read, or -1 when the syscall fails.
; Notes:
;   When the syscall returns a negative error code, the routine calls
;   __errno_location to store the positive errno value before returning -1.
; -----------------------------------------------------------------------------
global ft_read
extern __errno_location
; __errno_location returns a thread-local pointer used to update errno.

section .text
	ft_read:
		push rbp
		mov rbp, rsp
		push rbx
		push r12
		; Preserve non-volatile registers used across the syscall and PLT call.

		; Linux x86-64 syscall number 0 = read.
		mov rax, 0
		syscall

		cmp rax, 0
		jl .error

		pop r12
		pop rbx
		pop rbp
		ret
	
	.error:
		; Save the negative kernel error code before resolving errno storage.
		mov r12, rax
		call __errno_location wrt ..plt
		mov rcx, r12
		neg ecx
		mov [rax], ecx
		mov rax, -1
		
		pop r12
		pop rbx
		pop rbp
		ret

section .note.GNU-stack noalloc noexec nowrite progbits