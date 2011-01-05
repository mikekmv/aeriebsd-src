/*
 * Copyright (C) Caldera International Inc. 2001-2002.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code and documentation must retain the above
 * copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 * This product includes software developed or owned by Caldera
 * International, Inc.
 * 4. Neither the name of Caldera International, Inc. nor the names of other
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE FOR ANY DIRECT,
 * INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*-
 * Copyright (c) 1991
 * The Regents of the University of California. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/param.h>

#undef CMASK			/* XXX */
#undef BIG			/* XXX */
#define MAXPTR (-1)		/* max value of any pointer variable */
#ifdef NROFF	/*NROFF*/
#define EM t.Em
#define HOR t.Hor
#define VERT t.Vert
#define INCH 240	/*increments per inch*/
#define SPS INCH/10	/*space size*/
#define SS INCH/10	/* " */
#define TRAILER 0
#define UNPAD 0227
#define PO 0 /*page offset*/
#define ASCII 1
#define PTID 1
#define LG 0
#define DTAB 0	/*set at 8 Ems at init time*/
#define ICS 2*SPS
#define TEMP 1536	/* 384k */
#endif
#ifndef NROFF	/*TROFF*/
#define INCH 432	/*troff resolution*/
#define SPS 20	/*space size at 10pt; 1/3 Em*/
#define SS 12	/*space size in 36ths of an em*/
#define TRAILER 6048	/*144*14*3 = 14 inches*/
#define UNPAD 027
#define PO 416 /*page offset 26/27ths inch*/
#define HOR 1
#define VERT 3
#define EM (6*(pts&077))
#define ASCII 0
#define PTID 0
#define LG 1
#define DTAB (INCH/2)
#define ICS 3*SPS
#define TEMP 2048	/* 512K */
#endif

#define NARSP	0177	/*narrow space*/
#define HNSP	0226	/*half narrow space*/
#define PS	10	/*default point size*/
#define FT	0	/*default font position*/
#define LL	65*INCH/10	/*line length; 39picas=6.5in*/
#define VS	INCH/6	/*vert space; 12points*/
#define NN	2048	/*number registers*/
#define NNAMES	14 /*predefined reg names*/
#define NIF	63	/*if-else nesting*/
#define NS	256	/*name buffer*/
#define NTM	1024	/*tm buffer*/
#define NEV	4	/*environments*/
#define EVLSZ	32	/*size of ev stack*/
#define NM	3600
#define EVS	80*256	/*environment size in words*/
#define DELTA	2048	/*delta core bytes*/
#define NHYP	12	/*max hyphens per word*/
#define NHEX	256	/*byte size of exception word list*/
#define NTAB	45	/*tab stops*/
#define NSO	12	/*"so" depth*/
#define WDSIZE	2048	/*word buffer size*/
#define LNSIZE	5760	/*line buffer size*/
#define NDI	5	/*number of diversions*/

#define DBL	0100000	/*double size indicator*/
#define MOT	0100000	/*motion character indicator*/
#define MOTV	0160000	/*clear for motion part*/
#define VMOT	0040000	/*vert motion bit*/
#define NMOT	0020000	/* negative motion indicator*/
#define MMASK	0100000	/*macro mask indicator*/
#define CMASK	0100377
#define ZBIT	0400	/*zero width char*/
#define BMASK	0377
#define BYTE	8
#define IMP	004	/*impossible char*/
#define FILLER	037
#define PRESC	026
#define HX	0376	/*High-order part of xlss*/
#define LX	0375	/*low-order part of xlss*/
#define CONT	025
#define COLON	013
#define XPAR	030
#define ESC	033
#define FLSS	031
#define RPT	014
#define JREG	0374
#define NTRAP	20	/*number of traps*/
#define NPN	20	/*numbers in "-o"*/
#define T_PAD	0101	/*cat padding*/
#define T_INIT	0100
#define T_IESC	16 /*initial offset*/
#define T_STOP	0111
#define NPP	10	/*pads per field*/
#define FBUFSZ	2048
#define OBUFSZ	153600	/*bytes*/
#define IBUFSZ	16384	/*bytes*/
#define NC	256	/*cbuf size words*/
#define NOV	10	/*number of overstrike chars*/
#define TDELIM	032
#define LEFT	035
#define RIGHT	036
#define LEADER	001
#define TAB	011
#define TMASK	037777
#define RTAB	0100000
#define CTAB	0040000
#define OHC	024

#define PAIR(A,B) (A|(B<<BYTE))

#define BLK  256	/*alloc block words*/

#define	BIG 4096

#ifdef BIG
typedef long filep;
#define NBLIST	BIG	/*allocation , BIG = 256 per 65k*/
#define BLKBITS	8	/*for BLK=128*/
#else
typedef unsigned filep;
#define NBLIST	TEMP	/*allocation list, TEMP<=512*/
/* BLK*NBLIST<=65536 words, if filep=unsigned */
#define BLKBITS	0
#endif

