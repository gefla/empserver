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
 *  stmtch.c: Matching operations on structures and commands
 * 
 *  Known contributors to this file:
 *     
 */
/*
 * XXX These routines gamble that structures are all longword-aligned.
 * If this is not true, they will BREAK!
 */

#include "misc.h"
#include "match.h"
#include "common.h"

/*
 * find a matching string from a member string pointer
 * in a structure "str".  Pointer is incremented by
 * the (passed) size of the structure.
 */
int
stmtch(register s_char *obj, s_char *base, int off, int size)
{
    register s_char *str;
    register int stat2;
    register int i;
    register int n;

    stat2 = M_NOTFOUND;
    str = base + off;
    for (i = 0; *(s_char **)str; i++, str += size) {
	if ((n = mineq(obj, *(s_char **)str)) == ME_MISMATCH)
	    continue;
	if (n == ME_EXACT)
	    return i;
	if (stat2 != M_NOTFOUND)
	    return M_NOTUNIQUE;
	stat2 = i;
    }
    return stat2;
}

/*
 * do partial match comparison.
 */
int
mineq(register s_char *a, register s_char *b)
{
    do {
	if (*a++ != *b++)
	    return ME_MISMATCH;
    } while (*b != ' ' && *a != 0);
    if (*a == 0 && (*b == ' ' || *b == 0))
	return ME_EXACT;
    return ME_PARTIAL;
}
