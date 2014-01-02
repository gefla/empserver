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
 *  add.c: Add a new country to the game
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     Markus Armbruster, 2004-2013
 */

#include <config.h>

#include "commands.h"

int
add(void)
{
    struct natstr *natp;
    int i;
    char cntryname[sizeof(natp->nat_cnam)];
    char pname[sizeof(natp->nat_pnam)];
    natid freecn;
    char prompt[128];
    char buf[1024];
    char *p;
    int stat;

    for (freecn = 0; NULL != (natp = getnatp(freecn)); freecn++) {
	if (natp->nat_stat == STAT_UNUSED)
	    break;
    }
    if (freecn < MAXNOC)
	sprintf(prompt, "New country number? (%d is unused) ", freecn);
    else
	strcpy(prompt, "New country number? (they all seem to be used) ");
    p = getstarg(player->argp[1], prompt, buf);
    if (!p || !*p)
	return RET_SYN;
    i = atoi(p);
    if (i == 0) {
	pr("Not allowed to add country #0\n");
	return RET_FAIL;
    }
    natp = getnatp(i);
    if (!natp) {
	pr("Can't add country #%d\n", i);
	return RET_FAIL;
    }
    p = getstarg(player->argp[2], "Country name? ", buf);
    if (!p)
	return RET_SYN;
    if (!check_nat_name(p, natp->nat_cnum))
	return RET_FAIL;
    strcpy(cntryname, p);
    p = getstarg(player->argp[3], "Representative? ", buf);
    if (!p || !*p)
	return RET_SYN;
    if (strlen(p) >= sizeof(pname)) {
	pr("Representative too long\n");
	return RET_FAIL;
    }
    strcpy(pname, p);
    p = getstarg(player->argp[4],
		 "Status? (visitor, player, god, delete) ", buf);
    if (!p || !*p)
	return RET_SYN;
    switch (*p) {
    case 'v':
	stat = STAT_VIS;
	break;
    case 'n':
	pr("Argument 'new' is deprecated and will go away in a future release.\n"
	   "Use 'player' instead.\n");
	/* fall through */
    case 'p':
	stat = STAT_NEW;
	break;
    case 'g':
	stat = STAT_GOD;
	break;
    case 'd':
	stat = STAT_UNUSED;
	break;
    default:
	pr("Illegal status\n");
	return RET_SYN;
    }
    if (stat == STAT_UNUSED
	? natp->nat_stat >= STAT_SANCT : natp->nat_stat != STAT_UNUSED) {
	pr("Country %s status is %s.\n"
	   "%s it in this state is normally a bad idea.",
	   prnat(natp), symbol_by_value(natp->nat_stat, nation_status),
	   stat == STAT_UNUSED ? "Deleting" : "Replacing");
	if (!askyn("Are you sure? "))
	    return RET_FAIL;
    }
    if (getplayer(natp->nat_cnum)) {
	pr("%s is logged in!\n", prnat(natp));
	return RET_FAIL;
    }
    nat_reset(natp, natp->nat_cnum, cntryname, pname, stat);
    putnat(natp);
    return 0;
}
