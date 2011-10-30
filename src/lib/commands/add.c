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
 *  add.c: Add a new country to the game
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     Markus Armbruster, 2004-2011
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
    natid coun;
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
    if (i >= MAXNOC) {
	pr("Max # countries is %d\n", MAXNOC);
	return RET_FAIL;
    }
    coun = i;
    if (coun == 0) {
	pr("Not allowed to add country #0\n");
	return RET_FAIL;
    }
    natp = getnatp(coun);
    p = getstarg(player->argp[2], "Country name? ", buf);
    if (!p)
	return RET_SYN;
    if (!check_nat_name(p, coun))
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
		 "Status? (visitor, new, active, god, delete) ", buf);
    if (!p || !*p)
	return RET_SYN;
    switch (*p) {
    case 'v':
	stat = STAT_VIS;
	break;
    case 'n':
	stat = STAT_NEW;
	break;
    case 'a':
	stat = STAT_ACTIVE;
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
    strcpy(natp->nat_cnam, cntryname);
    strcpy(natp->nat_pnam, pname);
    if (stat == STAT_NEW || stat == STAT_VIS)
	nat_reset(natp, stat, 0, 0);
    else {
	natp->nat_stat = stat;
	pr("No special initializations done...\n");
    }
    putnat(natp);
    return 0;
}
