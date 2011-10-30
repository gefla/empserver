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
 *  chan.c: Change country/representative name
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#include <config.h>

#include "commands.h"
#include "news.h"
#include "optlist.h"

int
chan(void)
{
    char *p;
    int charge;
    int btucost;
    char buf[1024];
    struct natstr *us;

    p = getstarg(player->argp[1], "country name or representative? ", buf);
    if (!p)
	return RET_SYN;
    us = getnatp(player->cnum);
    if (us->nat_stat == STAT_VIS) {
	pr("Visitor countries can't change their country name or representative.\n");
	return RET_FAIL;
    }
    switch (*p) {
    case 'n':
    case 'c':
	charge = 0;
	btucost = 0;
	if (us->nat_stat == STAT_ACTIVE) {
	    if (opt_BLITZ == 0) {
		if (us->nat_btu < 254) {
		    pr("You need 254 btus to change your country name!\n");
		    return RET_FAIL;
		}
		pr("This command costs 254 BTU's and 10%% of your money.\n");
		if (!confirm("Are you sure you want to do this? "))
		    return RET_FAIL;
		btucost = 254;
		if (us->nat_money <= 0)
		    charge = 0;
		else
		    charge = us->nat_money / 10;
	    }
	}
	p = getstarg(player->argp[2], "New country name -- ", buf);
	if (!p)
	    return RET_SYN;
	if (!check_nat_name(p, player->cnum))
	    return RET_FAIL;
	player->dolcost += charge;
	player->btused += btucost;
	strcpy(us->nat_cnam, p);
	putnat(us);
	nreport(player->cnum, N_NAME_CHNG, 0, 1);
	break;
    case 'p':
    case 'r':
	pr("(note: these are stored in plain text.)\n");
	p = getstarg(player->argp[2], "New representative name -- ", buf);
	if (!p)
	    return RET_SYN;
	p[sizeof(us->nat_pnam) - 1] = 0;
	strcpy(us->nat_pnam, p);
	putnat(us);
	break;
    default:
	pr("Only \"country\" or \"representative\" can change.\n");
	return RET_SYN;
    }
    return RET_OK;
}
