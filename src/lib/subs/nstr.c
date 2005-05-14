/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nstr.c: compile and execute the item selections on sectors
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1997
 *     Markus Armbruster, 2004
 */

#include <limits.h>
#include "misc.h"
#include "file.h"
#include "match.h"
#include "nsc.h"
#include "prototypes.h"

static char *nstr_parse_val(char *, struct valstr *);
static int nstr_match_ca(struct valstr *, struct castr *);
static int nstr_match_val(struct valstr *, int, struct castr *, int);
static struct valstr *nstr_resolve_sel(struct valstr *, struct castr *);
static struct valstr *nstr_mkselval(struct valstr *, int, struct castr *);
static struct valstr *nstr_resolve_id(struct valstr *, struct castr *, int, int);
static int nstr_promote(int);


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
    int lft_type, rgt_type;

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
	 * An identifier can name a selector or, if the other operand
	 * is a selector, a value for that.  The condition is
	 * ambiguous if both selector x value and value x selector are
	 * possible.  Example: n<n for sectors could mean newdes<n or
	 * n<newdes.
	 */
	lft_val = nstr_match_val(&np->lft, type, ca, rgt_caidx);
	rgt_val = nstr_match_val(&np->rgt, type, ca, lft_caidx);
	/*
	 * if lft_val >= 0, then rhs names a selector and lhs names
	 * one of its values.  Likewise for rgt_val.
	 */
	if (lft_val >= 0 && rgt_val >= 0) {
	    pr("%.*s -- condition ambiguous\n", (int)(tail-cond), cond);
	    return -1;
	} else if (rgt_val >= 0) {
	    /* selector x value */
	    if (!nstr_resolve_sel(&np->lft, &ca[lft_caidx]))
		return -1;
	    nstr_mkselval(&np->rgt, rgt_val, &ca[lft_caidx]);
	} else if (lft_val >= 0) {
	    /* value x selector */
	    nstr_mkselval(&np->lft, lft_val, &ca[rgt_caidx]);
	    if (!nstr_resolve_sel(&np->rgt, &ca[rgt_caidx]))
		return -1;
	} else {
	    /*
	     * Neither side works as selector value; any identifiers
	     * must name selectors.
	     */
	    if (!nstr_resolve_id(&np->lft, ca, lft_caidx,
				 nstr_promote(ca[rgt_caidx].ca_type)
				 == NSC_STRING))
		return -1;
	    if (!nstr_resolve_id(&np->rgt, ca, rgt_caidx,
				 nstr_promote(ca[lft_caidx].ca_type)
				 == NSC_STRING))
		return -1;
	}

	/* find operator type, coerce operands */
	lft_type = nstr_promote(np->lft.val_type);
	rgt_type = nstr_promote(np->rgt.val_type);
	np->optype = NSC_NOTYPE;
	if (lft_type == NSC_TYPEID) {
	    if (!nstr_coerce_val(&np->rgt, NSC_TYPEID, str))
		np->optype = NSC_TYPEID;
	} else if (rgt_type == NSC_TYPEID) {
	    if (!nstr_coerce_val(&np->lft, NSC_TYPEID, str))
		np->optype = NSC_TYPEID;
	} else if (lft_type == NSC_STRING) {
	    if (!nstr_coerce_val(&np->rgt, NSC_STRING, str))
		np->optype = NSC_STRING;
	} else if (rgt_type == NSC_STRING) {
	    if (!nstr_coerce_val(&np->lft, NSC_STRING, str))
		np->optype = NSC_STRING;
	} else if (lft_type == NSC_DOUBLE) {
	    if (!nstr_coerce_val(&np->rgt, NSC_DOUBLE, str))
		np->optype = NSC_DOUBLE;
	} else if (rgt_type == NSC_DOUBLE) {
	    if (!nstr_coerce_val(&np->lft, NSC_DOUBLE, str))
		np->optype = NSC_DOUBLE;
	} else {
	    if (!nstr_coerce_val(&np->lft, NSC_LONG, str)
		&& !nstr_coerce_val(&np->rgt, NSC_LONG, str))
		np->optype = NSC_LONG;
	}
	if (np->optype == NSC_NOTYPE)
	    return -1;

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
    if (sz1 == sz2) return strncmp(s1, s2, sz2);
    if (sz1 < sz2) return -strnncmp(s2, sz2, s1, sz1);
    res = strncmp(s1, s2, sz2);
    return res ? res : s1[sz2];
}

