/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  nstr.c: compile and execute the item selections on sectors
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1997
 *     Markus Armbruster, 2004-2008
 */

#include <config.h>

#include <ctype.h>
#include <limits.h>
#include "file.h"
#include "match.h"
#include "player.h"
#include "prototypes.h"

static char *nstr_parse_val(char *, struct valstr *);
static int nstr_match_ca(struct valstr *, struct castr *);
static int nstr_is_name_of_ca(struct valstr *, struct castr *, int);
static int nstr_ca_comparable(struct castr *, int, int);
static int nstr_match_val(struct valstr *, struct castr *, int);
static struct valstr *nstr_resolve_id(struct valstr *, struct castr *, int);
static struct valstr *nstr_resolve_val(struct valstr *, int, struct castr *);
static int nstr_optype(enum nsc_type, enum nsc_type);

/*
 * Compile conditions into array NP[LEN].
 * Return number of conditions, or -1 on error.
 * It is an error if there are more than LEN conditions.
 * TYPE is the context type, a file type.
 * STR is the condition string, in Empire syntax, without the leading
 * '?'.
 */
int
nstr_comp(struct nscstr *np, int len, int type, char *str)
{
    struct castr *ca = ef_cadef(type);
    char *cond;
    char *tail;
    int i;
    struct nscstr dummy;
    int lft_caidx, rgt_caidx;
    int lft_val, rgt_val;
    int two_sels;

    cond = str;
    for (i = 0; ; ++i, ++np) {
	if (i >= len)
	    np = &dummy;

	/* left operand */
	tail = nstr_parse_val(cond, &np->lft);
	lft_caidx = nstr_match_ca(&np->lft, ca);

	/* operator */
	if (*tail != '<' && *tail != '=' && *tail != '>' && *tail != '#') {
	    if (*tail)
		pr("%s -- expected condition operator\n", cond);
	    else
		pr("%s -- missing condition operator\n", cond);
	    return -1;
	}
	np->operator = *tail;
	++tail;

	/* right operand */
	tail = nstr_parse_val(tail, &np->rgt);
	rgt_caidx = nstr_match_ca(&np->rgt, ca);

	/*
	 * Resolve identifiers
	 *
	 * If just one operand is an identifier, it names a selector.
	 * If both operands are identifiers, things get complicated:
	 * either can then name a selector or a symbolic value for the
	 * selector named by the other operand.
	 */
	if (np->lft.val_cat == NSC_ID && np->rgt.val_cat == NSC_ID) {
	    lft_val = nstr_match_val(&np->lft, ca, rgt_caidx);
	    rgt_val = nstr_match_val(&np->rgt, ca, lft_caidx);
	    two_sels = nstr_ca_comparable(ca, lft_caidx, rgt_caidx);
	    /*
	     * If lft_val >= 0 interpreting rgt as a selector and lft
	     * as one of its values works.  Likewise for rgt_val >= 0.
	     * If two_sels, interpreting both lft and rgt as selector
	     * works.
	     */
	    switch ((lft_val >= 0) + (rgt_val >= 0) + !!two_sels) {
	    case 0:		/* no interpretation */
		if (lft_caidx >= 0 && rgt_caidx >= 0) {
		    /*
		     * Both identifiers name selectors.  Since
		     * !two_sels, they can't be comparable.
		     * Example: type=civil.
		     */
		    pr("%.*s -- not comparable\n", (int)(tail-cond), cond);
		    return -1;
		}
		/*
		 * At least one identifier doesn't name a selector,
		 * and nstr_resolve_id() will fail for it below
		 */
		break;
	    case 1:		/* one unambigous interpretation */
		break;
	    default:		/* multiple interpretations */
		/*
		 * Last-resort disambiguation: if the identifier is
		 * the unabbreviated name of a selector, discard
		 * value, else discard selector interpretation.
		 * Example: resolve wing=g to wing='g', not wing=group
		 * or 'wing'=group.
		 */
		if (nstr_is_name_of_ca(&np->lft, ca, lft_caidx))
		    lft_val = -1;
		else
		    two_sels = 0;
		if (nstr_is_name_of_ca(&np->rgt, ca, rgt_caidx))
		    rgt_val = -1;
		else
		    two_sels = 0;
		if ((lft_val >= 0) + (rgt_val >= 0) + !!two_sels == 1)
		    break;	/* last-resort disambiguation worked */
		/*
		 * Example: n<n for sectors could mean newdes<n or
		 * n<newdes.
		 */
		pr("%.*s -- condition ambiguous\n", (int)(tail-cond), cond);
		return -1;
	    }
	    /* resolve identifiers naming values */
	    if (lft_val >= 0)
		nstr_resolve_val(&np->lft, lft_val, &ca[rgt_caidx]);
	    if (rgt_val >= 0)
		nstr_resolve_val(&np->rgt, rgt_val, &ca[lft_caidx]);
	}
	/* remaining identifiers name selectors */
	if (!nstr_resolve_id(&np->lft, ca, lft_caidx))
	    return -1;
	if (!nstr_resolve_id(&np->rgt, ca, rgt_caidx))
	    return -1;

	/* find operator type */
	np->optype = nstr_optype(np->lft.val_type, np->rgt.val_type);
	if (np->optype == NSC_NOTYPE) {
	    pr("%.*s -- not comparable\n", (int)(tail-cond), cond);
	    return -1;
	}

	/* another condition? */
	if (*tail == 0)
	    break;
	if (*tail != '&') {
	    pr("%s -- expected `&'\n", cond);
	    return -1;
	}
	cond = tail + 1;
    }

    if (i >= len) {
	/* could just return I and let caller gripe or enlarge buffer */
	pr("%s -- too many conditions\n", str);
	return -1;
    }

    return i + 1;
}

