
#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#define FUNCNAME	wcstof
typedef float		float_type;
#define STRTOD_FUNC	strtof

#include "_wcstod.h"
