/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  land.c: Land unit post-read and pre-write
 *
 *  Known contributors to this file:
 *     Steve McClure, 1996
 *     Markus Armbruster, 2004-2011
 */

#include <config.h>

#include "file.h"
#include "land.h"
#include "lost.h"
#include "misc.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "unit.h"

void
lnd_postread(int n, void *ptr)
{
    struct lndstr *lp = ptr;

    if (lp->lnd_uid != n) {
	logerror("lnd_postread: Error - %d != %d, zeroing.\n",
		 lp->lnd_uid, n);
	memset(lp, 0, sizeof(struct lndstr));
    }

    if (opt_MOB_ACCESS)
	lnd_do_upd_mob(lp);
    player->owner = (player->god || lp->lnd_own == player->cnum);
}

void
lnd_prewrite(int n, void *old, void *new)
{
    struct lndstr *oldlp = old;
    struct lndstr *lp = new;
    natid own = lp->lnd_effic < LAND_MINEFF ? 0 : lp->lnd_own;
    int ship = lp->lnd_ship;
    int land = lp->lnd_land;

    /* Be careful with writing to *lp, in case oldlp == lp */

    if (!own) {
	lp->lnd_effic = 0;
	ship = land = -1;
    }
    item_prewrite(lp->lnd_item);

    if (CANT_HAPPEN(ship >= 0 && land >= 0))
	land = -1;
    if (oldlp->lnd_ship != ship)
	lnd_carrier_change(lp, EF_SHIP, oldlp->lnd_ship, ship);
    if (oldlp->lnd_land != land)
	lnd_carrier_change(lp, EF_LAND, oldlp->lnd_land, land);

    if (oldlp->lnd_own != own) {
	lost_and_found(EF_LAND, oldlp->lnd_own, own,
		       lp->lnd_uid, lp->lnd_x, lp->lnd_y);
	CANT_HAPPEN(!oldlp->lnd_own
		    && unit_update_cargo((struct empobj *)oldlp));
    }

    lp->lnd_own = own;
    lp->lnd_ship = ship;
    lp->lnd_land = land;
    if (!own || lp->lnd_x != oldlp->lnd_x || lp->lnd_y != oldlp->lnd_y)
	unit_update_cargo((struct empobj *)lp);
}

char *
prland(struct lndstr *lp)
{
    return prbuf("%s #%d", lchr[(int)lp->lnd_type].l_name, lp->lnd_uid);
}
