#include <machine/asm.h>

	.text
LEAF(alpha_rpcc,1)
	rpcc	v0
	RET
	END(alpha_rpcc)
