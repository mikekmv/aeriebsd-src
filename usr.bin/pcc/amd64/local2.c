/*	$Id: local2.c,v 1.1 2009/02/13 15:24:58 mickey Exp $	*/
/*
 * Copyright (c) 2008 Michael Shalayeff
 * Copyright (c) 2003 Anders Magnusson (ragge@ludd.luth.se).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

# include "pass2.h"
# include <ctype.h>
# include <string.h>

static int stkpos;

void
deflab(int label)
{
	printf(LABFMT ":\n", label);
}

static int regoff[7];
static TWORD ftype;

/*
 * Print out the prolog assembler.
 * addto and regoff are already calculated.
 */
static void
prtprolog(struct interpass_prolog *ipp, int addto)
{
	static int lwnr;
	int i;

	printf("\tpushq %%rbp\n");
	printf("\tmovq %%rsp,%%rbp\n");
	if (addto)
		printf("\tsubq $%d,%%rsp\n", addto);
	for (i = 0; i < MAXREGS; i++)
		if (TESTBIT(ipp->ipp_regs, i))
			fprintf(stdout, "\tmov %s,-%d(%s)\n",
			    rnames[i], regoff[i], rnames[FPREG]);
	if (kflag == 0)
		return;

	/* if ebx are not saved to stack, it must be moved into another reg */
	/* check and emit the move before GOT stuff */
	if (!TESTBIT(ipp->ipp_regs, EBX)) {
		struct interpass *ip = (struct interpass *)ipp;

		ip = DLIST_PREV(ip, qelem);
		ip = DLIST_PREV(ip, qelem);
		ip = DLIST_PREV(ip, qelem);
		if (ip->type != IP_NODE || ip->ip_node->n_op != ASSIGN ||
		    ip->ip_node->n_left->n_op != REG)
			comperr("prtprolog pic error");
		ip = (struct interpass *)ipp;
		ip = DLIST_NEXT(ip, qelem);
		if (ip->type != IP_NODE || ip->ip_node->n_op != ASSIGN ||
		    ip->ip_node->n_left->n_op != REG)
			comperr("prtprolog pic error2");
		printf("	movl %s,%s\n",
		    rnames[ip->ip_node->n_right->n_rval],
		    rnames[ip->ip_node->n_left->n_rval]);
		tfree(ip->ip_node);
		DLIST_REMOVE(ip, qelem);
	}
	printf("	call .LW%d\n", ++lwnr);
	printf(".LW%d:\n", lwnr);
	printf("	popl %%ebx\n");
	printf("	addl $_GLOBAL_OFFSET_TABLE_+[.-.LW%d], %%ebx\n", lwnr);
}

/*
 * calculate stack size and offsets
 */
static int
offcalc(struct interpass_prolog *ipp)
{
	int i, addto;

	addto = p2maxautooff;
	if (addto >= AUTOINIT/SZCHAR)
		addto -= AUTOINIT/SZCHAR;
	for (i = 0; i < MAXREGS; i++)
		if (TESTBIT(ipp->ipp_regs, i)) {
			addto += SZINT/SZCHAR;
			regoff[i] = addto;
		}
	return addto;
}

void
prologue(struct interpass_prolog *ipp)
{
	int addto;

	ftype = ipp->ipp_type;

#ifdef LANG_F77
	if (ipp->ipp_vis)
		printf("	.globl %s\n", ipp->ipp_name);
	printf("	.align 4\n");
	printf("%s:\n", ipp->ipp_name);
#endif
	/*
	 * We here know what register to save and how much to 
	 * add to the stack.
	 */
	addto = offcalc(ipp);
	prtprolog(ipp, addto);
}

