/*
 * Public Domain 2003 Dale Rahn
 *
 */
#include <stdio.h>
#include <dlfcn.h>


void ad(void);
extern int libglobal;

void (*ad_f)(void) = &ad;
int *a = &libglobal;
int
main()
{

	ad_f();

	return 1;
}
