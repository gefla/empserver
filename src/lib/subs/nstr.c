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
 *  nstr.c: compile and execute the item selections on sectors
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1997
 *     Markus Armbruster, 2004
 */

#include "misc.h"
#include "file.h"
#include "match.h"
#include "nsc.h"

static int nstr_promote(int valtype);

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
    char *cond;
    char *tail;
    int i;
    struct nscstr dummy;
    int lft_type, rgt_type;

    cond = str;
    for (i = 0; ; ++i, ++np) {
	if (i >= len)
	    np = &dummy;

	/* left operand */
	tail = nstr_comp_val(cond, &np->lft, type);
	if (!tail)
	    return -1;

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
	tail = nstr_comp_val(tail, &np->rgt, type);
	if (!tail)
	    return -1;

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
    int i, op, optype;
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
	    CANT_HAPPEN("unimplemented OPTYPE"); /* FIXME */
	    return 0;
	default:
	    CANT_HAPPEN("bad OPTYPE");
	    return 0;
	}
    }

    return 1;
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
nstr_comp_val(char *str, struct valstr*val, int type)
{
    char id[32];
    long l;
    double d;
    char *tail, *tail2;
    struct castr *cap;
    unsigned i;
    int j;

    val->val_type = NSC_NOTYPE;
    val->val_cat = NSC_NOCAT;
    val->val_as_type = -1;

    if (isalpha(str[0])) {
	/* identifier */
	for (i = 0; isalnum(str[i]) || str[i] == '_'; ++i) {
	    if (i < sizeof(id) - 1)
		id[i] = str[i];
	}
	tail = str + i;
	if (i < sizeof(id)) {
	    id[i] = 0;

	    val->val_as_type = typematch(id, type);

	    cap = ef_cadef(type);
	    if (cap) {
		j = stmtch(id, cap, offsetof(struct castr, ca_name),
			   sizeof(struct castr));
		if (j >= 0
		    && (!(cap[j].ca_flags & NSC_DEITY) || player->god)) {
		    if (cap[j].ca_type == NSC_TYPEID && val->val_as_type >= 0)
			/*
			 * Got two matches of type NSC_TYPEID, need to
			 * choose.  Prefer typematch(), because ?des=n
			 * would be interpreted as ?des=newdes
			 * otherwise
			 */
			;
		    else {
			val->val_type = cap[j].ca_type;
			val->val_cat = NSC_OFF;
			val->val_as.sym.off = cap[j].ca_off;
			val->val_as.sym.idx = 0;
		    }
		}
	    } else
		j = M_NOTFOUND;
	} else
	    j = M_NOTFOUND;

	if (val->val_type == NSC_NOTYPE) {
	    if (val->val_as_type >= 0) {
		val->val_type = NSC_TYPEID;
		val->val_cat = NSC_VAL;
		val->val_as.lng = val->val_as_type;
	    } else if (j >= 0)
		pr("%s -- selector access denied\n", id);
	    else if (j == M_NOTUNIQUE)
		pr("%s -- ambiguous selector name\n", id);
	    else
		pr("%s -- unknown selector name\n", id);
	}

	return val->val_type == NSC_NOTYPE ? NULL : tail;
    }

    /* single character type */
    id[0] = str[0];
    id[1] = 0;
    val->val_as_type = typematch(id, type);

    /* literals */
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
    /* FIXME implement NSC_STRING literals */

    CANT_HAPPEN(val->val_type != NSC_NOTYPE);
    if (val->val_as_type >= 0) {
	val->val_type = NSC_TYPEID;
	val->val_cat = NSC_VAL;
	val->val_as.lng = val->val_as_type;
	return str + 1;
    }

    pr("%s -- invalid value for condition\n", str);
    return NULL;
}

/*
 * Promote VALTYPE.
 * If VALTYPE is an integer type, return NSC_LONG.
 * If VALTYPE is a floating-point type, return NSC_DOUBLE.
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
	    if (val->val_as_type >= 0) {
		val->val_cat = NSC_VAL;
		val->val_as.lng = val->val_as_type;
	    } else
		return cond_type_mismatch(str);
	    break;
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
    nsc_type valtype = NSC_LONG;
    int idx;

    switch (val->val_cat) {
    default:
	CANT_HAPPEN("Bad VAL category");
	/* fall through */
    case NSC_VAL:
	valtype = val->val_type;
	break;
    case NSC_OFF:
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
	case NSC_STRING:
	    val->val_as.str = *(char **)memb_ptr;
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
	case NSC_STRING: val->val_as.str = ""; break;
	default:
	    CANT_HAPPEN("bad WANT argument");
	}
    }

    val->val_type = valtype;
}
