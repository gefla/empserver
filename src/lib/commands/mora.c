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
 *  mora.c: Edit morale of a land unit
 * 
 *  Known contributors to this file:
 *     Jeff Bailey
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "land.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "commands.h"
#include "optlist.h"

int
morale(void)
{
    struct nstr_item np;
    struct lndstr land;
    struct natstr *natp;
    int i, min;
    s_char *p;
    s_char mess[128];
    s_char buf[1024];

    if (!snxtitem(&np, EF_LAND, player->argp[1]))
	return RET_SYN;
    while (!player->aborted && nxtitem(&np, (s_char *)&land)) {
	if (!player->owner || land.lnd_own == 0)
	    continue;
	natp = getnatp(land.lnd_own);
	min = morale_base - (int)natp->nat_level[NAT_HLEV];
	sprintf(mess, "New retreat percentage for %s (min %d%%)? ",
		prland(&land), min);
	p = getstarg(player->argp[2], mess, buf);
	if (!check_land_ok(&land))
	    continue;
	if (player->aborted)
	    continue;
	if (!p || (i = atoi(p)) < 0)
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
