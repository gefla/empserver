/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  acce.c: Report rejection status of telegrams/treaties/annos/loans
 * 
 *  Known contributors to this file:
 *     
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "nat.h"
#include "sect.h"
#include "file.h"
#include "xy.h"
#include "commands.h"
#include "optlist.h"

/*
 * report rejection status of telegrams and treaties.
 * Optional argument reports staus from the
 * viewpoint of another country
 */
int
acce(void)
{
    struct natstr *natp;
    struct natstr *np;
    natid cn;
    natid as;
    int n;

    if (player->argp[1] == 0)
	as = player->cnum;
    else {
	if ((n = natarg(player->argp[1], "Which country? ")) < 0) {
	    pr("Bad country number\n");
	    return RET_SYN;
	}
	as = (natid)n;
    }
    if ((natp = getnatp(as)) == 0) {
	pr("Bad country number %d\n", player->cnum);
	return RET_SYN;
    }
    pr("\t%s Acceptance Status Report\t", cname(as));
    prdate();
    pr("\n  Acceptance status          %5s                theirs\n",
       player->cnum == as ? "yours" : " his");
    pr("                       tel trty anno loan   tel trty anno loan\n");
    for (cn = 1; cn < MAXNOC; cn++) {
	if ((np = getnatp(cn)) == 0)
	    break;
	if (cn == as)
	    continue;
	if ((np->nat_stat & STAT_NORM) == 0 &&
	    (np->nat_stat & STAT_SANCT) == 0)
	    continue;
	if (opt_HIDDEN) {
	    if (!player->god && !getcontact(getnatp(player->cnum), cn))
		continue;
	}
	pr("%3d) %-14.14s  ", cn, cname(cn));
	pr("%-9s %s\n", rejectname(natp, cn), rejectname(np, as));
    }
    return RET_OK;
}
