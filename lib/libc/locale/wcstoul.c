
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include "wctoint.h"

#define	FUNCNAME	wcstoul
typedef unsigned long uint_type;
#define	MAX_VALUE	ULONG_MAX

#include "_wcstoul.h"
