/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  nsc.h: Definitions for Empire conditionals
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Markus Armbruster, 2004-2016
 */

#ifndef NSC_H
#define NSC_H

#include <stddef.h>
#include "misc.h"
#include "xy.h"

#define NS_LSIZE	128
#define NS_NCOND	16

/* Value type */
enum nsc_type {
    NSC_NOTYPE,
    /* promoted types */
    NSC_LONG,			/* long */
    NSC_DOUBLE,			/* double */
    NSC_STRING,			/* character string */
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
    NSC_STRINGY,		/* char[] */
    /* aliases, must match typedefs */
    NSC_NATID = NSC_UCHAR	/* nation ID */
};

/* Is TYPE a promoted value type? */
#define NSC_IS_PROMOTED(type) (NSC_LONG <= (type) && (type) <= NSC_STRING)

/* Return nsc_type for a signed integer with the same size as TYPE. */
#define NSC_SITYPE(type) \
    (BUILD_ASSERT_ONE(NSC_SITYPE_(type) != NSC_NOTYPE) * NSC_SITYPE_(type))
#define NSC_SITYPE_(type)				\
    (sizeof(type) == 1 ? NSC_CHAR			\
     : sizeof(type) == sizeof(short) ? NSC_SHORT	\
     : sizeof(type) == sizeof(int) ? NSC_INT		\
     : sizeof(type) == sizeof(long) ? NSC_LONG		\
     : NSC_NOTYPE)

/* Value category */
enum nsc_cat {
    NSC_NOCAT,
    NSC_VAL,			/* evaluated value */
    NSC_OFF,			/* symbolic value: at offset in object */
    NSC_ID			/* unresolved identifier (internal use) */
};

/*
 * Value, possibly symbolic
 *
 * If type is NSC_NOTYPE, it's an error value.
 * If category is NSC_VAL, the value is in @val_as, and the type is a
 * promoted type.
 * If category is NSC_OFF, the value is in a context object, and
 * @val_as.sym specifies how to get it, as follows.
 * If sym.get is null, and type is NSC_STRINGY, the value is a string
 * stored in sym.len characters starting at sym.offs in the context
 * object.  sym.idx must be zero.  Ugly wart: if sym.len is one, the
 * terminating null character may be omitted.
 * Else if sym.get is null, and sym.len is zero, the value is in the
 * context object at offset sym.off.  sym.idx must be zero.
 * Else if sym.get is null, sym.len is non-zero, and the value has
 * index sym.idx in an array of sym.len elements at offset sym.off in
 * in the context object.  I.e. the value is at sym.off + sym.idx *
 * SZ, where SZ is the size of the value.
 * If sym.get is not null, you obtain the value by calling get() like
 * VAL->get(VAL, NP, CTXO), where NP points to the country to use for
 * coordinate translation and access control (null for none), and CTXO
 * is the context object.  get() either returns a null pointer and
 * sets VAL->val_as to the value, as appropriate for the type.  Or it
 * returns another context object and sets VAL->val_as.sym for it.
 */
struct valstr {
    enum nsc_type val_type;	/* type of value */
    enum nsc_cat val_cat;	/* category of value */
    union {
	struct {		/* cat NSC_OFF */
	    ptrdiff_t off;
	    int len;
	    int idx;
	    void *(*get)(struct valstr *, struct natstr *, void *);
	    int hidden;
	} sym;
	double dbl;		/* cat NSC_VAL, type NSC_DOUBLE */
	struct {		/* cat NSC_VAL, type NSC_STRING, cat NSC_ID */
	    char *base;
	    size_t maxsz;
	} str;
	long lng;		/* cat NSC_VAL, type NSC_LONG */
    } val_as;
};

/* Compiled condition */
struct nscstr {
    char operator;		/* '<', '=', '>', '#' */
    enum nsc_type optype;	/* operator type */
    struct valstr lft;		/* left operand */
    struct valstr rgt;		/* right operand */
};

/* Selection type */
enum ns_seltype {
    NS_UNDEF,			/* error value */
    NS_LIST,			/* list of IDs */
    NS_DIST,			/* circular area */
    NS_AREA,			/* rectangular area */
    NS_ALL,			/* everything */
    NS_XY,			/* one sector area */
    NS_GROUP,			/* group, i.e. fleet, wing, army */
    NS_CARGO			/* loaded on the same carrier */
};

