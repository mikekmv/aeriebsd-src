/*
 * Copyright (c) 2009 Michael Shalayeff
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef lint
static const char rcsid[] = "$ABSD: ld.c,v 1.3 2009/10/23 21:28:03 mickey Exp $";
#endif

#include <sys/param.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <elf_abi.h>
#include <elfuncs.h>
#include <a.out.h>
#include <ar.h>
#include <ranlib.h>
#include <getopt.h>
#include <paths.h>
#include <errno.h>
#include <err.h>

#include "ld.h"

TAILQ_HEAD(, pathlist) libdirs = TAILQ_HEAD_INITIALIZER(libdirs);
TAILQ_HEAD(, objlist) objlist = TAILQ_HEAD_INITIALIZER(objlist);

/*
 * this object constain all the generated symbols and sections;
 */
struct objlist sysobj;

int endian = ELFDATANONE;
int elfclass = ELFCLASSNONE;
int machine = EM_NONE;
int magic = ZMAGIC;
int as_needed;
int Bflag;	/* 0 - static, 1 - dynamic, 2 - shlib */
int check_sections = 1;
int cref;
int nostdlib;
int pie;
int warncom;
int trace;	/* trace objects being loaded */
int relocatable;/* produce relocatable output */
int strip;	/* 0 - do not, 1 - strip debug, 2 - strip all */
int errors;	/* non-fatal errors accumulated */
int printmap;	/* print edit map to stdout */
const char *entry_name;
struct symlist *sentry;

#define OPTSTRING       "+c:C:dD:e:Ef:F:h:il:L:m:MnNo:OqrR:sStT:u:vVxXy:Y:z:Z"
const struct option longopts[] = {
	{ "as-needed",		no_argument,	&as_needed, 1 },
	{ "no-as-needed",	no_argument,	&as_needed, 0 },
	{ "check-sections",	no_argument,	&check_sections, 1 },
	{ "no-check-sections",	no_argument,	&check_sections, 0 },
	{ "cref",		no_argument,	&cref, 1 },
	{ "defsym",		required_argument,	0, 'D' },
	{ "nostdlib",		no_argument,	&nostdlib, 1 },
	{ "pie",		no_argument,	&pie, 1 },
	{ "pic-executable",	no_argument,	&pie, 1 },
/*	{ "rpath",		required_argument,	0, '???' }, */
	{ "section-start",	required_argument,	0, 0 },
	{ "Ttext",		required_argument,	0, 0 },
	{ "Tdata",		required_argument,	0, 0 },
	{ "Tbss",		required_argument,	0, 0 },
	{ "warn-common",	no_argument,	&warncom, 1 },
	{ "Bshared",		no_argument,	&Bflag, 2 },
	{ "shared",		no_argument,	&Bflag, 2 },
	{ "Bstatic",		no_argument,	&Bflag, 0 },
	{ "static",		no_argument,	&Bflag, 0 },
	{ "Bdynamic",		no_argument,	&Bflag, 1 },
	{ "dc",			no_argument,		0, 'd' },
	{ "dn",			no_argument,	&Bflag, 0 },
	{ "dp",			no_argument,		0, 'd' },
	{ "dy",			no_argument,	&Bflag, 1 },
	{ "entry",		required_argument,	0, 'e' },
	{ "export-dynamic",	no_argument,		0, 'E' },
	{ "EB",			no_argument,	&endian, ELFDATA2MSB },
	{ "EL",			no_argument,	&endian, ELFDATA2LSB },
	{ "auxiliary",		required_argument,	0, 'f' },
	{ "filter",		required_argument,	0, 'F' },
	{ "fini",		required_argument,	0, 'c' },
	{ "soname",		required_argument,	0, 'h' },
	{ "init",		required_argument,	0, 'C' },
	{ "library",		required_argument,	0, 'l' },
	{ "library-path",	required_argument,	0, 'L' },
	{ "print-map",		no_argument,		0, 'M' },
	{ "nmagic",		no_argument,	&magic, NMAGIC },
	{ "omagic",		no_argument,	&magic, OMAGIC },
	{ "output",		required_argument,	0, 'o' },
	{ "emit-relocs",	no_argument,		0, 'q' },
	{ "relocatable",	no_argument,		0, 'r' },
	{ "just-symbols",	required_argument,	0, 'R' },
	{ "strip-all",		no_argument,		0, 's' },
	{ "strip-debug",	no_argument,		0, 'S' },
	{ "trace",		no_argument,		0, 't' },
	{ "script",		required_argument,	0, 'T' },
	{ "undefined",		required_argument,	0, 'u' },
	{ "version",		no_argument,		0, 'v' },
	{ "discard-all",	no_argument,		0, 'x' },
	{ "discard-locals",	no_argument,		0, 'X' },
	{ "trace-symbol",	required_argument,	0, 'y' },
	{ NULL }
};

