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
 */

#include <ctype.h>
#include "struct.h"
#include "misc.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "match.h"
#include "file.h"
#include "player.h"
#include "prototypes.h"

static int legal_val(s_char *str, int val);

/*
 * Compiles and adds "str" to the list of conditionals.
 * type is the EF typename of the item type we're selecting.
 * returns amount of "str" used by nstr_comp (i.e. how far
 * the pointer was advanced).  The last is only meaningful
 * if several conditionals are expected in one string.
 */
s_char *
nstr_comp(struct nscstr *np, int *size, int type, s_char *str)
{
    register s_char *bp;
    register s_char *cp;
    register int c;
    s_char ident[80];
    s_char arg[255];
    int val;

    strncpy(arg, str, sizeof(arg) - 1);
    arg[sizeof(arg) - 1] = 0;
    cp = arg;
    bp = ident;
    while ((c = *cp++) && bp < &ident[sizeof(ident) - 1]) {
	if (c == '<' || c == '=' || c == '>' || c == '#')
	    break;
	*bp++ = c;
    }
    *bp = 0;
    if (c == 0) {
	pr("'%s'? -- meaningless condition?\n", arg);
	return 0;
    }
    np[*size].oper = c & NSC_OPMASK;
    if ((val = encode(ident, &np[*size].fld1, type)) < 0)
	return 0;
    if (val == 2)
	np[*size].oper |= NSC_ISNUM1;
    bp = ident;
    while ((c = *cp++) && bp < &ident[sizeof(ident) - 1]) {
	if (c == '&')
	    break;
	*bp++ = c;
    }
    *bp = 0;
    if ((val = encode(ident, &np[*size].fld2, type)) < 0)
	return 0;
    if (val == 2)
	np[*size].oper |= NSC_ISNUM2;
    if (c == 0)
	cp--;
    (*size)++;
    return str + (cp - arg);
}

/*
 * return true if the conditions on this item
 * are all true.
 */
int
nstr_exec(struct nscstr *conds, register int ncond, void *ptr, int type)
{
    register struct nscstr *nsc;
    register int op;
    register int lhs;
    register int rhs;
    register int oper;

    for (nsc = conds; --ncond >= 0; nsc++) {
	oper = nsc->oper;
	if (oper & NSC_ISNUM2) {
	    rhs = nsc->fld2;
	} else
	    rhs = decode(player->cnum, nsc->fld2, ptr, type);

	if (oper & NSC_ISNUM1) {
	    lhs = nsc->fld1;
	} else
	    lhs = decode(player->cnum, nsc->fld1, ptr, type);

	op = oper & NSC_OPMASK;
	if ((op == '<' && lhs >= rhs)
	    || (op == '=' && lhs != rhs)
	    || (op == '>' && lhs <= rhs)
	    || (op == '#' && lhs == rhs))
	    return 0;
    }
    return 1;
}

int
encode(register s_char *str, long int *val, int type)
{
    register int i;
    struct castr *cap;

    if (str == 0) {
	*val = 0;
	return 0;
    }
    if (isdigit(*str) || ((*str == '-') && isdigit(str[1]))) {
	*val = atoi(str);
	return 2;
    }
    /*
     * FIXME This accepts the first match found, even if there are
     * more matches in other tables, i.e. it quietly accepts ambiguous
     * matches (matches in multiple tables), and fails to prefer an
     * exact match to partial match in an earlier table.
     */
    if ((i = typematch(str, type)) >= 0) {
	*val = i;
	return 1;
    }
    if ((cap = ef_cadef(type)) != 0) {
	i = stmtch(str, (caddr_t)cap, fldoff(castr, ca_name),
		   sizeof(struct castr));
	if (i >= 0) {
	    *val = cap[i].ca_code;
	    return legal_val(str, *val);
	}
	if (i == M_NOTUNIQUE) {
	    pr("%s -- ambiguous type selector\n", str);
	    return 0;
	}
    }
    /*
     * Only check for commodity selectors on objects which
     * are allowed to have commodities.
     */
    if (ef_flags(type) & EFF_COM) {
	i = stmtch(str, (caddr_t)var_ca, fldoff(castr, ca_name),
		   sizeof(struct castr));
	if (i >= 0) {
	    *val = var_ca[i].ca_code;
	    return legal_val(str, *val);
	}
	if (i == M_NOTUNIQUE) {
	    pr("%s -- ambiguous commodity selector\n", str);
	    return 0;
	}
    }
    pr("%s -- not a valid selector\n", str);
    return 0;
}

static int
legal_val(s_char *str, int val)
{
    if (val & NSC_DEITY && !player->god) {
	pr("%s -- permission denied\n", str);
	return -1;
    }
    return 1;
}

int
decode(natid cnum, long int code, void *addr, int type)
{
    register int val;
    register int nsc_code;
    struct natstr *np;
    long code_type = (code & NSC_TMASK);

    val = (code & ~NSC_MASK) & 0xffff;

    /* handle negative numbers properly */
    /* this assumes a binary two's complement number representation */
    if (val >= 0x8000)
	val -= 0x10000;

    nsc_code = code & NSC_CMASK;
    if (nsc_code == NSC_VAR) {
	u_short *item = ef_items(type, addr);
	val = item ? item[val] : 0;
    } else if (nsc_code == NSC_OFF) {
	/*
	 * add offset to value
	 */
	addr = (s_char *)addr + val;
	switch (code_type) {
	case NSC_TIME:
	    val = *((time_t *) addr);
	    break;
	case NSC_CHAR:
	    val = *((s_char *)addr);
	    break;
	case NSC_UCHAR:
	    val = (int)*((unsigned char *)addr);
	    break;
	case NSC_SHORT:
	    val = *((short *)addr);
	    break;
	case NSC_USHORT:
	    val = *((u_short *)addr);
	    break;
	case NSC_INT:
	    val = *((int *)addr);
	    break;
	case NSC_LONG:
	    val = *((long *)addr);
	    break;
	case NSC_XCOORD:
	    val = *((short *)addr);
	    np = getnatp(cnum);
	    val = xrel(np, val);
	    break;
	case NSC_YCOORD:
	    val = *((short *)addr);
	    np = getnatp(cnum);
	    val = yrel(np, val);
	    break;
	default:
	    logerror("bad type in decode: %x!\n", code & NSC_TMASK);
	    val = 0;
	    break;
	}
    }
    return val;
}

s_char *
decodep(long int code, void *addr)
{
    addr = (char *)addr + ((code & ~NSC_MASK) & 0xffff);

    if ((code & NSC_TMASK) == NSC_CHARP)
	return *(s_char **)addr ? *((s_char **)addr) : (s_char *)"";
    return addr;
}
