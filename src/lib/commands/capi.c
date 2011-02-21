/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  capi.c: Move your capital
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 2000
 */

#include <config.h>

#include "commands.h"

int
capi(void)
{
    struct sctstr sect;
    struct natstr *np;
    struct nstr_sect nstr;
    int found;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    np = getnatp(player->cnum);
    found = 0;
    while (!found && nxtsct(&nstr, &sect)) {
	if (player->owner && (sect.sct_type == SCT_CAPIT ||
			      sect.sct_type == SCT_MOUNT))
	    found++;
    }
    if (!found)
	return RET_FAIL;
    if (!(np->nat_flags & NF_SACKED) &&
	sect.sct_x == np->nat_xcap && sect.sct_y == np->nat_ycap) {
	pr("%s is already your capital.\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	return RET_FAIL;
    }
    np->nat_xcap = sect.sct_x;
    np->nat_ycap = sect.sct_y;
    /* They have set a capital, so it wasn't sacked last. */
    np->nat_flags &= ~NF_SACKED;
    putnat(np);
    pr("Capital now at %s.\n", xyas(sect.sct_x, sect.sct_y, player->cnum));
    return 0;
}
