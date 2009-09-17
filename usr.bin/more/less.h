/*
 * Copyright (c) 1988 Mark Nudleman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)less.h	8.1 (Berkeley) 6/6/93
 *	@(#)pathnames.h	8.1 (Berkeley) 6/6/93
 */

#define	_PATH_HELPFILE	"/usr/share/misc/more.help"

#define	RECOMP

#define	NULL_POSITION	((off_t)(-1))

#define	EOI		(0)
#define	READ_INTR	(-2)

/* Special chars used to tell put_line() to do something special */
#define	UL_CHAR		'\201'		/* Enter underline mode */
#define	UE_CHAR		'\202'		/* Exit underline mode */
#define	BO_CHAR		'\203'		/* Enter boldface mode */
#define	BE_CHAR		'\204'		/* Exit boldface mode */

#define	CONTROL_CHAR(c)		(iscntrl(c))
#define	CARAT_CHAR(c)		((c == '\177') ? '?' : (c | 0100))

#define	TOP		(0)
#define	TOP_PLUS_ONE	(1)
#define	BOTTOM		(-1)
#define	BOTTOM_PLUS_ONE	(-2)
#define	MIDDLE		(-3)

/* Special key identifier */
#define	SP_ARR_UP	0	
#define	SP_ARR_DOWN	1
#define	SP_PAGE_UP	2
#define	SP_PAGE_DOWN	3
#define	SP_END_KEYS	4

#define	A_INVALID		-1
#define	A_TABLE_END		-2

#define	A_AGAIN_SEARCH		1
#define	A_B_LINE		2
#define	A_B_SCREEN		3
#define	A_B_SCROLL		4
#define	A_B_SEARCH		5
#define	A_DIGIT			6
#define	A_EXAMINE		7
#define	A_FREPAINT		8
#define	A_F_LINE		9
#define	A_F_SCREEN		10
#define	A_F_SCROLL		11
#define	A_F_SEARCH		12
#define	A_GOEND			13
#define	A_GOLINE		14
#define	A_GOMARK		15
#define	A_HELP			16
#define	A_NEXT_FILE		17
#define	A_PERCENT		18
#define	A_PREFIX		19
#define	A_PREV_FILE		20
#define	A_QUIT			21
#define	A_REPAINT		22
#define	A_SETMARK		23
#define	A_STAT			24
#define	A_VISUAL		25
#define	A_TAGFILE		26
#define	A_FILE_LIST		27

extern int sigs, linenums, auto_wrap, ignaw, retain_below, tabstop, bs_mode;
extern int top_scroll, caseless, tagoption, screen_trashed, hit_eof, nscroll;
extern int quit_at_eof, squeeze, ac, curr_ac, quitting, short_file;
extern int back_scroll, any_display, errmsgs, file, cbufs, ispipe, reading;
extern int erase_char, kill_char, werase_char, cmdstack;
extern int sc_width, sc_height, sc_window;
extern int bo_width, be_width;
extern int ul_width, ue_width;
extern int so_width, se_width;
extern char *current_file, *line, **av, *tags;
extern char *current_name, *firstsearch, *next_name;

/* Output a plain backspace, without erasing the previous char */
extern char *sc_backspace;
#define	putbs()		putp(sc_backspace)
extern char *sc_b_in;
#define	bo_enter()	putp(sc_b_in)
extern char *sc_b_out;
#define	bo_exit()	putp(sc_b_out)
extern char *sc_u_in;
#define	ul_enter()	putp(sc_u_in)
extern char *sc_u_out;
#define	ul_exit()	putp(sc_u_out)
extern char *sc_s_in;
#define	so_enter()	putp(sc_s_in)
extern char *sc_s_out;
#define	so_exit()	putp(sc_s_out)
extern char *sc_eol_clear;
#define	clear_eol()	putp(sc_eol_clear)
extern char *sc_addline;
#define	add_line()	tputs(sc_addline, sc_height, putchar)
extern char *sc_home;
#define	home()		putp(sc_home)

void init(void);
void deinit(void);
void commands(void);
void intread(void);
void init_signals(int);
void psignals(void);
int iread(int, char *, int);
void quit(void);
void next_file(int);
char *bad_file(const char *, char *, size_t len);
void clr_linenum(void);
void open_getchr(void);
off_t position(int);
void ch_init(int, int);
off_t ch_length(void);
int getchr(void);
int onscreen(off_t);
int ch_seek(off_t);
int ch_end_seek(void);
int ch_beg_seek(void);
off_t ch_length(void);
off_t ch_tell(void);
int ch_forw_get(void);
int ch_back_get(void);
int ch_addbuf(int);
off_t back_raw_line(off_t);
off_t forw_raw_line(off_t);
void add_lnum(int, off_t);
int find_linenum(off_t);
void pos_clear(void);
void copytable(void);
off_t forw_line(off_t);
void add_forw_pos(off_t);
void start_mca(int, const char *);
void error(const char *);
void init_mark(void);
void setmark(int);
void lastmark(void);
void gomark(int);
int search(int, char *, int, int);
int get_back_scroll(void);
int currline(int);
void add_back_pos(off_t);
off_t back_line(off_t);
void put_line(void);
void prepaint(off_t);
#define	repaint()	prepaint(position(TOP));
void eof_check(void);
void squish_check(void);
void forw(int, off_t, int);
void back(int, off_t, int);
void forward(int, int);
void backward(int, int);
void jump_forw(void);
void jump_back(int);
void jump_percent(int);
void jump_loc(off_t);
void raw_mode(int);
void get_term(void);
void lower_left(void);
void findtag(char *, char **, char **);
int tagsearch(char *);
void ierror(char *);
void prewind(void);
int pappend(int);
int cmd_decode(int);
void noprefix(void);
void windoch(int);
void prev_file(int);
int edit(char *);
void backspace(void);
void special_key_init(void);
void add_cmdtable(char *, int);
