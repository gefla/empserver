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
 *  nstreval.c: evaluate compiled values
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1997
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "nat.h"
#include "nsc.h"
#include "optlist.h"

/*
 * Initialize @val to symbolic value for selector @ca with index @idx.
 * Return @val.
 */
struct valstr *
nstr_mksymval(struct valstr *val, struct castr *ca, int idx)
{
    val->val_type = ca->ca_type;
    val->val_cat = NSC_OFF;
    val->val_as.sym.off = ca->ca_off;
    val->val_as.sym.len = ca->ca_len;
    val->val_as.sym.idx = idx;
    val->val_as.sym.get = ca->ca_get;
    val->val_as.sym.hidden = ca->ca_flags & NSC_HIDDEN;
    return val;
}

/*
 * Evaluate @val.
 * If @val has category NSC_OFF, read the value from the context object
 * @ptr, and translate it for country @cnum (coordinate system and
 * contact status).  No translation when @cnum is NATID_BAD.
 * @ptr points to a context object of the type that was used to compile
 * the value.
 * Unless @want is NSC_NOTYPE, coerce the value to promoted value type
 * @want.  @val must be coercible.
 * The result's type is promoted on success, NSC_NOTYPE on error.
 * In either case, the category is NSC_VAL.
 * Return @val.
 */
struct valstr *
nstr_eval(struct valstr *val, natid cnum, void *ptr, enum nsc_type want)
{
    char *memb_ptr;
    enum nsc_type valtype;
    int idx, hidden;
    coord c;
    struct natstr *natp;
    struct relatstr *relp;

    if (CANT_HAPPEN(want != NSC_NOTYPE && !NSC_IS_PROMOTED(want)))
	want = nstr_promote(want);

    switch (val->val_cat) {
    case NSC_VAL:
	valtype = val->val_type;
	if (CANT_HAPPEN(!NSC_IS_PROMOTED(valtype)))
	    valtype = nstr_promote(valtype);
	break;
    case NSC_OFF:
	if (val->val_as.sym.get) {
	    natp = getnatp(cnum);
	    do {
		ptr = val->val_as.sym.get(val, natp, ptr);
	    } while (ptr && val->val_as.sym.get);
	    if (!ptr) {
		valtype = val->val_type;
		val->val_cat = NSC_VAL;
		break;
	    }
	}

	valtype = NSC_LONG;
	memb_ptr = ptr;
	memb_ptr += val->val_as.sym.off;
	idx = val->val_as.sym.idx;
	hidden = val->val_as.sym.hidden;
	val->val_cat = NSC_VAL;
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
	    c = ((short *)memb_ptr)[idx];
	    if (cnum == NATID_BAD) {
		/* FIXME use variant of xrel() that takes orig instead of nation */
		if (c >= WORLD_X / 2)
		    c -= WORLD_X;
	    } else
		c = xrel(getnatp(cnum), c);
	    val->val_as.lng = c;
	    break;
	case NSC_YCOORD:
	    c = ((short *)memb_ptr)[idx];
	    if (cnum == NATID_BAD) {
		/* FIXME use variant of yrel() that takes orig instead of nation */
		if (c >= WORLD_Y / 2)
		    c -= WORLD_Y;
	    } else
		c = yrel(getnatp(cnum), c);
	    val->val_as.lng = c;
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
				/* really SIZE_MAX, but that's C99 */
	    valtype = NSC_STRING;
	    break;
	case NSC_TIME:
	    val->val_as.lng = ((time_t *)memb_ptr)[idx];
	    break;
	default:
	    CANT_REACH();
	    valtype = NSC_NOTYPE;
	}

	if (hidden) {
	    if (CANT_HAPPEN(hidden && valtype != NSC_LONG))
		break;		/* not implemented */
	    relp = ptr;
	    if (CANT_HAPPEN(relp->ef_type != EF_RELAT))
		break;		/* only defined for nation selectors */
	    if (!opt_HIDDEN || cnum == NATID_BAD
		|| getnatp(cnum)->nat_stat == STAT_GOD)
		break;
	    if (!in_contact(cnum, idx) || !in_contact(relp->rel_uid, idx))
		val->val_as.lng = -1;
	}
	break;
    default:
	CANT_REACH();
	valtype = NSC_NOTYPE;
    }

    /* coerce */
    if (valtype == want)
	;
    else if (want == NSC_DOUBLE) {
	if (valtype == NSC_LONG) {
	    valtype = want;
	    val->val_as.dbl = val->val_as.lng;
	}
    }

    if (CANT_HAPPEN(valtype != want && want != NSC_NOTYPE))
	valtype = NSC_NOTYPE;

    val->val_type = valtype;
    return val;
}

/*
 * Promote @valtype.
 * If @valtype is an integer type, return NSC_LONG.
 * If @valtype is a floating-point type, return NSC_DOUBLE.
 * If @valtype is a string type, return NSC_STRING.
 */
int
nstr_promote(int valtype)
{
    switch (valtype) {
    case NSC_LONG:
    case NSC_DOUBLE:
    case NSC_STRING:
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
	CANT_REACH();
	valtype = NSC_NOTYPE;
    }
    return valtype;
}

char *
symbol_by_value(int key, struct symbol *table)
{
    int i;

    for (i = 0; table[i].name; i++)
	if (key == table[i].value)
	    return table[i].name;

    return NULL;
}

int
symbol_set_fmt(char *buf, size_t sz, int flags, struct symbol symtab[],
	       char *sep, int all)
{
    char *pfx = "";
    int n, i;
    char *p;

    if (buf && sz)
	buf[0] = 0;
    n = 0;
    for (i = 0; i < 32; i++) {
	if (!(flags & bit(i)))
	    continue;
	p = symbol_by_value(bit(i), symtab);
	if (p)
	    n += snprintf(buf + n, sz - n, "%s%s", pfx, p);
	else if (all)
	    n += snprintf(buf + n, sz - n, "%s#%d", pfx, i);
	if ((size_t)n >= sz)
	    sz = n;
	pfx = sep;
    }

    CANT_HAPPEN((size_t)n >= sz && buf);
    return n;
}