/* Like strcmp(S1, S2), but limit length of S1 to SZ1 and of S2 to SZ2.  */
static int
strnncmp(char *s1, size_t sz1, char *s2, size_t sz2)
{
    int res;
    if (sz1 == sz2)
	return strncmp(s1, s2, sz2);
    if (sz1 < sz2)
	return -strnncmp(s2, sz2, s1, sz1);
    res = strncmp(s1, s2, sz2);
    return res ? res : s1[sz2];
}

#define EVAL(op, lft, rgt)			\
    ((op) == '<' ? (lft) < (rgt)		\
     : (op) == '=' ? (lft) == (rgt)		\
     : (op) == '>' ? (lft) > (rgt)		\
     : (op) == '#' ? (lft) != (rgt)		\
     : (CANT_REACH(), 0))

/*
 * Evaluate compiled conditions in array NP[NCOND].
 * Return non-zero iff they are all true.
 * PTR points to a context object of the type that was used to compile
 * the conditions.
 */
int
nstr_exec(struct nscstr *np, int ncond, void *ptr)
{
    int i, op, optype, cmp;
    struct valstr lft, rgt;

    for (i = 0; i < ncond; ++i) {
	op = np[i].operator;
	optype = np[i].optype;
	if (np[i].lft.val_cat == NSC_NOCAT || np[i].rgt.val_cat == NSC_NOCAT)
	    return 0;
	lft = np[i].lft;
	nstr_exec_val(&lft, player->cnum, ptr, optype);
	rgt = np[i].rgt;
	nstr_exec_val(&rgt, player->cnum, ptr, optype);
	switch (optype) {
	case NSC_LONG:
	    if (!EVAL(op, lft.val_as.lng, rgt.val_as.lng))
		return 0;
	    break;
	case NSC_DOUBLE:
	    if (!EVAL(op, lft.val_as.dbl, rgt.val_as.dbl))
		return 0;
	    break;
	case NSC_STRING:
	    cmp = strnncmp(lft.val_as.str.base, lft.val_as.str.maxsz,
			   rgt.val_as.str.base, rgt.val_as.str.maxsz);
	    if (!EVAL(op, cmp, 0))
		return 0;
	    break;
	default:
	    CANT_REACH();
	    return 0;
	}
    }

    return 1;
}

/*
 * Parse a value in STR into VAL.
 * Return a pointer to the first character after the value.
 * Value is either evaluated into NSC_STRING, NSC_DOUBLE or NSC_LONG,
 * or an identifier.
 */
