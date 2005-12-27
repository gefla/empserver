/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  marketup.c: Market updater thread
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1996
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "empthread.h"
#include "file.h"
#include "server.h"
#include "prototypes.h"

/*ARGSUSED*/
static void
check_all_markets(void *unused)
{
    player->proc = empth_self();
    player->cnum = 0;
    player->god = 1;

    check_market();
    check_trade();

    ef_flush(EF_NATION);
    ef_flush(EF_SECTOR);
    ef_flush(EF_PLANE);
    ef_flush(EF_SHIP);
    ef_flush(EF_LAND);
    ef_flush(EF_COMM);
    ef_flush(EF_TRADE);

    player_delete(player);
    empth_exit();
    /*NOTREACHED*/
}

/*ARGSUSED*/
void
market_update(void *unused)
{
    time_t now;
    struct player *dp;

    while (1) {
	time(&now);
/*	logerror("Checking the world markets at %s", ctime(&now));*/
	dp = player_new(-1);
	if (dp) {
	    empth_create(PP_UPDATE, check_all_markets, (50 * 1024), 0,
			 "MarketCheck", "Checks the world markets", dp);
	} else {
	    logerror("can't create dummy player for market update");
	}
	now = now + 300;	/* Every 5 minutes */
	empth_sleep(now);
    }
    /*NOTREACHED*/
}