void
eoftn(struct interpass_prolog *ipp)
{
	int i;

	if (ipp->ipp_ip.ip_lbl == 0)
		return; /* no code needs to be generated */

	/* return from function code */
	for (i = 0; i < MAXREGS; i++)
		if (TESTBIT(ipp->ipp_regs, i))
			fprintf(stdout, "	movl -%d(%s),%s\n",
			    regoff[i], rnames[FPREG], rnames[i]);

	/* struct return needs special treatment */
	if (ftype == STRTY || ftype == UNIONTY) {
		printf("	movl 8(%%ebp),%%eax\n");
		printf("	leave\n");
		printf("	ret $%d\n", 4);
	} else {
		printf("	leave\n");
	}
	printf("\t.size %s,.-%s\n", ipp->ipp_name,
	    ipp->ipp_name);
}

/*
 * add/sub/...
 *
 * Param given:
 */
void
hopcode(int f, int o)
{
	char *str;

	switch (o) {
	case PLUS:
		str = "add";
		break;
	case MINUS:
		str = "sub";
		break;
	case AND:
		str = "and";
		break;
	case OR:
		str = "or";
		break;
	case ER:
		str = "xor";
		break;
	default:
		comperr("hopcode2: %d", o);
		str = 0; /* XXX gcc */
	}
	printf("%s%c", str, f);
}

/*
 * Return type size in bytes.  Used by R2REGS, arg 2 to offset().
 */
int
tlen(NODE *p)
{
	switch(p->n_type) {
		case CHAR:
		case UCHAR:
			return(1);

		case SHORT:
		case USHORT:
			return(SZSHORT/SZCHAR);

		case DOUBLE:
			return(SZDOUBLE/SZCHAR);

		case INT:
		case UNSIGNED:
			return(SZINT/SZCHAR);

		case LONG:
		case ULONG:
		case LONGLONG:
		case ULONGLONG:
			return SZLONGLONG/SZCHAR;

		default:
			if (!ISPTR(p->n_type))
				comperr("tlen type %d not pointer");
			return SZPOINT(p->n_type)/SZCHAR;
		}
}

/*
 * Emit code to compare two longlong numbers.
 */
static void
twollcomp(NODE *p)
{
	int o = p->n_op;
	int s = getlab2();
	int e = p->n_label;
	int cb1, cb2;

	if (o >= ULE)
		o -= (ULE-LE);
	switch (o) {
	case NE:
		cb1 = 0;
		cb2 = NE;
		break;
	case EQ:
		cb1 = NE;
		cb2 = 0;
		break;
	case LE:
	case LT:
		cb1 = GT;
		cb2 = LT;
		break;
	case GE:
	case GT:
		cb1 = LT;
		cb2 = GT;
		break;
	
	default:
		cb1 = cb2 = 0; /* XXX gcc */
	}
	if (p->n_op >= ULE)
		cb1 += 4, cb2 += 4;
	expand(p, 0, "	cmpl UR,UL\n");
	if (cb1) cbgen(cb1, s);
	if (cb2) cbgen(cb2, e);
	expand(p, 0, "	cmpl AR,AL\n");
	cbgen(p->n_op, e);
	deflab(s);
}

int
fldexpand(NODE *p, int cookie, char **cp)
{
	CONSZ val;

	if (p->n_op == ASSIGN)
		p = p->n_left;
	switch (**cp) {
	case 'S':
		printf("%d", UPKFSZ(p->n_rval));
		break;
	case 'H':
		printf("%d", UPKFOFF(p->n_rval));
		break;
	case 'M':
	case 'N':
		val = (CONSZ)1 << UPKFSZ(p->n_rval);
		--val;
		val <<= UPKFOFF(p->n_rval);
		printf("0x%llx", (**cp == 'M' ? val : ~val) & 0xffffffff);
		break;
	default:
		comperr("fldexpand");
	}
	return 1;
}

