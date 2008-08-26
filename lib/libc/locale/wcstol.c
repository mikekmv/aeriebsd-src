
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include "wctoint.h"

#define	FUNCNAME	wcstol
typedef long int_type;
#define	MIN_VALUE	LONG_MIN
#define	MAX_VALUE	LONG_MAX

#include "_wcstol.h"