struct s {
	int nargs;
	struct s *pframe;
	filep pip;
	int pnchar;
	int prchar;
	int ppendt;
	int *pap;
	int *pcp;
	int pch0;
	int pch;
};

/* typewriter driving table structure*/
struct tw {
	int bset;
	int breset;
	int Hor;
	int Vert;
	int Newline;
	int Char;
	int Em;
	int Halfline;
	int Adj;
	char *twinit;
	char *twrest;
	char *twnl;
	char *hlr;
	char *hlf;
	char *flr;
	char *bdon;
	char *bdoff;
	char *ploton;
	char *plotoff;
	char *up;
	char *down;
	char *right;
	char *left;
	char *codetab[256-32];
	char *zzz;
};

struct contab {
	int rq;
	union {
		int (*f)();
		unsigned mx;
	} x;
};

struct d {
	filep op; int dnl,dimac,ditrap,ditf,alss,blss,nls,mkline,
	maxl,hnl,curd;
};

struct v {
	int pn,nl,yr,hp,ct,dn,mo,dy,dw,ln,dl,st,sb,cd;
	int vxx[NN-NNAMES];
};

extern int inchar[], *pinchar;
extern char obuf[], *obufp;
extern int r[];
extern int pto;
extern int pfrom;
extern int print;
extern char nextf[];
extern int nfi;
extern int stopmesg;
#ifdef NROFF
extern char termtab[];
extern int tti;
#endif
#ifndef NROFF
extern int oldbits;
#endif
extern int init;
extern int fc;
extern int eschar;
extern int pl;
extern int po;
extern int dfact;
extern int dfactd;
extern int res;
extern int smnt;
extern int ascii;
extern int ptid;
extern char ptname[];
extern int lg;
extern int pnlist[];
extern int *pnp;
extern int npn;
extern int npnflg;
extern int xflg;
extern int trflg;
extern int dpn;
extern int totout;
extern int ulfont;
extern int ulbit;
extern int tabch;
extern int ldrch;
extern int acctf;
extern int xxx;
extern struct contab contab[];
extern struct v v;
extern struct d d[], *dip;
extern char *fontfile;
extern char codetab[];
extern int bdtab[];
extern char pstab[];
extern char psctab[];

/*
troff environment block
*/

extern int block;
extern int ics;
extern int ic;
extern int icf;
extern int chbits;
extern int spbits;
extern int nmbits;
extern int apts;
extern int apts1;
extern int pts;
extern int pts1;
extern int font;
extern int font1;
extern int sps;
extern int spacesz;
extern int lss;
extern int lss1;
extern int ls;
extern int ls1;
extern int ll;
extern int ll1;
extern int lt;
extern int lt1;
extern int ad;
extern int nms;
extern int ndf;
extern int fi;
extern int cc;
extern int c2;
extern int ohc;
extern int tdelim;
extern int hyf;
extern int hyoff;
extern int un1;
extern int tabc;
extern int dotc;
extern int adsp;
extern int adrem;
extern int lastl;
extern int nel;
extern int admod;
extern int *wordp;
extern int spflg;
extern int *linep;
extern int *wdend;
extern int *wdstart;
extern int wne;
extern int ne;
extern int nc;
extern int nb;
extern int lnmod;
extern int nwd;
extern int nn;
extern int ni;
extern int ul;
extern int cu;
extern int ce;
extern int in;
extern int in1;
extern int un;
extern int wch;
extern int pendt;
extern int *pendw;
extern int pendnf;
extern int error;
extern int spread;
extern int it;
extern int itmac;
extern int lnsize;
extern int *hyptr[];
extern int tabtab[];
extern int line[];
extern int word[];
extern int blockxxx[];
extern int oline[];


extern int *vlist;
extern struct s *frame, *stk, *ejl;
extern struct s *nxf, *litlev;

#ifdef NROFF
extern struct tw t;
extern int pipeflg;
extern int hflg;
extern int eqflg;
#endif

#ifndef NROFF
extern int xpts;
extern int verm;
extern int *pslp;
extern int psflg;
extern int ppts;
extern int pfont;
extern int paper;
extern int mpts;
extern int mfont;
extern int mcase;
extern int escm;
extern int cs;
extern int code;
extern int ccs;
extern int bd;
extern int back;
#endif

