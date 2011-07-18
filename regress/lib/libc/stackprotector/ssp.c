
/* Written by Michael Shalayeff. Public Domain */

void
f(void)
{
	char buf[10];
	int i;

	for (i = 100; i--; )
		buf[i] = 0xff;
}

int
main()
{
	f();
}
