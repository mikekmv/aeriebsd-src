
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

#define	FUNCNAME	wcstoumax
typedef uintmax_t	uint_type;
#define	MAX_VALUE	UINTMAX_MAX

#include "_wcstoul.h"