#define EVAL(op, lft, rgt)			\
    ((op) == '<' ? (lft) < (rgt)		\
     : (op) == '=' ? (lft) == (rgt)		\
     : (op) == '>' ? (lft) > (rgt)		\
     : (op) == '#' ? (lft) != (rgt)		\
     : 0)

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
	case NSC_TYPEID:
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
	    CANT_HAPPEN("bad OPTYPE");
	    return 0;
	}
    }

    return 1;
}

/*
 * Parse a value in STR into VAL.
 * Return a pointer to the first character after the value.
 * Value is either evaluated (but not NSC_TYPEID) or an identifier.
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
	if (*tail) ++tail;
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
 * Match VAL in a selector's values, return its (non-negative) value.
 * TYPE is the context type, a file type.
 * CA is ef_cadef(TYPE).
 * Match values of selector descriptor CA[IDX], provided CA is not
 * null and IDX is not negative.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 * TODO: This is just a stub and works only for NSC_TYPEID.
 * Generalize: give struct castr enough info to find values, remove
 * parameter `type'.
 */
static int
nstr_match_val(struct valstr *val, int type, struct castr *ca, int idx)
{
    char id[32];

    if (val->val_cat != NSC_ID || val->val_as.str.maxsz >= sizeof(id))
	return M_NOTFOUND;

    if (idx < 0 || ca[idx].ca_type != NSC_TYPEID)
	return M_NOTFOUND;

    memcpy(id, val->val_as.str.base, val->val_as.str.maxsz);
    id[val->val_as.str.maxsz] = 0;

    return typematch(id, type);
}

/*
 * Change VAL to resolve identifier to selector or string.
 * Return VAL on success, NULL on error.
 * No change if VAL is not an identifier.  Otherwise, change it as
 * follows.
 * Error if IDX == M_NOTUNIQUE or IDX == M_NOTFOUND and !STRING_OK.
 * Change into string if IDX == M_NOTFOUND and STRING_OK.
 * Change into selector CA[IDX] if IDX >= 0.
 */
static struct valstr *
nstr_resolve_id(struct valstr *val, struct castr *ca, int idx, int string_ok)
{
    if (val->val_cat != NSC_ID)
	return val;

    if (idx == M_NOTUNIQUE) {
	pr("%.*s -- ambiguous name\n",
	   (int)val->val_as.str.maxsz, val->val_as.str.base);
	val->val_cat = NSC_NOCAT;
	return NULL;
    }

    if (idx < 0) {
	CANT_HAPPEN(idx != M_NOTFOUND);
	if (!string_ok) {
	    pr("%.*s -- unkown name\n",
	       (int)val->val_as.str.maxsz, val->val_as.str.base);
	    val->val_cat = NSC_NOCAT;
	    return NULL;
	}
	/* interpret unbound identifier as string */
	val->val_type = NSC_STRING;
	val->val_cat = NSC_VAL;
	return val;
    }

    return nstr_resolve_sel(val, &ca[idx]);
}

/*
 * Change VAL to resolve identifier to selector CA.
 * Return VAL on success, NULL if the player is denied access to the
 * selector.
 * VAL must be an identifier.
 */
