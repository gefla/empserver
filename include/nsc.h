/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  ---
 *
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  nsc.h: Definitions for Empire conditionals
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#ifndef _NSC_H_
#define _NSC_H_

#include "xy.h"

#define NS_LSIZE	128
#define NS_NCOND	16

enum {				/* masks for struct nscstr member oper */
    NSC_OPMASK = 0x0ff,		/* operator */
    NSC_ISNUM1 = 0x100,		/* is left operand a number? */
    NSC_ISNUM2 = 0x200		/* is right operand a number? */
};

struct nscstr {
    long fld1;			/* first commodity or number */
    long fld2;			/* second commodity or number */
    int oper;			/* required relationship operator */
};

struct nstr_sect {
    coord x, y;			/* current x-y */
    coord dx, dy;		/* accumlated x,y travel */
    int id;			/* return value of sctoff */
    int type;			/* type of query */
    int curdist;		/* dist query: current range */
    struct range range;		/* area of coverage */
    int dist;			/* dist query: range */
    coord cx, cy;		/* dist query: center x-y */
    int (*read)(int type, int id, caddr_t ptr);	/* read function */
    int ncond;			/* # of selection conditions */
    struct nscstr cond[NS_NCOND];	/* selection conditions */
};

struct nstr_item {
    int cur;			/* current item */
    int sel;			/* selection type */
    int type;			/* item type being selected */
    int curdist;		/* if NS_DIST, current item's dist */
    struct range range;		/* NS_AREA/NS_DIST: range selector */
    int dist;			/* NS_DIST: distance selector */
    coord cx, cy;		/* NS_DIST: center x-y, NS_XY: xy */
    int group;			/* NS_GROUP: fleet/wing match */
    int size;			/* NS_LIST: size of list */
    int index;			/* NS_LIST: index */
    int list[NS_LSIZE];		/* NS_LIST: item list */
    int (*read)(int type, int id, caddr_t ptr);	/* read function */
    int flags;			/* EFF_ flags */
    int ncond;			/* # of selection conditions */
    struct nscstr cond[NS_NCOND];	/* selection conditions */
};

#define NS_UNDEF	0
#define NS_LIST		1
#define NS_DIST		2
#define NS_AREA		3
#define NS_ALL		4
#define NS_XY		5
#define NS_GROUP	6

/*
 * codes looks like this:
 * D: is access restricted to deity?
 * T: type of member addressed by offset, if category NSC_OFF
 * C: category of value
 * V: value
 *
 * 2 2  1 1    1
 * 2 0  8 6    2    8    4    0
 * xxx xxxx xxxx xxxx xxxx xxxx
 * DTT TTCC VVVV VVVV VVVV VVVV
 */

/*
 * categories
 */
#define NSC_VAL		(0)	/* value is plain number */
#define NSC_VAR		(1<<16)	/* value is a vtype */
#define NSC_OFF		(2<<16)	/* value is an offset */
#define NSC_CMASK	(3<<16)

/*
 * how to interpret "offset" fields
 */
#define NSC_CHAR	(1<<18)	/* offset of s_char member */
#define NSC_UCHAR	(2<<18)	/* offset of unsigned char member */
#define NSC_SHORT	(3<<18)	/* offset of short */
#define NSC_USHORT	(4<<18)	/* offset of unsigned short member */
#define NSC_INT		(5<<18)	/* offset of int member */
#define NSC_LONG	(6<<18)	/* offset of long member */
#define NSC_XCOORD	(7<<18)	/* offset of coord that needs x conversion */
#define NSC_YCOORD	(8<<18)	/* offset of coord that needs y conversion */
#define NSC_FLOAT	(9<<18)	/* offset of float member */
#define NSC_CHARP	(10<<18) /* offset of char *member */
#define NSC_TIME	(11<<18) /* offset of time_t member */
#define NSC_TMASK	(15<<18)

#define NSC_NATID	NSC_UCHAR /* must match natid */

#define NSC_MASK	(0xffff0000)

#define NSC_DEITY	(1<<22)

struct castr {
    long ca_code;		/* encoded form */
    s_char *ca_name;		/* name used for matches */
    u_short ca_len;		/* Used for arrays */
};

/* variables using the above */

extern struct castr var_ca[];
extern struct castr sect_ca[];
extern struct castr ship_ca[];
extern struct castr plane_ca[];
extern struct castr land_ca[];
extern struct castr nuke_ca[];
extern struct castr news_ca[];
extern struct castr nat_ca[];
extern struct castr treaty_ca[];
extern struct castr trade_ca[];
extern struct castr loan_ca[];
extern struct castr genitem_ca[];
extern struct castr lost_ca[];
extern struct castr commodity_ca[];

/* src/lib/subs/nstr.c */
extern s_char *nstr_comp(struct nscstr *, int *, int, s_char *);
extern int encode(register s_char *, long *, int);


extern s_char *decodep(long, void *);
extern int decode(natid, long, void *, int);
extern int nstr_exec(struct nscstr *, register int, void *, int);

#endif /* _NSC_H_ */
