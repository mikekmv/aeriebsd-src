
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ohash.h"

struct _ohash_record {
	uint32_t	hv;
	const char 	*p;
};

#define DELETED		((const char *)h)
#define NONE		(h->size)

/* Don't bother changing the hash table if the change is small enough.  */
#define MINSIZE		(1UL << 4)
#define MINDELETED	4
