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
 *  nstr.c: Compile and execute the item selections on sectors
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
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
#include "common.h"
#include "gen.h"

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

	for (nsc=conds; --ncond >= 0; nsc++) {
	        oper = nsc->oper;
		if (oper > 65535) {
		    oper = oper - 65535;
		    rhs = nsc->fld2;
		} else
		    rhs = decode(player->cnum, nsc->fld2, ptr, type);

		if (oper > 255) {
		    oper = oper - 255;
		    lhs = nsc->fld1;
		} else
		    lhs = decode(player->cnum, nsc->fld1, ptr, type);

		op = oper;
		if ((op == '<' && lhs >= rhs) ||
		    (op == '=' && lhs != rhs) ||
		    (op == '>' && lhs <= rhs) ||
		    (op == '#' && lhs == rhs))
			return 0;
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
	if (val>=0x8000) val -= 0x10000;
	
	nsc_code = code & NSC_CMASK;
	if (nsc_code == NSC_VAR) {
		val = getvar(val, addr, type);
	} else if (nsc_code == NSC_OFF) {
		/*
		 * add offset to value
		 */
		addr = (s_char *)addr + val;
		switch (code_type) {
		case NSC_TIME:
		        val = *((time_t *)addr);
			break;
		case NSC_CHAR:
			val = *((s_char *) addr);
			break;
		case NSC_UCHAR:
			val = (int) *((unsigned char *) addr);
			break;
		case NSC_SHORT:
			val = *((short *) addr);
			break;
		case NSC_USHORT:
			val = *((u_short *) addr);
			break;
		case NSC_INT:
			val = *((int *) addr);
			break;
		case NSC_LONG:
			val = *((long *) addr);
			break;
		case NSC_XCOORD:
			val = *((short *) addr);
			np = getnatp(cnum);
			val = xrel(np, val);
			break;
		case NSC_YCOORD:
			val = *((short *) addr);
			np = getnatp(cnum);
			val = yrel(np, val);
			break;
		default:
			logerror("bad type in decode: %x!\n",
				code & NSC_TMASK);
			val = 0;
			break;
		}
	}
	if (code & NSC_ROUND)
		val = roundintby(val, 10);
	return val;
}

s_char *
decodep(long int code, void *addr)
{
	addr = (char *)addr + ((code & ~NSC_MASK) & 0xffff);

	if ((code & NSC_TMASK) == NSC_CHARP)
		return *(s_char **)addr?*((s_char **)addr):(s_char *)"";
	return addr;
}
