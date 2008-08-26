/* David Leonard, <d@csee.uq.edu.au>. Public domain. */

#include <machine/npx.h>

struct _machdep_state {
	int		esp;
	int		pad[3];
	union savefpu	fpreg;	/* must be 128-bit aligned */
};
