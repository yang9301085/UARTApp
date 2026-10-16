	.file	"<stdin>"
	.intel_syntax noprefix
	.text
	.globl	inspect_write
	.type	inspect_write, @function
inspect_write:
	push	rbp
	mov	rbp, rsp
	mov	DWORD PTR -36[rbp], edi
	mov	QWORD PTR -48[rbp], rsi
	mov	QWORD PTR -56[rbp], rdx
	mov	rax, QWORD PTR -56[rbp]
	mov	QWORD PTR -8[rbp], rax
	mov	rax, QWORD PTR -48[rbp]
	mov	QWORD PTR -16[rbp], rax
	mov	eax, DWORD PTR -36[rbp]
	mov	DWORD PTR -20[rbp], eax
	mov	rdx, QWORD PTR -8[rbp]
	mov	rsi, QWORD PTR -16[rbp]
	mov	edi, DWORD PTR -20[rbp]
	mov	eax, 1
#APP
# 30 "<stdin>" 1
	syscall
	
# 0 "" 2
#NO_APP
	mov	QWORD PTR -32[rbp], rax
	mov	rax, QWORD PTR -32[rbp]
	pop	rbp
	ret
	.size	inspect_write, .-inspect_write
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