static void
bfext(NODE *p)
{
	int ch = 0, sz = 0;

	if (ISUNSIGNED(p->n_right->n_type))
		return;
	switch (p->n_right->n_type) {
	case CHAR:
		ch = 'b';
		sz = 8;
		break;
	case SHORT:
		ch = 'w';
		sz = 16;
		break;
	case INT:
		ch = 'l';
		sz = 32;
		break;
	case LONG:
		ch = 'q';
		sz = 64;
		break;
	default:
		comperr("bfext");
	}

	sz -= UPKFSZ(p->n_left->n_rval);
	printf("\tshl%c $%d,", ch, sz);
	adrput(stdout, getlr(p, 'D'));
	printf("\n\tsar%c $%d,", ch, sz);
	adrput(stdout, getlr(p, 'D'));
	printf("\n");
}

/*
 * Push a structure on stack as argument.
 * the scratch registers are already free here
 */
static void
starg(NODE *p)
{
	FILE *fp = stdout;

	fprintf(fp, "	subl $%d,%%esp\n", p->n_stsize);
	fprintf(fp, "	pushl $%d\n", p->n_stsize);
	expand(p, 0, "	pushl AL\n");
	expand(p, 0, "	leal 8(%esp),A1\n");
	expand(p, 0, "	pushl A1\n");
	fprintf(fp, "	call memcpy\n");
	fprintf(fp, "	addl $12,%%esp\n");
}

/*
 * Compare two floating point numbers.
 */
static void
fcomp(NODE *p)  
{
	
	if (p->n_left->n_op == REG) {
		if (p->n_su & DORIGHT)
			expand(p, 0, "	fxch\n");
		expand(p, 0, "	fucompp\n");	/* emit compare insn  */
	} else if (p->n_left->n_type == DOUBLE)
		expand(p, 0, "	fcompl AL\n");	/* emit compare insn  */
	else if (p->n_left->n_type == FLOAT)
		expand(p, 0, "	fcomp AL\n");	/* emit compare insn  */
	else
		comperr("bad compare %p\n", p);
	expand(p, 0, "	fnstsw %ax\n");	/* move status reg to ax */
	
	switch (p->n_op) {
	case EQ:
		expand(p, 0, "	andb $64,%ah\n	jne LC\n");
		break;
	case NE:
		expand(p, 0, "	andb $64,%ah\n	je LC\n");
		break;
	case LE:
		expand(p, 0, "	andb $65,%ah\n	cmpb $1,%ah\n	jne LC\n");
		break;
	case LT:
		expand(p, 0, "	andb $65,%ah\n	je LC\n");
		break;
	case GT:
		expand(p, 0, "	andb $1,%ah\n	jne LC\n");
		break;
	case GE:
		expand(p, 0, "	andb $65,%ah\n	jne LC\n");
		break;
	default:
		comperr("fcomp op %d\n", p->n_op);
	}
}

/*
 * Convert an unsigned long long to floating point number.
 */
static void
ulltofp(NODE *p)
{
	static int loadlab;
	int jmplab;

	if (loadlab == 0) {
		loadlab = getlab2();
		expand(p, 0, "	.data\n");
		printf(LABFMT ":	.long 0,0x80000000,0x403f\n", loadlab);
		expand(p, 0, "	.text\n");
	}
	jmplab = getlab2();
	expand(p, 0, "	pushl UL\n	pushl AL\n");
	expand(p, 0, "	fildq (%esp)\n");
	expand(p, 0, "	addl $8,%esp\n");
	expand(p, 0, "	cmpl $0,UL\n");
	printf("	jge " LABFMT "\n", jmplab);
	printf("	fldt " LABFMT "\n", loadlab);
	printf("	faddp %%st,%%st(1)\n");
	printf(LABFMT ":\n", jmplab);
}

static int
argsiz(NODE *p)
{
	TWORD t = p->n_type;

	if (t < LONG || t == FLOAT || t > BTMASK)
		return 4;
	if (t == LONG || t == ULONG || t == LONGLONG || t == ULONGLONG ||
	    t == DOUBLE)
		return 8;
	if (t == LDOUBLE)
		return 16;
	if (t == STRTY || t == UNIONTY)
		return p->n_stsize;
	comperr("argsiz");
	return 0;
}