static struct valstr *
nstr_resolve_sel(struct valstr *val, struct castr *ca)
{
    if (CANT_HAPPEN(val->val_cat != NSC_ID)) {
	val->val_cat = NSC_NOCAT;
	return val;
    }

    if ((ca->ca_flags & NSC_DEITY) && !player->god) {
	pr("%.*s -- not accessible to mortals\n",
	   (int)val->val_as.str.maxsz, val->val_as.str.base);
	val->val_cat = NSC_NOCAT;
	return NULL;
    }

    val->val_type = ca->ca_type;
    val->val_cat = NSC_OFF;
    val->val_as.sym.off = ca->ca_off;
    val->val_as.sym.len = ca->ca_len;
    val->val_as.sym.idx = 0;
    return val;
}

/*
 * Initialize VAL to value SELVAL for selector CA, return VAL.
 */
static struct valstr *
nstr_mkselval(struct valstr *val, int selval, struct castr *ca)
{
    if (CANT_HAPPEN(ca->ca_type != NSC_TYPEID)) {
	val->val_type = NSC_NOTYPE;
	val->val_cat = NSC_NOCAT;
	return val;
    }

    val->val_type = ca->ca_type;
    val->val_cat = NSC_VAL;
    val->val_as.lng = selval;
    return val;
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
    return nstr_resolve_id(val, ca, nstr_match_ca(val, ca), 0) ? tail : NULL;
}


/*
 * Promote VALTYPE.
 * If VALTYPE is an integer type, return NSC_LONG.
 * If VALTYPE is a floating-point type, return NSC_DOUBLE.
 * If VALTYPE is NSC_STRINGY, return NSC_STRING.
 * If VALTYPE is NSC_NOTYPE, NSC_STRING or NSC_TYPEID, return VALTYPE.
 */
static int
nstr_promote(int valtype)
{
    switch (valtype) {
    case NSC_NOTYPE:
    case NSC_LONG:
    case NSC_DOUBLE:
    case NSC_STRING:
    case NSC_TYPEID:
	break;
    case NSC_CHAR:
    case NSC_UCHAR:
    case NSC_SHORT:
    case NSC_USHORT:
    case NSC_INT:
    case NSC_XCOORD:
    case NSC_YCOORD:
    case NSC_TIME:
	valtype = NSC_LONG;
	break;
    case NSC_FLOAT:
	valtype = NSC_DOUBLE;
	break;
    case NSC_STRINGY:
	valtype = NSC_STRING;
	break;
    default:
	CANT_HAPPEN("bad VALTYPE");
	valtype = NSC_NOTYPE;
    }
    return valtype;
}

static int
cond_type_mismatch(char *str)
{
    if (str)
	pr("%s -- condition operand type mismatch\n", str);
    return -1;
}

/*
 * Coerce VAL to promoted value type TO.
 * Return 0 on success, -1 on error.
 * If VAL is evaluated, convert it, else only check.
 * STR is the condition text to be used for error messages.  Suppress
 * messages if it is a null pointer.
 */
int
nstr_coerce_val(struct valstr *val, nsc_type to, char *str)
{
    /* FIXME get rid of promotion?  */
    nsc_type from = nstr_promote(val->val_type);

    if (from == NSC_NOTYPE)
	return 0;

    if (from != to) {
	switch (to) {
	case NSC_TYPEID:
	    return cond_type_mismatch(str);
	case NSC_STRING:
	    return cond_type_mismatch(str); /* FIXME implement */
	case NSC_DOUBLE:
	    if (from == NSC_LONG) {
		if (val->val_cat == NSC_VAL)
		    val->val_as.dbl = val->val_as.lng;
	    } else
		return cond_type_mismatch(str);
	    break;
	case NSC_LONG:
	    return cond_type_mismatch(str);
	default:
	    CANT_HAPPEN("bad TO argument");
	    to = from;
	}
    }

    if (val->val_cat == NSC_VAL) {
	/* coord literals don't occur, conversion not implemented */
	CANT_HAPPEN(val->val_type == NSC_XCOORD
		    || val->val_type == NSC_YCOORD);
	val->val_type = to;
    }

    return 0;
}

