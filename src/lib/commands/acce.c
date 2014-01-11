/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  acce.c: Report rejection status of telegrams/annos/loans
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "commands.h"

/*
 * report rejection status
 * Optional argument reports staus from the
 * viewpoint of another country
 */
int
acce(void)
{
    static char *rejects[] = {
	/* must follow reject flags defined in nat.h */
	"  YES  YES  YES",
	"  NO   YES  YES",
	"  YES  NO   YES",
	"  NO   NO   YES",
	"  YES  YES  NO ",
	"  NO   YES  NO ",
	"  YES  NO   NO ",
	"  NO   NO   NO ",
    };
    struct natstr *natp;
    struct natstr *np;
    natid cn;
    natid as;

    if (!player->argp[1]) {
	natp = getnatp(player->cnum);
    } else {
	if (!(natp = natargp(player->argp[1], NULL)))
	    return RET_SYN;
    }
    as = natp->nat_cnum;
    pr("\t%s Acceptance Status Report\t", cname(as));
    prdate();
    pr("\n  Acceptance status       %s           theirs\n",
       player->cnum == as ? "yours" : " his ");
    pr("                       tel anno loan   tel anno loan\n");
    for (cn = 0; cn < MAXNOC; cn++) {
	if (cn == as)
	    continue;
	if (!(np = getnatp(cn)))
	    break;
	if (np->nat_stat == STAT_UNUSED)
	    continue;
	pr("%3d) %-14.14s  ", cn, cname(cn));
	pr("%s %s\n",
	   rejects[getrejects(cn, natp)],
	   rejects[getrejects(as, np)]);
    }
    return RET_OK;
}