void
zzzcode(NODE *p, int c)
{
	NODE *r, *l;
	int pr, lr, s;
	char *ch;

	switch (c) {
	case 'A': /* swap st0 and st1 if right is evaluated second */
		if ((p->n_su & DORIGHT) == 0) {
			if (logop(p->n_op))
				printf("	fxch\n");
			else
				printf("r");
		}
		break;

	case 'C':  /* remove from stack after subroutine call */
		if (p->n_left->n_flags & FSTDCALL)
			break;
		pr = p->n_qual;
		if (p->n_op == STCALL || p->n_op == USTCALL)
			pr += 4;
		if (p->n_op == UCALL)
			return; /* XXX remove ZC from UCALL */
		if (pr)
			printf("	addl $%d, %s\n", pr, rnames[ESP]);
		break;

	case 'E': /* Perform bitfield sign-extension */
		bfext(p);
		break;

	case 'F': /* Structure argument */
		if (p->n_stalign != 0) /* already on stack */
			starg(p);
		break;

	case 'G': /* Floating point compare */
		fcomp(p);
		break;

	case 'J': /* convert unsigned long long to floating point */
		ulltofp(p);
		break;

	case 'M': /* Output sconv move, if needed */
		l = getlr(p, 'L');
		/* XXX fixneed: regnum */
		pr = DECRA(p->n_reg, 0);
		lr = DECRA(l->n_reg, 0);
		if ((pr == AL && lr == EAX) || (pr == BL && lr == EBX) ||
		    (pr == CL && lr == ECX) || (pr == DL && lr == EDX))
			;
		else
			printf("	movb %%%cl,%s\n",
			    rnames[lr][2], rnames[pr]);
		l->n_rval = l->n_reg = p->n_reg; /* XXX - not pretty */
		break;

	case 'N': /* output extended reg name */
		printf("%s", rnames[getlr(p, '1')->n_rval]);
		break;

	case 'S': /* emit eventual move after cast from longlong */
		pr = DECRA(p->n_reg, 0);
		lr = p->n_left->n_rval;
		switch (p->n_type) {
		case CHAR:
		case UCHAR:
			if (rnames[pr][2] == 'l' && rnames[lr][2] == 'x' &&
			    rnames[pr][1] == rnames[lr][1])
				break;
			if (rnames[lr][2] == 'x') {
				printf("\tmovb %%%cl,%s\n",
				    rnames[lr][1], rnames[pr]);
				break;
			}
			/* Must go via stack */
			s = BITOOR(freetemp(1));
			printf("\tmovl %%e%ci,%d(%%rbp)\n", rnames[lr][1], s);
			printf("\tmovb %d(%%rbp),%s\n", s, rnames[pr]);
			comperr("SCONV1 %s->%s", rnames[lr], rnames[pr]);
			break;

		case SHORT:
		case USHORT:
			if (rnames[lr][1] == rnames[pr][2] &&
			    rnames[lr][2] == rnames[pr][3])
				break;
			printf("\tmovw %%%c%c,%%%s\n",
			    rnames[lr][1], rnames[lr][2], rnames[pr]+2);
			comperr("SCONV2 %s->%s", rnames[lr], rnames[pr]);
			break;
		case INT:
		case UNSIGNED:
			if (rnames[lr][1] == rnames[pr][2] &&
			    rnames[lr][2] == rnames[pr][3])
				break;
			printf("\tmovl %%e%c%c,%s\n",
				    rnames[lr][1], rnames[lr][2], rnames[pr]);
			comperr("SCONV3 %s->%s", rnames[lr], rnames[pr]);
			break;

		default:
			if (rnames[lr][1] == rnames[pr][2] &&
			    rnames[lr][2] == rnames[pr][3])
				break;
			comperr("SCONV4 %s->%s", rnames[lr], rnames[pr]);
			break;
		}
		break;

	default:
		comperr("zzzcode %c", c);
	}
}

