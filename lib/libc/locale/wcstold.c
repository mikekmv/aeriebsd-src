
#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#define FUNCNAME	wcstold
typedef long double	float_type;
#define STRTOD_FUNC	strtold

#include "_wcstod.h"