const struct ldarch ldarchs[] = {
/*	{ EM_VAX, 0, vax_order, vax_fix }, */
/*	{ EM_ALPHA, 0, alpha_order, alpha_fix }, */
	{ EM_386, 0, i386_order, i386_fix },
/*	{ EM_AMD64, 0, amd64_order, amd64_fix }, */
/*	{ EM_MIPS, 0, mips_order, mips_fix }, */
/*	{ EM_MIPS64, 0, mips64_order, mips64_fic }, */
/*	{ EM_PARISC, 0, hppa_order, hppa_fix }, */
/*	{ EM_PARISC64, 0, hppa64_order, hppa64_fix }, */
/*	{ EM_PPC, 0, ppc_order, ppc_fix }, */
/*	{ EM_PPC64, 0, ppc64_order, ppc64_fix }, */
/*	{ EM_SPARC, 0, sparc_order, sparc_fix }, */
/*	{ EM_SPARC64, 0, sparc64_order, sparc64_fix }, */
/*	{ EM_SH, 0, sh_order, sh_fix }, */
/*	{ EM_ARM, 0, arm_order, arm_fix }, */
/*	{ EM_68K, 0, m68k_order, m68k_fix }, */
};
const int ldnarch = sizeof(ldarchs)/sizeof(ldarchs[0]);
const struct ldarch *ldarch;

int usage(void);
int libdir_add(const char *);
int obj_add(const char *, const char *, FILE *, off_t);
int lib_add(const char *);
int lib_namtab(const char *, FILE *, u_long, off_t, u_long);
int lib_symdef(const char *, FILE *, u_long);
int mmbr_name(struct ar_hdr *, char **, int, int *, FILE *);