/* Sector iterator */
struct nstr_sect {
    coord x, y;			/* current x-y */
    coord dx, dy;		/* accumulated x,y travel */
    int id;			/* return value of sctoff */
    enum ns_seltype type;	/* selection type: NS_AREA or NS_DIST */
    int curdist;		/* NS_DIST: current range */
    struct range range;		/* area of coverage */
    int dist;			/* NS_DIST: range */
    coord cx, cy;		/* NS_DIST: center x-y */
    int ncond;			/* # of selection conditions */
    struct nscstr cond[NS_NCOND]; /* selection conditions */
};

/* Item iterator */
struct nstr_item {
    int cur;			/* current item */
    enum ns_seltype sel;	/* selection type, any but NS_UNDEF */
    int type;			/* item type being selected */
    int curdist;		/* if NS_DIST, current item's dist */
    struct range range;		/* NS_AREA/NS_DIST: range selector */
    int dist;			/* NS_DIST: distance selector */
    coord cx, cy;		/* NS_DIST: center x-y, NS_XY: xy */
    char group;			/* NS_GROUP: fleet/wing match */
    int next;			/* NS_CARGO: next item */
    int size;			/* NS_LIST: size of list */
    int index;			/* NS_LIST: index */
    int list[NS_LSIZE];		/* NS_LIST: item list */
    int ncond;			/* # of selection conditions */
    struct nscstr cond[NS_NCOND]; /* selection conditions */
};

/*
 * Symbol binding: associate @name with @value.
 */
struct symbol {
    int value;
    char *name;
};

/* Selector flags */
enum {
    NSC_DEITY = bit(0),		/* access restricted to deity */
    NSC_BITS = bit(3),		/* value consists of flag bits */
    NSC_HIDDEN = bit(4)		/* visibility depends on contact */
};

/* Selector use by xdump and xundump */
enum ca_dump {
    /* order is relevant */
    CA_DUMP,			/* xdump and xundump normally */
    CA_DUMP_CONST,		/* same, but value can't be changed  */
    CA_DUMP_ONLY,		/* only in xdump command */
    CA_DUMP_NONE		/* do not xdump or xundump */
};

/*
 * Selector descriptor
 *
 * A selector describes an attribute of some context object.
 * A selector with @ca_type NSC_NOTYPE is invalid.
 * If @ca_get is null, the selector describes a datum of type @ca_type
 * at offset @ca_offs in the context object.
 * A datum of type NSC_STRINGY is a string stored in an array of
 * @ca_len characters.  Ugly wart: if @ca_len is one, the terminating
 * null character may be omitted.
 * A datum of any other type is either a scalar of that type (if
 * @ca_len is zero), or an array of @ca_len elements of that type.
 * If @ca_get is not null, the selector is virtual.  Values can be
 * obtained by calling @ca_get(VAL, NP, CTXO), where VAL has been
 * initialized from the selector and an index by nstr_mksymval(),
 * NP points to the country to use for coordinate translation and
 * access control (null for none), and CTXO is the context object.
 * See struct valstr for details.
 * Because virtual selectors don't have a setter method, xundump must
 * be made to ignore them, by setting @ca_dump to CA_DUMP_NONE.
 * If @ca_table is not EF_BAD, the datum refers to that Empire table;
 * @ca_type must be an integer type.
 * If NSC_BITS is set in @ca_flags, the datum consists of flag bits,
 * and the referred table must be a symbol table defining those bits.
 * If NSC_DEITY is set in @ca_flags, only deities can use this
 * selector.
 * If NSC_HIDDEN is set in @ca_flags, the selector must be an array of
 * MAXNOC elements, indexed by country number.  Array elements are
 * masked for contact when opt_HIDDEN is on.  Only implemented for
 * integer types and context object EF_RELAT.
 * @ca_dump specifies how xdump and xundump are to use the selector.
 */
