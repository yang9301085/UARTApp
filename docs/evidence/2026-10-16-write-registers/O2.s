	.file	"<stdin>"
	.intel_syntax noprefix
	.text
	.p2align 4
	.globl	inspect_write
	.type	inspect_write, @function
inspect_write:
	mov	eax, 1
#APP
# 30 "<stdin>" 1
	syscall
	
# 0 "" 2
#NO_APP
	ret
	.size	inspect_write, .-inspect_write
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
