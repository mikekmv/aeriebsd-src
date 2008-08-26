/*
 *	Written by Artur Grabowski <art@openbsd.org>, 2002 Public Domain.
 */
void foo(void) __attribute__((constructor));

int constructed = 0;

void
foo(void)
{
	constructed = 1;
}

int
main(int argc, char *argv[])
{
	return !constructed;
}