int
usage(void)
{
	extern char *__progname;
	fprintf(stderr, "usage: %s [file]\n", __progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char output[MAXPATHLEN];
	FILE *fp;
	int ch, li;

	strlcpy(output, "a.out", sizeof output);
	libdir_add(_PATH_USRLIB);

	errors = 0;
	while ((ch = getopt_long(argc, argv, OPTSTRING, longopts, &li)) != -1)
		switch (ch) {
		case 0:
			/* check out 'li' to see what matched */
			break;

		case 'c':	/* fini symbol */
			break;

		case 'C':	/* init symbol */
			break;

		case 'D':	/* define symbol */
			break;

		case 'd':	/* FORCE_COMMON_ALLOCATION */
			break;

		case 'e':	/* entry */
			entry_name = optarg;
			break;

		case 'E':	/* export dynamic symbols */
			break;

		case 'f':	/* add name to DT_AUXILIARY list */
			break;

		case 'F':	/* add name to DT_FILTER list */
			break;

		case 'h':	/* set the DT_SONAME field */
			break;

		case 'i':	/* perform incremental linking */
		case 'r':
			relocatable++;
			break;

		case 'L':	/* add to the library search path */
		case 'Y':
			libdir_add(optarg);
			break;

		case 'l':
			/* end of options proceede with loading */
			break;

		case 'M':	/* print linking map to stdout */
			printmap++;
			break;

		case 'n':	/* make NMAGIC output */
			magic = NMAGIC;
			break;

		case 'N':	/* make OMAGIC output */
			magic = OMAGIC;
			break;

		case 'o':	/* set output file format */
			break;

		case 'O':	/* optimise the otput binary (slow) */
			break;

		case 'q':	/* leave the relocs in after full linking */
			break;

		case 'R':	/* read in only symbols from this file(s) */
			break;

		case 'S':	/* strip only debugging info from the output */
			strip = 1;
			break;

		case 's':	/* strip all symbol info from the otput */
			strip = 2;
			break;

		case 't':	/* trace files processed */
			trace++;
			break;

		case 'T':	/* read the linking script from file */
			break;

		case 'u':	/* enter an undefined symbol */
			sym_undef(optarg);
			break;

		case 'v':
		case 'V':	/* print ld(1) version */
			break;

		case 'x':	/* discard all local symbols */
			break;

		case 'X':	/* discard only local symbols */
			break;

		case 'y':	/* trace all files for this symbol */
			break;

		case 'z':	/* special options */
			break;

		case 'Z':	/* make ZMAGIC output */
			magic = ZMAGIC;
			break;

		default:
			usage();
			break;
		}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		errx(1, "no input files");

	/* make a "system" object for our self-defined syms */
	sysobj.ol_path = output;
	TAILQ_INIT(&sysobj.ol_syms);
	/* allocate sections (.got .plt etc) */
	if (Bflag || pie) {
		sysobj.ol_nsect = 2;
		if (!(sysobj.ol_sections = calloc(sysobj.ol_nsect,
		    sizeof *sysobj.ol_sections)))
			err(1, "calloc");
		if (!(sysobj.ol_sects = calloc(sysobj.ol_nsect,
		    MAX(sizeof(Elf32_Shdr), sizeof(Elf64_Shdr)))))
			err(1, "calloc");
		/* os_sect is set in ldmap() */
		sysobj.ol_sections[0].os_obj = &sysobj;
		sysobj.ol_sections[1].os_obj = &sysobj;
	}
	TAILQ_INSERT_TAIL(&objlist, &sysobj, ol_entry);

	for (; argc--; argv++) {
		if (**argv == '-')
			switch (argv[0][1]) {
			case 'L':
				libdir_add(*argv + 2);
				continue;

			case 'l':	/* load the archive */
				lib_add(*argv);
				continue;

			default:
				usage();
				break;
			}

		if (!(fp = fopen(*argv, "r")))
			err(1, "fopen: %s", *argv);

		if (trace)
			printf("%s\n", *argv);

		if (obj_add(*argv, NULL, fp, 0))
			return 1;
		fclose(fp);
	}

	if (errors)
		return 1;

	/* and now do the jobs */
	if (elfclass == ELFCLASS32)
		return ldload32(output, ldmap32(ldarch));
	else
		return ldload64(output, ldmap64(ldarch));
}

/*
 * pre-scan thru the ldarch on the first encountered object
 * in order to generate necessary symbols and other gedoens
 */
const struct ldarch *
ldinit(void)
{
	const struct ldarch *lda;
	const struct ldorder *order;
	int i;

	for (lda = ldarchs, i = 0; i < ldnarch; lda++, i++)
		if (lda->la_mach == machine)
			break;

	if (i >= ldnarch) {
		warn("unknown machine %d", machine);
		return NULL;
	}

	for (i = 0, order = lda->la_order;
	    order->ldo_order != ldo_kaput; order++, i++) {
		/* XXX we have to check ldo_flags as in ldmap() */

		switch (order->ldo_order) {
		case ldo_symbol:
			if (order->ldo_flags & LD_ENTRY) {
				if (!entry_name)
					entry_name = order->ldo_name;
				sentry = sym_undef(entry_name);
				break;
			}
			break;
		}
	}

	return lda;
}

/*
 * add another library search dir (such as from -L)
 */
int
libdir_add(const char *path)
{
	char pathbuf[MAXPATHLEN];
	struct stat sb;
	struct pathlist *pl;
	size_t len;

	if (stat(path, &sb) < 0)
		err(1, "stat: %s", path);

	if (!S_ISDIR(sb.st_mode))
		errx(1, "%s: not a directory", path);

	if ((pl = calloc(1, sizeof *pl)) == NULL)
		err(1, "calloc");

	len = strlen(path);
	if (path[len - 1] != '/') {
		snprintf(pathbuf, sizeof pathbuf, "%s/", path);
		path = pathbuf;
		len++;
	}
	if ((pl->pl_path = strdup(path)) == NULL)
		err(1, "strdup");

	TAILQ_INSERT_TAIL(&libdirs, pl, pl_entry);
	return 0;
}

char **nametab;

/*
 * search the library for objects resolving
 * currently undefined symbols;
 * if a library index (see ranlib(1)) is available
 * use that to pull needed objects and keep cycling
 * until no new symbols had been resolved;
 * otherwise scan whole library (once) checking
 * each member for symbols to resolve.
 */
int
lib_add(const char *path)
{
	char pathbuf[MAXPATHLEN];
	char armag[SARMAG];
	struct ar_hdr ah;
	off_t off, symoff;
	struct pathlist *pl;
	FILE *fp;
	char *p, *nt = NULL;
	u_long len, symlen;
	int nlen, ltrace = trace;

	if (path[0] == '-' && path[1] == 'l') {
		TAILQ_FOREACH(pl, &libdirs, pl_entry) {
			snprintf(pathbuf, sizeof pathbuf, "%slib%s.a",
			    pl->pl_path, path+2);
			if (ltrace) {
				printf("%s (%s)\n", path, pathbuf);
				ltrace = 0;
			}
			path = pathbuf;
			if (access(path, F_OK) < 0)
				continue;
			break;
		}
	}

	if (!(fp = fopen(path, "r")))
		err(1, "%s: fopen", pathbuf);

	if (ltrace)
		printf("%s\n", path);

	if (fread(armag, sizeof armag, 1, fp) != 1 ||
	    strncmp(armag, ARMAG, SARMAG))
		errx(1, "%s: not an archive", path);

	symoff = 0;
	symlen = 0;
	nametab = NULL;
	nt = NULL;
	while (fread(&ah, sizeof ah, 1, fp) == 1) {
		if (memcmp(ah.ar_fmag, ARFMAG, sizeof ah.ar_fmag))
			errx(1, "%s: invalid archive header", path);

		off = ftello(fp);
		errno = 0;
		len = strtoul(ah.ar_size, NULL, 0);
		if (errno == ERANGE && len == ULONG_MAX)
			errx(1, "broken archive header");

		if (!strncmp(ah.ar_name, AR_NAMTAB, sizeof(AR_NAMTAB) - 1)) {
			char **pn, *pp;
			int i;

			/* un-ranliebt archive will do it the hard way */
			if (!symoff || !symlen)
				continue;
			/*
			 * read the name table first;
			 * optimise for duplicate libs and cache the names
			 */

			if (!(nt = malloc(len + 1)))
				err(1, "namtab malloc");

			if (fread(nt, len, 1, fp) != 1)
				err(1, "%s: fread", path);

			nt[len] = '\0';
			for (p = nt, i = 0; *p; )
				if (*p++ == '\n')
					i++;

			/* XXX check overflow */
			if (!(nametab = malloc((i + 1) * sizeof *nametab)))
				err(1, "nameidx malloc");

			for (pn = nametab, p = pp = nt; *p; p++)
				if (*p == '\n') {
					*pn++ = pp;
					if (p[-1] == '/')
						p[-1] = '\0';
					*p++ = '\0';
					pp = p;
				}
			*pn = NULL;

			if (lib_namtab(path, fp, len, symoff, symlen))
				exit(1);
			else
				break;
		}

		if (!strncmp(ah.ar_name, RANLIBMAG2, sizeof(RANLIBMAG2) - 1)) {
			symoff = off;
			symlen = len;
			if (fseeko(fp, off + len, SEEK_SET) < 0)
				err(1, "fseeko: %s", path);
			continue;
		}

		if (!strncmp(ah.ar_name, RANLIBMAG, sizeof(RANLIBMAG) - 1)) {
			if (lib_namtab(path, fp, len, symoff, symlen))
				exit(1);
			else
				break;
		}

		/* see if we have a symdef but no namtab */
		if (symoff) {
			if (lib_namtab(path, fp, len, symoff, symlen))
				exit(1);
			else
				break;
		}

		/*
		 * no symdef
		 * try to match symbols from every obj
		 * (vewy zlou)
		 */

		nlen = sizeof ah.ar_name;
		if (!(p = malloc(nlen)))
			err(1, "malloc");
		*p = '\0';
		if (mmbr_name(&ah, &p, 0, &nlen, fp))
			exit(1);

		errx(1, "no symdef: not implemented");
	}

	if (nametab) {
		free(nametab);
		free(nt);
	}
	fclose(fp);
	return 0;
}

/*
 * scan old (a.out/bsd) symdef index
 */
int
lib_symdef(const char *path, FILE *fp, u_long len)
{
	warnx("old symdef: not implemented");
	return -1;
}

/*
 * scan the new (sysv/elf) library index
 */
int
lib_namtab(const char *path, FILE *fp, u_long len, off_t symoff, u_long symlen)
{
	char *p, *pp;
	uint32_t num, *offs;
	int i, more;

	if (fseeko(fp, symoff, SEEK_SET) < 0)
		err(1, "fseeko: %s", path);

	if (!(pp = malloc(symlen)))
		err(1, "symdef malloc");

	if (fread(pp, symlen, 1, fp) != 1)
		err(1, "fread: %s", path);

	offs = (uint32_t *)pp;
	num = betoh32(*offs++);

	more = 1;
	while (more) {
		more = 0;
		for (i = 0, p = pp + (num + 1) * sizeof *offs;
		    i < num; i++, p += strlen(p) + 1) {
			struct ar_hdr mh;
			off_t foff;
			char *name;
			int nlen;

			if (!sym_isundef(p))
				continue;

			foff = ntohl(offs[i]);
			if (fseeko(fp, foff, SEEK_SET) < 0)
				err(1, "fseeko: %s", path);
			if (fread(&mh, sizeof mh, 1, fp) != 1)
				err(1, "fread: %s", path);
			if (memcmp(mh.ar_fmag, ARFMAG,
			    sizeof mh.ar_fmag))
				errx(1, "%s: invalid ar", path);
			nlen = sizeof mh.ar_name;
			if (!(name = malloc(nlen)))
				err(1, "malloc");
			*name = '\0';
			if (mmbr_name(&mh, &name, 0, &nlen, fp))
				return -1;

			/*
			 * this can be optimised by checking
			 * inside the obj_add if we added
			 * any new undef syms
			 */
			more = 1;
			obj_add(path, name, fp, foff + sizeof mh);
			free(name);
		}
	}
	free(pp);

	return 0;
}

/*
 *	given the archive member header -- produce member name
 *	(pwnz0red shamelessly from nm.c)
 */
int
mmbr_name(struct ar_hdr *arh, char **name, int baselen, int *namelen, FILE *fp)
{
	char *p = *name + strlen(*name);
	long i;

	if (nametab && arh->ar_name[0] == '/') {
		int len;

		i = atol(&arh->ar_name[1]);
		len = strlen(nametab[i]) + 1;
		if (len > *namelen) {
			p -= (long)*name;
			if ((*name = realloc(*name, baselen+len)) == NULL)
				err(1, "realloc");
			*namelen = len;
			p += (long)*name;
		}
		strlcpy(p, nametab[i], len);
		return 0;
	} else
#ifdef AR_EFMT1
	/*
	 * BSD 4.4 extended AR format: #1/<namelen>, with name as the
	 * first <namelen> bytes of the file
	 */
	if ((arh->ar_name[0] == '#') &&
	    (arh->ar_name[1] == '1') &&
	    (arh->ar_name[2] == '/') &&
	    (isdigit(arh->ar_name[3]))) {
		int len = atoi(&arh->ar_name[3]);

		if (len > *namelen) {
			p -= (long)*name;
			if ((*name = realloc(*name, baselen+len)) == NULL)
				err(1, "realloc");
			*namelen = len;
			p += (long)*name;
		}
		if (fread(p, len, 1, fp) != 1) {
			warnx("%s: premature EOF", *name);
			free(*name);
			return(1);
		}
		p += len;
	} else
#endif
	for (i = 0; i < sizeof(arh->ar_name); ++i)
		if (arh->ar_name[i] && arh->ar_name[i] != ' ')
			*p++ = arh->ar_name[i];
	*p = '\0';
	if (p[-1] == '/')
		*--p = '\0';

	return (0);
}

/*
 * scan through the global objects list
 * calling a function for each of which
 */
int
obj_foreach(int (*func)(struct objlist *, void *), void *v)
{
	struct objlist *ol;

	TAILQ_FOREACH(ol, &objlist, ol_entry) {
		if ((*func)(ol, v))
			return 1;
	}

	return 0;
}

/*
 * load an object from path (or path(name) for archive)
 * resolving undefined symbols an fetcing all info
 * needed for the second pass (mapping)
 */
int
obj_add(const char *path, const char *name, FILE *fp, off_t foff)
{
	off_t ofoff;
	struct objlist *ol;

	if ((ol = calloc(1, sizeof *ol)) == NULL)
		err(1, "calloc");

	if ((ol->ol_path = strdup(path)) == NULL)
		err(1, "strdup");

	ofoff = 0;
	if (foff) {
		ofoff = ftello(fp);
		ol->ol_off = foff;
		if (fseeko(fp, foff, SEEK_SET) < 0)
			err(1, "fseeko: %s", path);
	}

	/* gotta be ar member */
	if (name) {
		char b[MAXPATHLEN];
		snprintf(b, sizeof b, "%s(%s)", path, name);
		if (!(ol->ol_name = strdup(b)))
			err(1, "strdup");
	} else
		ol->ol_name = ol->ol_path;

	if (fread(&ol->ol_hdr, sizeof ol->ol_hdr, 1, fp) != 1)
		err(1, "fread header: %s", ol->ol_name);

	TAILQ_INIT(&ol->ol_syms);
	if (IS_ELF(ol->ol_hdr.elf32) &&
	    ol->ol_hdr.elf32.e_ident[EI_CLASS] == ELFCLASS32 &&
	    ol->ol_hdr.elf32.e_ident[EI_VERSION] == ELF_TARG_VER) {

		if (elf32_objadd(ol, fp, foff))
			exit(1);

	} else if (IS_ELF(ol->ol_hdr.elf64) &&
	    ol->ol_hdr.elf64.e_ident[EI_CLASS] == ELFCLASS64 &&
	    ol->ol_hdr.elf64.e_ident[EI_VERSION] == ELF_TARG_VER) {

		if (elf64_objadd(ol, fp, foff))
			exit(1);
#if 0
	} else if (!BAD_OBJECT(ol->ol_hdr.aout)) {
		/* a.out goes here */
#endif
	} else
		errx(1, "%s: bad format", path);

	TAILQ_INSERT_TAIL(&objlist, ol, ol_entry);
	fseeko(fp, ofoff, SEEK_SET);
	return 0;
}