static char *
nstr_parse_val(char *str, struct valstr *val)
{
    long l;
    double d;
    char *tail, *tail2;

    /* string */
    if (str[0] == '\'') {
	for (tail = str + 1; *tail && *tail != '\''; ++tail) ;
	/* FIXME implement \ quoting */
	val->val_type = NSC_STRING;
	val->val_cat = NSC_VAL;
	val->val_as.str.base = str + 1;
	val->val_as.str.maxsz = tail - (str + 1);
	if (*tail)
	    ++tail;
	/* FIXME else unclosed string */
	return tail;
    }

    /* identifier */
    if (isalpha(str[0])) {
	for (tail = str+1; isalnum(*tail) || *tail == '_'; ++tail) ;
	val->val_type = NSC_NOTYPE;
	val->val_cat = NSC_ID;
	val->val_as.str.base = str;
	val->val_as.str.maxsz = tail - str;
	return tail;
    }

    /* number */
    l = strtol(str, &tail, 0);
    d = strtod(str, &tail2);
    if (tail2 > tail) {
	val->val_type = NSC_DOUBLE;
	val->val_cat = NSC_VAL;
	val->val_as.dbl = d;
	return tail2;
    }
    if (tail != str) {
	val->val_type = NSC_LONG;
	val->val_cat = NSC_VAL;
	val->val_as.lng = l;
	return tail;
    }

    /* funny character, interpret as identifier */
    tail = str+1;
    val->val_type = NSC_NOTYPE;
    val->val_cat = NSC_ID;
    val->val_as.str.base = str;
    val->val_as.str.maxsz = tail - str;
    return tail;
}

/*
 * Match VAL in table of selector descriptors CA, return index.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 * A VAL that is not an identifier doesn't match anything.  A null CA
 * is considered empty.
 */
static int
nstr_match_ca(struct valstr *val, struct castr *ca)
{
    char id[32];

    if (val->val_cat != NSC_ID || val->val_as.str.maxsz >= sizeof(id))
	return M_NOTFOUND;

    if (!ca)
	return M_NOTFOUND;

    memcpy(id, val->val_as.str.base, val->val_as.str.maxsz);
    id[val->val_as.str.maxsz] = 0;

    return stmtch(id, ca, offsetof(struct castr, ca_name),
		  sizeof(struct castr));
}

/*
 * Is identifier VAL the name of the selector given by CA and IDX?
 * Return non-zero if and only if IDX is non-negative and VAL is the
 * name of CA[IDX].
 * IDX must have been obtained from nstr_match_ca(VAL, CA).
 */
static int
nstr_is_name_of_ca(struct valstr *val, struct castr *ca, int idx)
{
    if (CANT_HAPPEN(val->val_cat != NSC_ID && idx >= 0))
	return 0;
    return idx >= 0 && strlen(ca[idx].ca_name) == val->val_as.str.maxsz;
}

/*
 * Do we have two comparable selectors?
 * Check selector descriptors CA[LFT_IDX] (unless LFT_IDX is negative)
 * and CA[RGT_IDX] (unless RGT_IDX is negative).  CA may be null when
 * both are negative.
 */
static int
nstr_ca_comparable(struct castr *ca, int lft_idx, int rgt_idx)
{
    if (lft_idx < 0 || rgt_idx < 0)
	return 0;
    if (ca[lft_idx].ca_table != ca[rgt_idx].ca_table)
	return 0;		/* Example: land type=spy */
    return nstr_optype(ca[lft_idx].ca_type, ca[rgt_idx].ca_type)
	!= NSC_NOTYPE;		/* Example: ship name=effic */
}

/*
 * Match VAL in a selector's values, return its (non-negative) value.
 * Match values of selector descriptor CA[IDX], provided IDX is not
 * negative.  CA may be null when IDX is negative.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 */
