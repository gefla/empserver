/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  rela.c: Show relations with other countries
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"

/*
 * report relations with other countries.
 * Optional argument reports relations from the
 * viewpoint of another country
 */
int
c_relations(void)
{
    struct natstr *np;
    natid cn;
    natid as;
    int n;

    if (!player->argp[1])
	as = player->cnum;
    else {
	if ((n = natarg(player->argp[1], NULL)) < 0)
	    return RET_SYN;
	as = (natid)n;
    }
    pr("\t%s Diplomatic Relations Report\t", cname(as));
    prdate();
    pr("\n  Formal Relations         %5s      theirs\n",
       player->cnum == as ? "yours" : "his");
    for (cn = 1; cn < MAXNOC; cn++) {
	if (!(np = getnatp(cn)))
	    break;
	if (cn == as)
	    continue;
	if (np->nat_stat < STAT_SANCT)
	    continue;
	if (opt_HIDDEN) {
	    if (!player->god && !in_contact(as, cn))
		continue;
	    if (!player->god && !in_contact(player->cnum, cn))
		continue;
	}
	pr("%3d) %-20.20s  ", cn, cname(cn));
	pr("%-10s %s\n",
	   relations_string(relations_with(as, cn)),
	   relations_string(relations_with(cn, as)));
    }
    return RET_OK;
}