extern int level;
extern int stdi;
extern int waitf;
extern int nofeed;
extern int quiet;
extern int stop;
extern char ibuf[];
extern char xbuf[];
extern char *ibufp;
extern char *xbufp;
extern char *eibuf;
extern char *xeibuf;
extern int cbuf[];
extern int *cp;
extern int nx;
extern int mflg;
extern int ch;
extern int cps;
extern int ibf;
extern int ttyod;
extern struct sgttyb ttys;
extern int iflg;
extern char *enda;
extern int rargc;
extern char **argp;
extern char trtab[];
extern int lgf;
extern int copyf;
extern int ch0;
extern int cwidth;
extern filep ip;
extern int nlflg;
extern int *ap;
extern int donef;
extern int nflush;
extern int nchar;
extern int rchar;
extern int nfo;
extern int ifile;
extern int padc;
extern int raw, raw2;
extern int fontlab[];
extern int ifl[];
extern int ifi;
extern int flss;
extern int nonumb;
extern int trap;
extern int tflg;
extern int ejf;
extern int lit;
extern int gflag;
extern int dilev;
extern int tlss;
extern filep offset;
extern int em;
extern int ds;
extern filep woff;
extern int app;
extern int ndone;
extern int lead;
extern int ralss;
extern filep nextb;
extern int *argtop;
extern int nrbits;
extern int nform;
extern int oldmn;
extern int newmn;
extern int macerr;
extern filep apptr;
extern int diflg;
extern filep roff;
extern int wbfi;
extern int inc[];
extern int fmt[];
extern int evi;
extern int vflag;
extern int noscale;
extern int po1;
extern int nlistx[];
extern int mlist[];
extern int evlist[];
extern int ev;
extern int tty;
extern int sfont;
extern int sv;
extern int esc;
extern int widthp;
extern int xfont;
extern int setwdf;
extern int xbitf;
extern int over;
extern int nhyp;
extern int **hyp;
extern int *olinep;
extern int esct;
extern int ttysave;
extern int dotT;
extern char *unlkp;
extern int no_out;

char *setbrk(int);
int max(int, int);
long atoi0(void);
long atoi1(void);
int control(int, int);
int pchar(int);
int width(int);
void move(void);
int find(int, int []);
int findr(int);
filep finds(int);
int findt1(void);
int findn(int);
int findmn(int);
void clrmn(int);
int nextfile(void);
void flushi(void);
int popf(void);
int popi(void);
int pushi(filep);
void flushi(void);
filep boff(int);
filep alloc(void);
filep incoff(filep);
void ffree(filep);
void setrpt(void);
void seta(void);
void setn1(int);
int sethl(int);
void setwd(void);
void setn(void);
int setstr(void);
void getpn(char *);
int getach(void);
int getname(void);
int getlg(int);
int getch(void);
int getch0(void);
int getsn(void);
int getrq(void);
void prstrfl(char *);
void prstr(char *);
void cvtime(void);
void ptinit(void);
void ptout(int);
void ptout1(void);
void ptlead(void);
void dostop(void);
void twdone(void);
char *plot(char *);
void edone(int);
void oput(int);
void oputs(char *);
void flusho(void);
void pchar1(int);
void wbt(int);
void wbf(int);
void wbfl(void);
void copys(void);
long ckph(void);
int xlss(void);
int makem(int);
int vmot(void);
int hmot(void);
int mot(void);
void setfont(int);
void setps(void);
int setch(void);
void setnel(void);
void mchbits(void);
int skip(void);
int quant(int, int);
int inumb(int *);
int fnumb(int,int (*)(int));
int vnumb(int *);
int hnumb(int *);
void text(void);
void newline(int);
void chkpn(void);
void eject(struct s *);
void horiz(int);
int getword(int);
void hyphen(int *);
int eat(int);
int rbf(void);
int rbf0(filep);
int sumhp(void);
void collect(void);
int rdtty(void);
int chget(int);
int alph(int);


void
caseds(void), caseas(void), casesp(int), caseft(void), caseps(void),
casevs(void), casenr(void), caseif(int), casepo(void), casetl(void),
casetm(int), casebp(void), casech(void), casepn(void), tbreak(void),
caseti(void), casene(void), casenf(void), casece(void), casefi(void),
casein(void), caseli(void), casell(void), casens(void), casemk(void),
casert(void), caseam(void), casede(void), casedi(void), caseda(void),
casewh(void), casedt(void), caseit(void), caserm(void), casern(void),
casead(void), casers(void), casena(void), casepl(void), caseta(void),
casetr(void), caseul(void), caselt(void), casenx(void), caseso(void),
caseig(void), casetc(void), casefc(void), caseec(void), caseeo(void),
caselc(void), caseev(void), caserd(void), caseab(void), casefl(void),
casess(void), casefp(void), casecs(void), casebd(void), caselg(void),
casehc(void), casehy(void), casenh(void), casenm(void), casenn(void),
casesv(void), caseos(void), casels(void), casecc(void), casec2(void),
caseem(void), caseaf(void), casehw(void), casemc(void), casepm(void),
casecu(void), casepi(void), caserr(void), caseuf(void), caseie(void),
caseel(void), casepc(void), caseht(void), casefz(void), casecf(void);
void done3(int);
void done2(int);
void done1(int);
void done(int);