/*ARGSUSED*/
int
rewfld(NODE *p)
{
	return(1);
}

int canaddr(NODE *);
int
canaddr(NODE *p)
{
	int o = p->n_op;

	if (o==NAME || o==REG || o==ICON || o==OREG ||
	    (o==UMUL && shumul(p->n_left, SOREG)))
		return(1);
	return(0);
}

/*
 * Does the bitfield shape match?
 */
int
flshape(NODE *p)
{
	int o = p->n_op;

	if (o == OREG || o == REG || o == NAME)
		return SRDIR; /* Direct match */
	if (o == UMUL && shumul(p->n_left, SOREG))
		return SROREG; /* Convert into oreg */
	return SRREG; /* put it into a register */
}

/* INTEMP shapes must not contain any temporary registers */
/* XXX should this go away now? */
int
shtemp(NODE *p)
{
	return 0;
#if 0
	int r;

	if (p->n_op == STARG )
		p = p->n_left;

	switch (p->n_op) {
	case REG:
		return (!istreg(p->n_rval));

	case OREG:
		r = p->n_rval;
		if (R2TEST(r)) {
			if (istreg(R2UPK1(r)))
				return(0);
			r = R2UPK2(r);
		}
		return (!istreg(r));

	case UMUL:
		p = p->n_left;
		return (p->n_op != UMUL && shtemp(p));
	}

	if (optype(p->n_op) != LTYPE)
		return(0);
	return(1);
#endif
}

void
adrcon(CONSZ val)
{
	printf("$" CONFMT, val);
}

void
conput(FILE *fp, NODE *p)
{
	int val = p->n_lval;

	switch (p->n_op) {
	case ICON:
		if (p->n_name[0] != '\0') {
			fprintf(fp, "%s", p->n_name);
			if (val)
				fprintf(fp, "+%d", val);
		} else
			fprintf(fp, "%d", val);
		return;

	default:
		comperr("illegal conput, p %p", p);
	}
}

/*ARGSUSED*/
void
insput(NODE *p)
{
	comperr("insput");
}

/*
 * Write out the upper address, like the upper register of a 2-register
 * reference, or the next memory location.
 */
void
upput(NODE *p, int size)
{

	size /= SZCHAR;
	switch (p->n_op) {
	case REG:
		fprintf(stdout, "%%%s", &rnames[p->n_rval][3]);
		break;

	case NAME:
	case OREG:
		p->n_lval += size;
		adrput(stdout, p);
		p->n_lval -= size;
		break;
	case ICON:
		fprintf(stdout, "$" CONFMT, p->n_lval >> 32);
		break;
	default:
		comperr("upput bad op %d size %d", p->n_op, size);
	}
}

void
adrput(FILE *io, NODE *p)
{
	int r;
	/* output an address, with offsets, from p */

	if (p->n_op == FLD)
		p = p->n_left;

	switch (p->n_op) {

	case NAME:
		if (p->n_name[0] != '\0') {
			fputs(p->n_name, io);
			if (p->n_lval != 0)
				fprintf(io, "+" CONFMT, p->n_lval);
		} else
			fprintf(io, CONFMT, p->n_lval);
		return;

	case OREG:
		r = p->n_rval;
		if (p->n_name[0])
			printf("%s%s", p->n_name, p->n_lval ? "+" : "");
		if (p->n_lval)
			fprintf(io, "%d", (int)p->n_lval);
		if (R2TEST(r)) {
			fprintf(io, "(%s,%s,8)", rnames[R2UPK1(r)],
			    rnames[R2UPK2(r)]);
		} else
			fprintf(io, "(%s)", rnames[p->n_rval]);
		return;
	case ICON:
#ifdef PCC_DEBUG
		/* Sanitycheck for PIC, to catch adressable constants */
		if (kflag && p->n_name[0]) {
			static int foo;

			if (foo++ == 0) {
				printf("\nfailing...\n");
				fwalk(p, e2print, 0);
				comperr("pass2 conput");
			}
		}
#endif
		/* addressable value of the constant */
		fputc('$', io);
		conput(io, p);
		return;

	case REG:
		switch (p->n_type) {
		case LONGLONG:
		case ULONGLONG:
			fprintf(io, "%%%c%c%c", rnames[p->n_rval][0],
			    rnames[p->n_rval][1], rnames[p->n_rval][2]);
			break;
		case SHORT:
		case USHORT:
			fprintf(io, "%%%s", &rnames[p->n_rval][2]);
			break;
		default:
			fprintf(io, "%s", rnames[p->n_rval]);
		}
		return;

	default:
		comperr("illegal address, op %d, node %p", p->n_op, p);
		return;

	}
}

