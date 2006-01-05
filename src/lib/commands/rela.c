/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  rela.c: Show relations with other countries
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "nat.h"
#include "file.h"
#include "xy.h"
#include "commands.h"
#include "optlist.h"

/*
 * report relations with other countries.
 * Optional argument reports relations from the
 * viewpoint of another country
 */
int
rela(void)
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
    pr("\t%s Diplomatic Relations Report\t", cname(as));
    prdate();
    if (opt_SLOW_WAR) {
	pr("\n  Formal Relations         %5s      theirs\n",
	   player->cnum == as ? "yours" : "his");
    } else {
	pr("\n  Formal Relations         %5s     theirs\n",
	   player->cnum == as ? "yours" : "his");
    }
    for (cn = 1; cn < MAXNOC; cn++) {
	if ((np = getnatp(cn)) == 0)
	    break;
	if (cn == as)
	    continue;
	if (np->nat_stat < STAT_SANCT)
	    continue;
	if (opt_HIDDEN) {
	    if (!player->god && !getcontact(natp, cn))
		continue;
	    if (!player->god && !getcontact(getnatp(player->cnum), cn))
		continue;
	}
	pr("%3d) %-20.20s  ", cn, cname(cn));
	if (opt_SLOW_WAR) {
	    pr("%-10s %s\n", relatename(natp, cn), relatename(np, as));
	} else {
	    pr("%-9s %s\n", relatename(natp, cn), relatename(np, as));
	}
    }
    return RET_OK;
}
