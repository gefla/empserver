/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  marketup.c: Market updater thread
 *
 *  Known contributors to this file:
 *     Steve McClure, 1996
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include "empthread.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "server.h"

/*ARGSUSED*/
static void
market_update(void *unused)
{
    time_t now;

    player->proc = empth_self();
    player->cnum = 0;
    player->god = 1;

    for (;;) {
	time(&now);
	check_market();
	check_trade();
	now += 300;		/* Every 5 minutes */
	empth_sleep(now);
    }
    /*NOTREACHED*/
}

void
market_init(void)
{
    struct player *dp;

    if (!opt_MARKET)
	return;
    dp = player_new(-1);
    if (!dp)
	exit_nomem();
    if (!empth_create(market_update, 65536, 0, "MarketUpdate", dp))
	exit_nomem();
}
