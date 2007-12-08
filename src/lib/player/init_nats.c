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
 *  init_nats.c: Initialize country and nation file stuff
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 2000
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include "file.h"
#include "game.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"

int
init_nats(void)
{
    static int nstat[] = {
	/* must match nat_status */
	0, VIS, VIS, SANCT, NORM, GOD
    };
    struct natstr *np;

    np = getnatp(player->cnum);
    if (CANT_HAPPEN(!np))
	return -1;
    player->nstat = nstat[np->nat_stat];
    player->god = np->nat_stat == STAT_GOD;
    player->map = ef_ptr(EF_MAP, player->cnum);
    player->bmap = ef_ptr(EF_BMAP, player->cnum);
    if (opt_HIDDEN) {
	putcontact(np, player->cnum, FOUND_SPY);
    }
    if (np->nat_money < 0)
	player->broke = 1;
    else {
	player->nstat |= MONEY;
	player->broke = 0;
    }
    if (grant_btus(np, game_tick_to_now(&np->nat_access)))
	player->nstat |= CAP;
    putnat(np);
    return 0;
}
