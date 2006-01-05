/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2004
 */

#ifndef NSC_H
#define NSC_H

#include <stddef.h>
#include "xy.h"

#define NS_LSIZE	128
#define NS_NCOND	16

/* Value type */
typedef enum {
    NSC_NOTYPE,
    /* promoted types */
    NSC_LONG,			/* long */
    NSC_DOUBLE,			/* double */
    NSC_STRING,			/* char *, zero-terminated string */
    NSC_TYPEID,			/* signed char, index into chr table */
    /* unpromoted types */
    NSC_CHAR,			/* signed char */
    NSC_UCHAR,			/* unsigned char */
    NSC_SHORT,			/* short */
    NSC_USHORT,			/* unsigned short */
    NSC_INT,			/* int */
    NSC_XCOORD,			/* coord that needs x conversion */
    NSC_YCOORD,			/* coord that needs y conversion */
    NSC_TIME,			/* time_t */
    NSC_FLOAT,			/* float */
    NSC_STRINGY,		/* char[], zero-terminated string */
				/* FIXME zero may be missing */
    /* aliases, must match typedefs */
    NSC_NATID = NSC_UCHAR	/* nation id */
} nsc_type;
typedef char packed_nsc_type;

/* Return nsc_type for a signed integer with the same size as TYPE.  */
#define NSC_SITYPE(type)				\
    (sizeof(type) == 1 ? NSC_CHAR			\
     : sizeof(type) == sizeof(short) ? NSC_SHORT	\
     : sizeof(type) == sizeof(int) ? NSC_INT		\
     : sizeof(type) == sizeof(long) ? NSC_LONG		\
     : 1/0)

/* Value category */
typedef enum {
    NSC_NOCAT,
    NSC_VAL,			/* evaluated value */
    NSC_OFF,			/* symbolic value: at offset in object */
    NSC_ID			/* unresolved identifier (internal use) */
} nsc_cat;
typedef char packed_nsc_cat;

enum {
    NSC_DEITY = bit(0),		/* access restricted to deity */
    NSC_EXTRA = bit(1),		/* computable from other selectors */
    NSC_CONST = bit(2),		/* field cannot be changed */
    NSC_BITS = bit(3)		/* value consists of flag bits */
};
typedef unsigned char nsc_flags;

/*
 * Value, possibly symbolic.
 * If type is NSC_NOTYPE, it's an error value.
 * If category is NSC_OFF, the value is in a context object at offset
 * val_as.sym.off + val_as.sym.idx * S, where S is the size of the
 * value.
 * If category is NSC_VAL, the value is in val_as, and the type is a
 * promoted type.
 */
struct valstr {
    packed_nsc_type val_type;	/* type of value */
    packed_nsc_cat val_cat;	/* category of value */
    union {
	struct {		/* cat NSC_OFF */
	    ptrdiff_t off;
	    int len;
	    int idx;
	} sym;
	double dbl;		/* cat NSC_VAL, type NSC_DOUBLE */
	struct {		/* cat NSC_VAL, type NSC_STRING, cat NSC_ID */
	    char *base;
	    size_t maxsz;
	} str;
	long lng;		/* cat NSC_VAL, type NSC_LONG, NSC_TYPEID */
    } val_as;
};

/* Compiled condition */
struct nscstr {
    char operator;		/* '<', '=', '>', '#' */
    packed_nsc_type optype;	/* operator type */
    struct valstr lft;		/* left operand */
    struct valstr rgt;		/* right operand */
};

/* Selection type */
typedef enum {
    NS_UNDEF,			/* error value */
    NS_LIST,			/* list of IDs */
    NS_DIST,			/* circular area */
    NS_AREA,			/* rectangular area */
    NS_ALL,			/* everything */
    NS_XY,			/* one sector area */
    NS_GROUP			/* group, i.e. fleet, wing, army */
} ns_seltype;

