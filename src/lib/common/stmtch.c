/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  stmtch.c: Matching operations on structures and commands
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2008
 */

#include <config.h>

#include "match.h"

/*
 * Find element named NEEDLE in array HAYSTACK[], return its index.
 * Return M_NOTFOUND if there are no matches, M_NOTUNIQUE if there are
 * several.
 * Each array element has a pointer to its name stored at offset OFFS.
 * Search stops when this name is a null pointer or empty.
 * NEEDLE is compared to element names with mineq(NEEDLE, NAME).
 * ELT_SIZE gives the size of an array element.
 */
int
stmtch(char *needle, void *haystack, ptrdiff_t offs, size_t elt_size)
{
#define ELT_NAME(i) (*(char **)((char *)haystack + (i)*elt_size + offs))
    int i, res;

    res = M_NOTFOUND;
    for (i = 0; ELT_NAME(i) && ELT_NAME(i)[0] != 0; i++) {
	switch (mineq(needle, ELT_NAME(i))) {
	case ME_MISMATCH:
	    break;
	case ME_PARTIAL:
	    if (res == M_NOTFOUND)
		res = i;
	    else
		res = M_NOTUNIQUE;
	    break;
	case ME_EXACT:
	    return i;
	}
    }
    return res;
#undef ELT_NAME
}

/*
 * Compare A with B.
 * Return ME_EXACT if they are the same, or A is a prefix of B
 * followed by a space in B.
 * Return ME_PARTIAL if A is a prefix of B not followed by a space.
 * Else return ME_MISMATCH.
 */
int
mineq(char *a, char *b)
{
    int i;

    /* find common prefix: */
    for (i = 0; a[i] != 0 && a[i] == b[i]; i++) ;

    if (a[i] != 0) return ME_MISMATCH;
    if (b[i] == 0 || b[i] == ' ') return ME_EXACT;
    return ME_PARTIAL;
}
