
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include "wctoint.h"

#define	FUNCNAME	wcstoull
typedef unsigned long long int uint_type;
#define	MAX_VALUE	ULLONG_MAX

#include "_wcstoul.h"
