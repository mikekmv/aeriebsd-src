
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include "wctoint.h"

#define	FUNCNAME	wcstoll
typedef long long int int_type;
#define	MIN_VALUE	LLONG_MIN
#define	MAX_VALUE	LLONG_MAX

#include "_wcstol.h"
