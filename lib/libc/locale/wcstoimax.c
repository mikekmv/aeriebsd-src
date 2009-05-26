
#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include "wctoint.h"

#define	FUNCNAME	wcstoimax
typedef intmax_t	int_type;
#define	MIN_VALUE	INTMAX_MIN
#define	MAX_VALUE	INTMAX_MAX

#include "_wcstol.h"