static int
nstr_match_val(struct valstr *val, struct castr *ca, int idx)
{
    char id[32];
    enum nsc_type type;

    if (val->val_cat != NSC_ID || idx < 0)
	return M_NOTFOUND;

    type = nstr_promote(ca[idx].ca_type);
    if (type == NSC_STRING)
	return 0;

    if (ca[idx].ca_table == EF_BAD || CANT_HAPPEN(type != NSC_LONG))
	return M_NOTFOUND;

    if (val->val_as.str.maxsz >= sizeof(id))
	return M_NOTFOUND;
    memcpy(id, val->val_as.str.base, val->val_as.str.maxsz);
    id[val->val_as.str.maxsz] = 0;
    return ef_elt_byname(ca[idx].ca_table, id);
}

/*
 * Change VAL to resolve identifier to selector.
 * Return VAL on success, NULL on error.
 * No change if VAL is not an identifier.
 * Else change VAL into symbolic value for selector CA[IDX] if IDX >=
 * 0, and error if not.
 */
static struct valstr *
nstr_resolve_id(struct valstr *val, struct castr *ca, int idx)
{
    if (val->val_cat != NSC_ID)
	return val;

    if (idx == M_NOTUNIQUE) {
	pr("%.*s -- ambiguous name\n",
	   (int)val->val_as.str.maxsz, val->val_as.str.base);
	val->val_cat = NSC_NOCAT;
	return NULL;
    }

    if (idx == M_NOTFOUND) {
	pr("%.*s -- unknown name\n",
	   (int)val->val_as.str.maxsz, val->val_as.str.base);
	val->val_cat = NSC_NOCAT;
	return NULL;
    }

    if ((ca[idx].ca_flags & NSC_DEITY) && !player->god) {
	pr("%.*s -- not accessible to mortals\n",
	   (int)val->val_as.str.maxsz, val->val_as.str.base);
	val->val_cat = NSC_NOCAT;
	return NULL;
    }

    return nstr_mksymval(val, &ca[idx], 0);
}

/*
 * Change VAL to resolve identifier to value SELVAL for selector CA.
 * Return VAL.
 * VAL must be an identifier, and SELVAL must have been obtained from
 * nstr_match_val(VAL, CA0, IDX), where CA = &CA0[IDX].
 */
static struct valstr *
nstr_resolve_val(struct valstr *val, int selval, struct castr *ca)
{
    enum nsc_type type = nstr_promote(ca->ca_type);

    if (CANT_HAPPEN(val->val_cat != NSC_ID)) {
	val->val_cat = NSC_NOCAT;
	return val;
    }

    if (type == NSC_STRING) {
	val->val_type = NSC_STRING;
	val->val_cat = NSC_VAL;
	/* map identifier ~ to empty string, like some commands do */
	if (val->val_as.str.maxsz == 1 && val->val_as.str.base[0] == '~')
	    val->val_as.str.maxsz = 0;
	return val;
    }

    if (CANT_HAPPEN(type != NSC_LONG || ca->ca_table == EF_BAD)) {
	val->val_type = NSC_NOTYPE;
	val->val_cat = NSC_NOCAT;
	return val;
    }

    val->val_type = type;
    val->val_cat = NSC_VAL;
    val->val_as.lng = selval;
    return val;
}

/*
 * Return operator type for operand types LFT, RGT.
 */
static int
nstr_optype(enum nsc_type lft, enum nsc_type rgt)
{
    lft = nstr_promote(lft);
    rgt = nstr_promote(rgt);
    if (lft == rgt)
	return lft;
    if (lft == NSC_DOUBLE && rgt == NSC_LONG)
	return NSC_DOUBLE;
    if (rgt == NSC_DOUBLE && lft == NSC_LONG)
	return NSC_DOUBLE;
    return NSC_NOTYPE;
}

/*
 * Compile a value in STR into VAL.
 * Return a pointer to the first character after the value on success,
 * NULL on error.
 * TYPE is the context type, a file type.
 * If STR names an array, VAL simply refers to the element with index
 * zero.
 */
char *
nstr_comp_val(char *str, struct valstr *val, int type)
{
    struct castr *ca = ef_cadef(type);
    char *tail = nstr_parse_val(str, val);
    if (!nstr_resolve_id(val, ca, nstr_match_ca(val, ca)))
	return NULL;
    return tail;
}
