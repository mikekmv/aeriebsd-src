
/* libmain - flex run-time support library "main" function */

#include <sys/cdefs.h>

int yylex(void);
int main(int, char **);

/* ARGSUSED */
int
main(int argc, char *argv[])
{
	while (yylex() != 0)
		;

	return 0;
}