static char *
ccbranches[] = {
	"je",		/* jumpe */
	"jne",		/* jumpn */
	"jle",		/* jumple */
	"jl",		/* jumpl */
	"jge",		/* jumpge */
	"jg",		/* jumpg */
	"jbe",		/* jumple (jlequ) */
	"jb",		/* jumpl (jlssu) */
	"jae",		/* jumpge (jgequ) */
	"ja",		/* jumpg (jgtru) */
};


/*   printf conditional and unconditional branches */
void
cbgen(int o, int lab)
{
	if (o < EQ || o > UGT)
		comperr("bad conditional branch: %s", opst[o]);
	printf("	%s " LABFMT "\n", ccbranches[o-EQ], lab);
}

static void
fixcalls(NODE *p, void *arg)
{
	/* Prepare for struct return by allocating bounce space on stack */
	switch (p->n_op) {
	case STCALL:
	case USTCALL:
		if (p->n_stsize+p2autooff > stkpos)
			stkpos = p->n_stsize+p2autooff;
		break;
	}
}

/*
 * Must store floats in memory if there are two function calls involved.
 */
static int
storefloat(struct interpass *ip, NODE *p)
{
	int l, r;

	switch (optype(p->n_op)) {
	case BITYPE:
		l = storefloat(ip, p->n_left);
		r = storefloat(ip, p->n_right);
		if (p->n_op == CM)
			return 0; /* arguments, don't care */
		if (callop(p->n_op))
			return 1; /* found one */
#define ISF(p) ((p)->n_type == FLOAT || (p)->n_type == DOUBLE || \
	(p)->n_type == LDOUBLE)
		if (ISF(p->n_left) && ISF(p->n_right) && l && r) {
			/* must store one. store left */
			struct interpass *nip;
			TWORD t = p->n_left->n_type;
			NODE *ll;
			int off;

                	off = BITOOR(freetemp(szty(t)));
                	ll = mklnode(OREG, off, FPREG, t);
			nip = ipnode(mkbinode(ASSIGN, ll, p->n_left, t));
			p->n_left = mklnode(OREG, off, FPREG, t);
                	DLIST_INSERT_BEFORE(ip, nip, qelem);
		}
		return l|r;

	case UTYPE:
		l = storefloat(ip, p->n_left);
		if (callop(p->n_op))
			l = 1;
		return l;
	default:
		return 0;
	}
}

void
myreader(struct interpass *ipole)
{
	struct interpass *ip;

	stkpos = p2autooff;
	DLIST_FOREACH(ip, ipole, qelem) {
		if (ip->type != IP_NODE)
			continue;
		walkf(ip->ip_node, fixcalls, 0);
		storefloat(ip, ip->ip_node);
	}
	if (stkpos > p2autooff)
		p2autooff = stkpos;
	if (stkpos > p2maxautooff)
		p2maxautooff = stkpos;
	if (x2debug)
		printip(ipole);
}

/*
 * Remove some PCONVs after OREGs are created.
 */
