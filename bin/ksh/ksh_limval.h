
/* Wrapper around the values.h/limits.h includes/ifdefs */

/* limits.h is included in sh.h */

#ifndef DMAXEXP
# define DMAXEXP	128	/* should be big enough */
#endif

#ifndef BITS
# define BITS(t)	(CHAR_BIT * sizeof(t))
#endif
