/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mora.c: Edit morale of a land unit
 *
 *  Known contributors to this file:
 *     Jeff Bailey
 */

#include <config.h>

#include "commands.h"
#include "land.h"
#include "optlist.h"

int
morale(void)
{
    struct nstr_item np;
    struct lndstr land;
    struct natstr *natp;
    int i, min;
    char *p;
    char mess[128];
    char buf[1024];

    if (!snxtitem(&np, EF_LAND, player->argp[1], NULL))
	return RET_SYN;
    while (nxtitem(&np, &land)) {
	if (!player->owner || land.lnd_own == 0)
	    continue;
	natp = getnatp(land.lnd_own);
	min = morale_base - (int)natp->nat_level[NAT_HLEV];
	sprintf(mess, "New retreat percentage for %s (min %d%%)? ",
		prland(&land), min);
	p = getstarg(player->argp[2], mess, buf);
	if (!p)
	    return RET_FAIL;
	if (!check_land_ok(&land))
	    continue;
	if ((i = atoi(p)) < 0)
	    continue;
	land.lnd_retreat = ((i < min) ? min : i);
	if (land.lnd_retreat > 100)
	    land.lnd_retreat = 100;
	pr("Unit %d retreat percentage changed to %d\n",
	   land.lnd_uid, land.lnd_retreat);
	putland(land.lnd_uid, &land);
    }

    return RET_OK;
}
