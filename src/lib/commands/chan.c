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
 *  chan.c: Change country/representative name
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     
 */

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "nat.h"
#include "file.h"
#include "news.h"
#include "optlist.h"
#include "commands.h"

int
chan(void)
{
    s_char *temp;
    struct natstr *natp;
    register s_char *p;
    natid cn;
    int charge;
    int nonb;
    int btucost;
    s_char buf[1024];
    struct natstr *us;

    if ((p =
	 getstarg(player->argp[1], "country name or representative? ",
		  buf)) == 0)
	return RET_SYN;
    us = getnatp(player->cnum);
    if (us->nat_stat == VIS) {
	pr("Visitor countries can't change their country name or representative.\n");
	return RET_FAIL;
    }
    switch (*p) {
    case 'n':
    case 'c':
	charge = 0;
	btucost = 0;
	if (!player->god && (us->nat_stat & STAT_NORM)) {
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
	if ((p =
	     getstarg(player->argp[2], "New country name -- ", buf)) == 0)
	    return RET_SYN;
	p[sizeof(us->nat_cnam) - 1] = 0;
	for (cn = 0; NULL != (natp = getnatp(cn)); cn++) {
	    if (!strcmp(p, natp->nat_cnam)) {
		pr("Country #%d is already called `%s'!\n", cn, p);
		return RET_FAIL;
	    }
	}
	nonb = 0;
	for (temp = p; *temp != '\0'; temp++) {
	    if (iscntrl(*temp)) {
		pr("No control characters allowed in country names!\n");
		return RET_FAIL;
	    } else if (*temp != ' ')
		nonb = 1;
	}
	if (!nonb) {
	    pr("Must have a non-blank name!\n");
	    return RET_FAIL;
	}
	player->dolcost += charge;
	player->btused += btucost;
	strcpy(us->nat_cnam, p);
	putnat(us);
	nreport(player->cnum, N_NAME_CHNG, 0, 1);
	break;
    case 'p':
    case 'r':
	pr("(note: these are stored in plain text.)\n");
	if ((p = getstarg(player->argp[2],
			  "New representative name -- ", buf)) == 0)
	    return RET_SYN;
	p[8] = 0;
	strcpy(us->nat_pnam, p);
	putnat(us);
	break;
    default:
	pr("Only \"country\" or \"representative\" can change.\n");
	return RET_SYN;
    }
    return RET_OK;
}
