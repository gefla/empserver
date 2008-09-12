/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  land.c: Land unit post-read and pre-write
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1996
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
    struct lndstr *llp = ptr;

    if (llp->lnd_uid != n) {
	logerror("lnd_postread: Error - %d != %d, zeroing.\n",
		 llp->lnd_uid, n);
	memset(llp, 0, sizeof(struct lndstr));
    }

    if (opt_MOB_ACCESS)
	lnd_do_upd_mob(llp);
    player->owner = (player->god || llp->lnd_own == player->cnum);
}

void
lnd_prewrite(int n, void *old, void *new)
{
    struct lndstr *oldlp = old;
    struct lndstr *llp = new;
    natid own = llp->lnd_own;

    if (llp->lnd_own && llp->lnd_effic < LAND_MINEFF) {
	own = 0;
	llp->lnd_ship = llp->lnd_land = -1;
    } else {
	item_prewrite(llp->lnd_item);
    }

    if (CANT_HAPPEN(llp->lnd_ship >= 0 && llp->lnd_land >= 0))
	llp->lnd_land = -1;
    if (oldlp->lnd_ship != llp->lnd_ship)
	lnd_carrier_change(llp, EF_SHIP, oldlp->lnd_ship, llp->lnd_ship);
    if (oldlp->lnd_land != llp->lnd_land)
	lnd_carrier_change(llp, EF_LAND, oldlp->lnd_land, llp->lnd_land);

    /* We've avoided assigning to llp->lnd_own, in case oldsp == sp */
    if (oldlp->lnd_own != own)
	lost_and_found(EF_LAND, oldlp->lnd_own, own,
		       llp->lnd_uid, llp->lnd_x, llp->lnd_y);

    llp->lnd_own = own;
    if (!own || llp->lnd_x != oldlp->lnd_x || llp->lnd_y != oldlp->lnd_y)
	unit_update_cargo((struct empobj *)llp);
}

void
lnd_oninit(void *ptr)
{
    struct lndstr *lp = ptr;

    lp->lnd_ship = lp->lnd_land = -1;
}

char *
prland(struct lndstr *lp)
{
    return prbuf("%s #%d", lchr[(int)lp->lnd_type].l_name, lp->lnd_uid);
}