static void
pconv2(NODE *p, void *arg)
{
	NODE *q;

	if (p->n_op == PLUS) {
		if (p->n_type == (PTR|SHORT) || p->n_type == (PTR|USHORT)) {
			if (p->n_right->n_op != ICON)
				return;
			if (p->n_left->n_op != PCONV)
				return;
			if (p->n_left->n_left->n_op != OREG)
				return;
			q = p->n_left->n_left;
			nfree(p->n_left);
			p->n_left = q;
			/*
			 * This will be converted to another OREG later.
			 */
		}
	}
}

void
mycanon(NODE *p)
{
	walkf(p, pconv2, 0);
}

void
myoptim(struct interpass *ip)
{
}

void
rmove(int s, int d, TWORD t)
{
	int sl, sh, dl, dh;

	switch (t) {
	case LONG:
	case ULONG:
	case LONGLONG:
	case ULONGLONG:
		printf("	movq %s,%s\n", rnames[s], rnames[d]);
		break;
	case CHAR:
	case UCHAR:
		printf("	movb %s,%s\n", rnames[s], rnames[d]);
		break;
	case FLOAT:
	case DOUBLE:
	case LDOUBLE:
#ifdef notdef
		/* a=b()*c(); will generate this */
		comperr("bad float rmove: %d %d", s, d);
#endif
		break;
	default:
		printf("	movl %s,%s\n", rnames[s], rnames[d]);
	}
}

/*
 * For class c, find worst-case displacement of the number of
 * registers in the array r[] indexed by class.
 */
int
COLORMAP(int c, int *r)
{
	int num;

	switch (c) {
	case CLASSA:
		num = r[CLASSB] > 4 ? 4 : r[CLASSB];
		num += 2*r[CLASSC];
		num += r[CLASSA];
		return num < 6;
	case CLASSB:
		num = r[CLASSA];
		num += 2*r[CLASSC];
		num += r[CLASSB];
		return num < 4;
	case CLASSC:
		num = r[CLASSA];
		num += r[CLASSB] > 4 ? 4 : r[CLASSB];
		num += 2*r[CLASSC];
		return num < 5;
	case CLASSD:
		return r[CLASSD] < DREGCNT;
	}
	return 0; /* XXX gcc */
}

char *rnames[] = {
	"%eax", "%edx", "%ecx", "%ebx", "%esi", "%edi", "%ebp", "%esp",
	"%al", "%ah", "%dl", "%dh", "%cl", "%ch", "%bl", "%bh",
	"%rax", "%rdx", "%rcx", "%rbx", "%rsi", "%rdi", "%rbp", "%rsp",
	"%r08", "%r09", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15",
	"%eaxedx",
	"%st0", "%st1", "%st2", "%st3", "%st4", "%st5", "%st6", "%st7",
};

/*
 * Return a class suitable for a specific type.
 */
int
gclass(TWORD t)
{
	if (t == CHAR || t == UCHAR)
		return CLASSB;
	if (t == LONG || t == ULONG || t == LONGLONG || t == ULONGLONG)
		return CLASSC;
	if (t == FLOAT || t == DOUBLE || t == LDOUBLE)
		return CLASSD;
	return CLASSA;
}

/*
 * Calculate argument sizes.
 */
void
lastcall(NODE *p)
{
	NODE *op = p;
	int size = 0;

	p->n_qual = 0;
	if (p->n_op != CALL && p->n_op != FORTCALL && p->n_op != STCALL)
		return;
	for (p = p->n_right; p->n_op == CM; p = p->n_left)
		size += argsiz(p->n_right);
	size += argsiz(p);
	if (kflag)
		size -= 4;
	op->n_qual = size; /* XXX */
}

/*
 * Special shapes.
 */
