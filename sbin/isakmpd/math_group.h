/* $EOM: math_group.h,v 1.7 1999/04/17 23:20:40 niklas Exp $	 */

/*
 * Copyright (c) 1998 Niels Provos.  All rights reserved.
 * Copyright (c) 1999 Niklas Hallqvist.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

/*
 * This code was written under funding by Ericsson Radio Systems.
 */

#ifndef _MATH_GROUP_H_
#define _MATH_GROUP_H_

enum groups {
	MODP,			/* F_p, Z modulo a prime */
	EC2N,			/* Elliptic Curve over the Field GF(2**N) */
	ECP,			/* Elliptic Curve over the Field Z_p */
	NOTYET			/* Not yet assigned */
};

/*
 * The group on which diffie hellmann calculations are done.
 */

struct group {
	enum groups     type;
	int             id;	/* Group ID */
	int             bits;	/* Number of key bits provided by this group */
	void           *group;
	void           *a, *b, *c, *d;
	void           *gen;	/* Group Generator */
	int             (*getlen) (struct group *);
	void            (*getraw) (struct group *, void *, u_int8_t *);
	int             (*setraw) (struct group *, void *, u_int8_t *, int);
	int             (*setrandom) (struct group *, void *);
	int             (*operation) (struct group *, void *, void *, void *);
	int             (*validate_public) (struct group *, void *);
};

/* Description of an Elliptic Group over GF(2**n) for Boot-Strapping */

struct ec2n_dscr {
	int             id;
	int             bits;	/* Key Bits provided by this group */
	char           *polynomial;	/* Irreducible polynomial */
	char           *gen_x;	/* X - Coord. of Generator */
	char           *a, *b;	/* Curve Parameters */
};

/* Description of F_p for Boot-Strapping */

struct modp_dscr {
	int             id;
	int             bits;	/* Key Bits provided by this group */
	char           *prime;	/* Prime */
	char           *gen;	/* Generator */
};

/* Prototypes */

void            group_init(void);
void            group_free(struct group *);
struct group   *group_get(u_int32_t);

void            ec2n_free(struct group *);
struct group   *ec2n_clone(struct group *, struct group *);
void            ec2n_init(struct group *);

void            modp_free(struct group *);
struct group   *modp_clone(struct group *, struct group *);
void            modp_init(struct group *);

#endif				/* _MATH_GROUP_H_ */
