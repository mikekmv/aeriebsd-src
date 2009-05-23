
#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: alloca.S,v 1.1.1.1 2008/08/26 14:38:16 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(alloca)
	popq	%rdx
	movq	%rsp,%rcx
	addq	$15,%rdi	/*  round up to 16 bytes */
	andq	$~15,%rdi
	subq	%rdi,%rsp
	movq	%rsp,%rax
	jmp	*%rdx