int
special(NODE *p, int shape)
{
	int o = p->n_op;

	switch (shape) {
	case SFUNCALL:
		if (o == STCALL || o == USTCALL)
			return SRREG;
		break;
	case SPCON:
		if (o != ICON || p->n_name[0] ||
		    p->n_lval < 0 || p->n_lval > 0x7fffffff)
			break;
		return SRDIR;
	case SMIXOR:
		return tshape(p, SZERO);
	case SMILWXOR:
		if (o != ICON || p->n_name[0] ||
		    p->n_lval == 0 || p->n_lval & 0xffffffff)
			break;
		return SRDIR;
	case SMIHWXOR:
		if (o != ICON || p->n_name[0] ||
		     p->n_lval == 0 || (p->n_lval >> 32) != 0)
			break;
		return SRDIR;
	}
	return SRNOPE;
}

/*
 * Target-dependent command-line options.
 */
void
mflags(char *str)
{
}

/*
 * Do something target-dependent for xasm arguments.
 */
int
myxasm(struct interpass *ip, NODE *p)
{
	struct interpass *ip2;
	NODE *in = 0, *ut = 0;
	TWORD t;
	char *w;
	int reg;
	int cw;

	cw = xasmcode(p->n_name);
	if (cw & (XASMASG|XASMINOUT))
		ut = p->n_left;
	if ((cw & XASMASG) == 0)
		in = p->n_left;

	switch (XASMVAL(cw)) {
	case 'D': reg = EDI; break;
	case 'S': reg = ESI; break;
	case 'a': reg = EAX; break;
	case 'b': reg = EBX; break;
	case 'c': reg = ECX; break;
	case 'd': reg = EDX; break;
	case 't': reg = 0; break;
	case 'u': reg = 1; break;
	case 'A': reg = EAXEDX; break;
	case 'q': /* XXX let it be CLASSA as for now */
		p->n_name = tmpstrdup(p->n_name);
		w = strchr(p->n_name, 'q');
		*w = 'r';
		return 0;
	default:
		return 0;
	}
	p->n_name = tmpstrdup(p->n_name);
	for (w = p->n_name; *w; w++)
		;
	w[-1] = 'r'; /* now reg */
	t = p->n_left->n_type;
	if (reg == EAXEDX) {
		p->n_label = CLASSC;
	} else {
		p->n_label = CLASSA;
		if (t == CHAR || t == UCHAR) {
			p->n_label = CLASSB;
			reg = reg * 2 + 8;
		}
	}
	if (t == FLOAT || t == DOUBLE || t == LDOUBLE) {
		p->n_label = CLASSD;
		reg += 037;
	}

	if (in && ut)
		in = tcopy(in);
	p->n_left = mklnode(REG, 0, reg, t);
	if (ut) {
		ip2 = ipnode(mkbinode(ASSIGN, ut, tcopy(p->n_left), t));
		DLIST_INSERT_AFTER(ip, ip2, qelem);
	}
	if (in) {
		ip2 = ipnode(mkbinode(ASSIGN, tcopy(p->n_left), in, t));
		DLIST_INSERT_BEFORE(ip, ip2, qelem);
	}
	return 1;
}

void
targarg(char *w, void *arg)
{
	NODE **ary = arg;
	NODE *p, *q;

	p = ary[(int)w[1]-'0']->n_left;
	if (optype(p->n_op) != LTYPE)
		comperr("bad xarg op %d", p->n_op);
	q = tcopy(p);
	if (q->n_op == REG) {
		if (*w == 'k') {
			q->n_type = INT;
		} else if (*w != 'w') {
			if (q->n_type > UCHAR) {
				regno(q) = regno(q)*2+8;
				if (*w == 'h')
					regno(q)++;
			}
			q->n_type = INT;
		} else
			q->n_type = SHORT;
	}
	adrput(stdout, q);
	tfree(q);
}

/*
 * target-specific conversion of numeric arguments.
 */
int
numconv(void *ip, void *p1, void *q1)
{
	NODE *p = p1, *q = q1;
	int cw = xasmcode(q->n_name);

	switch (XASMVAL(cw)) {
	case 'a':
	case 'b':
	case 'c':
	case 'd':
		p->n_name = tmpcalloc(2);
		p->n_name[0] = XASMVAL(cw);
		return 1;
	default:
		return 0;
	}
}