/*
 * Evaluate VAL.
 * If VAL is symbolic, evaluate it into a promoted value type.
 * Use coordinate system of country CNUM.
 * PTR points to a context object of the type that was used to compile
 * the value.
 * Unless WANT is NSC_NOTYPE, coerce the value to promoted value type
 * WANT.  VAL must be coercible.  That's the case if a previous
 * nstr_coerce_val(VAL, WANT, STR) succeeded.
 */
void
nstr_exec_val(struct valstr *val, natid cnum, void *ptr, nsc_type want)
{
    char *memb_ptr;
    nsc_type valtype;
    int idx;

    switch (val->val_cat) {
    default:
	CANT_HAPPEN("Bad VAL category");
	/* fall through */
    case NSC_VAL:
	valtype = val->val_type;
	break;
    case NSC_OFF:
	valtype = NSC_LONG;
	memb_ptr = ptr;
	memb_ptr += val->val_as.sym.off;
	idx = val->val_as.sym.idx;
	switch (val->val_type) {
	case NSC_CHAR:
	    val->val_as.lng = ((signed char *)memb_ptr)[idx];
	    break;
	case NSC_UCHAR:
	    val->val_as.lng = ((unsigned char *)memb_ptr)[idx];
	    break;
	case NSC_SHORT:
	    val->val_as.lng = ((short *)memb_ptr)[idx];
	    break;
	case NSC_USHORT:
	    val->val_as.lng = ((unsigned short *)memb_ptr)[idx];
	    break;
	case NSC_INT:
	    val->val_as.lng = ((int *)memb_ptr)[idx];
	    break;
	case NSC_LONG:
	    val->val_as.lng = ((long *)memb_ptr)[idx];
	    break;
	case NSC_XCOORD:
	    val->val_as.lng = xrel(getnatp(cnum), ((short *)memb_ptr)[idx]);
	    break;
	case NSC_YCOORD:
	    val->val_as.lng = yrel(getnatp(cnum), ((short *)memb_ptr)[idx]);
	    break;
	case NSC_FLOAT:
	    val->val_as.dbl = ((float *)memb_ptr)[idx];
	    valtype = NSC_DOUBLE;
	    break;
	case NSC_DOUBLE:
	    val->val_as.dbl = ((double *)memb_ptr)[idx];
	    valtype = NSC_DOUBLE;
	    break;
	case NSC_STRINGY:
	    CANT_HAPPEN(idx);
	    val->val_as.str.maxsz = val->val_as.sym.len;
	    val->val_as.str.base = (char *)memb_ptr;
	    valtype = NSC_STRING;
	    break;
	case NSC_STRING:
	    val->val_as.str.base = ((char **)memb_ptr)[idx];
	    val->val_as.str.maxsz = INT_MAX;
	    valtype = NSC_STRING;
	    break;
	case NSC_TIME:
	    val->val_as.lng = ((time_t *)memb_ptr)[idx];
	    break;
	case NSC_TYPEID:
	    val->val_as.lng = ((signed char *)memb_ptr)[idx];
	    valtype = NSC_TYPEID;
	    break;
	default:
	    CANT_HAPPEN("Bad VAL type");
	    val->val_as.lng = 0;
	}
	val->val_cat = NSC_VAL;
    }

    if (valtype == want)
	;
    else if (want == NSC_DOUBLE) {
	if (valtype == NSC_LONG) {
	    valtype = want;
	    val->val_as.dbl = val->val_as.lng;
	}
    } else if (want == NSC_STRING)
	CANT_HAPPEN("unimplemented WANT"); /* FIXME */

    if (CANT_HAPPEN(valtype != want && want != NSC_NOTYPE)) {
	valtype = want;
	switch (want) {
	case NSC_TYPEID:
	case NSC_LONG: val->val_as.lng = 0; break;
	case NSC_DOUBLE: val->val_as.dbl = 0.0; break;
	case NSC_STRING: val->val_as.str.base = NULL; break;
	default:
	    CANT_HAPPEN("bad WANT argument");
	}
    }

    val->val_type = valtype;
}
