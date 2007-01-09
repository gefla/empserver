/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  real.c: Show or set a realm
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"

static void
list_realm(int curr, struct natstr *natp)
{
    struct realmstr realm;

    getrealm(curr, natp->nat_cnum, &realm);
    pr("Realm #%d is %d:%d,%d:%d\n", curr,
       xrel(natp, realm.r_xl), xrel(natp, realm.r_xh),
       yrel(natp, realm.r_yl), yrel(natp, realm.r_yh));
}

int
real(void)
{
    struct realmstr realm;
    struct natstr *natp;
    int curr;
    int lastr;
    struct range abs;
    char *realmp = player->argp[1];

    natp = getnatp(player->cnum);
    if (!realmp) {
	curr = 0;
	lastr = MAXNOR - 1;
    } else {
	if (*realmp == '#')
	    ++realmp;
	if (*realmp && !isdigit(*realmp))
	    return RET_SYN;
	curr = lastr = atoi(realmp);
	if (curr < 0 || curr >= MAXNOR) {
	    pr("Realm number must be in the range 0:%d\n", MAXNOR - 1);
	    return RET_SYN;
	}
    }
    if (player->argp[2] == 0) {
	while (curr <= lastr) {
	    list_realm(curr, natp);
	    curr++;
	}
    } else {
	if (sarg_type(player->argp[2]) != NS_AREA)
	    return RET_SYN;
	abs.width = 0;
	abs.height = 0;
	if (!sarg_area(player->argp[2], &abs))
	    return RET_SYN;
	getrealm(curr, natp->nat_cnum, &realm);
	realm.r_xl = abs.lx;
	realm.r_xh = abs.hx - 1;
	realm.r_yl = abs.ly;
	realm.r_yh = abs.hy - 1;
	realm.r_timestamp = time(NULL);
	putrealm(&realm);
	list_realm(curr, natp);
    }
    return RET_OK;
}
