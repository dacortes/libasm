global ft_write
extern __errno_location

ft_write:
	push rbp
	mov rbp, rsp
	push rbx
	push r12
	
	mov rax, 1
	syscall
	
	test rax, rax
	js .error
	
	pop r12
	pop rbx
	pop rbp
	ret
	
.error:
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
	