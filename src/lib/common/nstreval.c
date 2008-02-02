/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nstreval.c: evaluate compiled values
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1997
 *     Markus Armbruster, 2004-2006
 */

#include <config.h>

#include <limits.h>
#include "file.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"


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
    struct natstr *natp;

    switch (val->val_cat) {
    default:
	CANT_REACH();
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
	case NSC_HIDDEN:
	    val->val_as.lng = -1;
	    if (CANT_HAPPEN(((struct natstr *)ptr)->ef_type != EF_NATION))
		break;
	    natp = getnatp(cnum);
	    if (!opt_HIDDEN
		|| natp->nat_stat == STAT_GOD
		|| (getcontact(natp, idx) && getcontact(ptr, idx)))
		val->val_as.lng = ((unsigned char *)memb_ptr)[idx];
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
	default:
	    CANT_REACH();
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
	CANT_REACH();		/* FIXME implement */

    if (CANT_HAPPEN(valtype != want && want != NSC_NOTYPE)) {
	valtype = want;
	switch (want) {
	case NSC_LONG: val->val_as.lng = 0; break;
	case NSC_DOUBLE: val->val_as.dbl = 0.0; break;
	case NSC_STRING: val->val_as.str.base = NULL; break;
	default:
	    CANT_REACH();
	}
    }

    val->val_type = valtype;
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
