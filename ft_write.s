; -----------------------------------------------------------------------------
; ft_write
; -----------------------------------------------------------------------------
; Invokes the Linux write system call and reproduces libc-style errno handling
; on failure.
;
; Parameters:
;   rdi -> File descriptor.
;   rsi -> Buffer to write.
;   rdx -> Number of bytes to write.
; Returns:
;   rax -> Number of bytes written, or -1 when the syscall fails.
; Notes:
;   When the syscall returns a negative error code, the routine calls
;   __errno_location to store the positive errno value before returning -1.
; -----------------------------------------------------------------------------
global ft_write
extern __errno_location
; __errno_location returns a thread-local pointer used to update errno.

section .text
	ft_write:
		push rbp
		mov rbp, rsp
		push rbx
		push r12
		; Preserve non-volatile registers used across the syscall and PLT call.
		
		; Linux x86-64 syscall number 1 = write.
		mov rax, 1
		syscall
		
		test rax, rax
		js .error
		
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