/* Sector iterator */
struct nstr_sect {
    coord x, y;			/* current x-y */
    coord dx, dy;		/* accumlated x,y travel */
    int id;			/* return value of sctoff */
    ns_seltype type;		/* type of query */
    int curdist;		/* dist query: current range */
    struct range range;		/* area of coverage */
    int dist;			/* dist query: range */
    coord cx, cy;		/* dist query: center x-y */
    int (*read)(int type, int id, void *ptr);	/* read function */
    int ncond;			/* # of selection conditions */
    struct nscstr cond[NS_NCOND];	/* selection conditions */
};

/* Item iterator */
struct nstr_item {
    int cur;			/* current item */
    ns_seltype sel;		/* selection type */
    int type;			/* item type being selected */
    int curdist;		/* if NS_DIST, current item's dist */
    struct range range;		/* NS_AREA/NS_DIST: range selector */
    int dist;			/* NS_DIST: distance selector */
    coord cx, cy;		/* NS_DIST: center x-y, NS_XY: xy */
    int group;			/* NS_GROUP: fleet/wing match */
    int size;			/* NS_LIST: size of list */
    int index;			/* NS_LIST: index */
    int list[NS_LSIZE];		/* NS_LIST: item list */
    int (*read)(int type, int id, void *ptr);	/* read function */
    int flags;			/* ef_flags(TYPE) */
    int ncond;			/* # of selection conditions */
    struct nscstr cond[NS_NCOND]; /* selection conditions */
};

/*
 * Symbol binding: associate NAME with VALUE.
 */
struct symbol {
    int value;
    char *name;
};

/*
 * Selector descriptor.
 */
struct castr {
    packed_nsc_type ca_type;	/* type of value */
    nsc_flags ca_flags;
    unsigned short ca_len;	/* non-zero: is an array; #array elements */
    ptrdiff_t ca_off;		/* offset of value in the context object */
    char *ca_name;
    int ca_table;		/* referred table ID, or EF_BAD */
};

/* variables using the above */

extern struct castr ichr_ca[];
extern struct castr pchr_ca[];
extern struct castr sect_ca[];
extern struct castr dchr_ca[];
extern struct castr ship_ca[];
extern struct castr mchr_ca[];
extern struct castr plane_ca[];
extern struct castr plchr_ca[];
extern struct castr land_ca[];
extern struct castr lchr_ca[];
extern struct castr nuke_ca[];
extern struct castr nchr_ca[];
extern struct castr treaty_ca[];
extern struct castr loan_ca[];
extern struct castr news_ca[];
extern struct castr lost_ca[];
extern struct castr commodity_ca[];
extern struct castr trade_ca[];
extern struct castr nat_ca[];
extern struct castr intrchr_ca[];
extern struct castr rpt_ca[];
extern struct castr empfile_ca[];
extern struct castr symbol_ca[];
extern struct symbol ship_chr_flags[];
extern struct symbol plane_chr_flags[];
extern struct symbol land_chr_flags[];
extern struct symbol nuke_chr_flags[];
extern struct symbol treaty_flags[];
extern struct castr mdchr_ca[];
extern struct symbol meta_type[];
extern struct symbol meta_flags[];
extern struct symbol missions[];
extern struct symbol plane_flags[];
extern struct symbol retreat_flags[];
extern struct symbol nation_status[];
extern struct symbol nation_flags[];
extern struct symbol nation_relations[];
extern struct symbol level[];
extern struct symbol agreement_statuses[];
extern struct symbol plague_stages[];
extern struct symbol packing[];
extern struct symbol resources[];

/* src/lib/subs/nstr.c */
extern int nstr_comp(struct nscstr *np, int len, int type, char *str);
extern char *nstr_comp_val(char *, struct valstr*, int);
extern int nstr_coerce_val(struct valstr *, nsc_type, char *);
extern int nstr_exec(struct nscstr *, int, void *);
extern void nstr_exec_val(struct valstr *, natid, void *, nsc_type);
extern char *symbol_by_value(int, struct symbol *);

#endif