struct castr {
    char *ca_name;
    ptrdiff_t ca_off;
    enum nsc_type ca_type;
    unsigned short ca_len;
    void *(*ca_get)(struct valstr *, struct natstr *, void *);
    int ca_table;
    int ca_flags;
    enum ca_dump ca_dump;
};

/* Is CA an array? */
#define CA_IS_ARRAY(ca) ((ca)->ca_type != NSC_STRINGY && (ca)->ca_len != 0)

/* If CA is an array, return its length, else zero */
#define CA_ARRAY_LEN(ca) ((ca)->ca_type != NSC_STRINGY ? (ca)->ca_len : 0)

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
extern struct castr loan_ca[];
extern struct castr news_ca[];
extern struct castr lost_ca[];
extern struct castr commodity_ca[];
extern struct castr trade_ca[];
extern struct castr nat_ca[];
extern struct castr cou_ca[];
extern struct castr relat_ca[];
extern struct castr contact_ca[];
extern struct castr reject_ca[];
extern struct castr realm_ca[];
extern struct castr game_ca[];
extern struct castr intrchr_ca[];
extern struct castr rpt_ca[];
extern struct castr update_ca[];
extern struct castr empfile_ca[];
extern struct castr symbol_ca[];
extern struct symbol ship_chr_flags[];
extern struct symbol plane_chr_flags[];
extern struct symbol land_chr_flags[];
extern struct symbol nuke_chr_flags[];
extern struct castr mdchr_ca[];
extern struct symbol meta_type[];
extern struct symbol meta_flags[];
extern struct symbol missions[];
extern struct symbol plane_flags[];
extern struct symbol retreat_flags[];
extern struct symbol nation_status[];
extern struct symbol nation_flags[];
extern struct symbol nation_rejects[];
extern struct symbol nation_relations[];
extern struct symbol level[];
extern struct symbol agreement_statuses[];
extern struct symbol plague_stages[];
extern struct symbol packing[];
extern struct symbol resources[];
extern struct symbol sect_chr_flags[];
extern struct symbol sector_navigation[];

/* src/lib/common/nstreval.c */
extern struct valstr *nstr_mksymval(struct valstr *, struct castr *, int);
extern struct valstr *nstr_eval(struct valstr *, natid, void *,
				enum nsc_type);
extern int nstr_promote(int);
extern char *symbol_by_value(int, struct symbol *);
extern int symbol_set_fmt(char *, size_t, int, struct symbol *,
			  char *, int);
/* src/lib/global/nsc.c */
extern void nsc_init(void);
/* src/lib/subs/nxtitem.c */
extern int nxtitem(struct nstr_item *, void *);
/* src/lib/subs/nxtsct.c */
extern int nxtsct(struct nstr_sect *, struct sctstr *);
/* src/lib/subs/snxtitem.c */
extern int snxtitem(struct nstr_item *, int, char *, char *);
extern void snxtitem_area(struct nstr_item *, int, struct range *);
extern void snxtitem_dist(struct nstr_item *, int, int, int, int);
extern void snxtitem_xy(struct nstr_item *, int, coord, coord);
extern void snxtitem_all(struct nstr_item *, int);
extern void snxtitem_group(struct nstr_item *, int, char);
extern void snxtitem_rewind(struct nstr_item *);
extern int snxtitem_list(struct nstr_item *, int, int *, int);
extern void snxtitem_cargo(struct nstr_item *, int, int, int);
extern int snxtitem_use_condarg(struct nstr_item *);
/* src/lib/subs/snxtsct.c */
extern int snxtsct(struct nstr_sect *, char *);
extern void snxtsct_area(struct nstr_sect *, struct range *);
extern void snxtsct_all(struct nstr_sect *);
extern void snxtsct_rewind(struct nstr_sect *);
extern void snxtsct_dist(struct nstr_sect *, coord, coord, int);
extern int snxtsct_use_condarg(struct nstr_sect *);
/* src/lib/subs/nstr.c */
extern int nstr_comp(struct nscstr *np, int len, int type, char *str);
extern char *nstr_comp_val(char *, struct valstr *, int);
extern int nstr_exec(struct nscstr *, int, void *);
/* src/lib/update/nxtitemp.c */
extern void *nxtitemp(struct nstr_item *);

#endif
