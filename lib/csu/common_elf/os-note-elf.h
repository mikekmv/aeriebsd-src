/*
 * Contents:
 *
 *  long Name length
 *  long Description length
 *  long ELF_NOTE_TYPE_OSVERSION (1) XXX - need a define.
 *  "AerieBSD\0"
 *  version? 0 XXX
 */

__asm("	.section \".note.aeriebsd.ident\", \"a\"\n"
"	.p2align 2\n"
"	.long	16\n"
"	.long	4\n"
"	.long	1\n"
"	.ascii \"AerieBSD\\0\"\n"
"	.p2align 4\n"
"	.long	0\n"
"	.p2align 2");
