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
 *  cnumb.c: Return country number give country name
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "file.h"
#include "match.h"
#include "nat.h"
#include "prototypes.h"

/*
 * Search for a country matching CNTRY, return its number.
 * Return M_NOTFOUND if no such country exists, M_NOTUNIQUE if there
 * are several.
 */
int
cnumb(char *cntry)
{
    char *ncp;
    char *cp;
    struct natstr *natp;
    int res;
    natid cn;

    res = M_NOTFOUND;
    for (cn = 0; cn < MAXNOC; cn++) {
	if ((natp = getnatp(cn)) == 0)
	    break;
	if (natp->nat_stat == STAT_UNUSED)
	    continue;
	ncp = natp->nat_cnam;
	for (cp = cntry; *cp == *ncp; cp++, ncp++) {
	    if (*cp == 0)
		return cn;	/* exact match */
	}
	if (*cp == 0) {
	    /* is a prefix */
	    if (res >= 0)
		return M_NOTUNIQUE;
	    res = cn;
	}
    }
    return res;
}
