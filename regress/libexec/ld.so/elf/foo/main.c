/*
 * Public domain. 2002, Matthieu Herrb
 *
 */
#include <stdio.h>
#include "elfbug.h"

int (*func)(void) = uninitialized;

int
main(int argc, char *argv[])
{
	fooinit();
	return (*func)();
}

