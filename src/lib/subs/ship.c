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
 *  ship.c: Ship pre-write data massage
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 */

#include <config.h>

#include "file.h"
#include "land.h"
#include "lost.h"
#include "misc.h"
#include "nsc.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "ship.h"

void
shp_postread(int n, void *ptr)
{
    struct shpstr *sp = ptr;

    if (sp->shp_uid != n) {
	logerror("shp_postread: Error - %d != %d, zeroing.\n",
		 sp->shp_uid, n);
	memset(sp, 0, sizeof(struct shpstr));
    }

    if (opt_MOB_ACCESS)
	shp_do_upd_mob(sp);
    player->owner = (player->god || sp->shp_own == player->cnum);
}

void
shp_prewrite(int n, void *old, void *new)
{
    struct shpstr *oldsp = old;
    struct shpstr *sp = new;
    natid own = sp->shp_own;
    struct lndstr *lp;
    struct plnstr *pp;
    int i;

    if (own && sp->shp_effic < SHIP_MINEFF) {
	mpr(own, "\t%s sunk!\n", prship(sp));
	own = 0;

	for (i = 0; NULL != (lp = getlandp(i)); i++) {
	    if (lp->lnd_own && lp->lnd_ship == n) {
		mpr(lp->lnd_own, "%s sunk!\n", prland(lp));
		lp->lnd_effic = 0;
		lp->lnd_ship = -1;
		lp->lnd_land = -1;
		putland(lp->lnd_uid, lp);
	    }
	}
	for (i = 0; NULL != (pp = getplanep(i)); i++) {
	    if (pp->pln_own && pp->pln_ship == n) {
		mpr(pp->pln_own, "%s sunk!\n", prplane(pp));
		pp->pln_effic = 0;
		pp->pln_ship = -1;
		pp->pln_land = -1;
		putplane(pp->pln_uid, pp);
	    }
	}
    } else {
	item_prewrite(sp->shp_item);
    }

    /* We've avoided assigning to sp->shp_own, in case oldsp == sp */
    if (oldsp->shp_own != own)
	lost_and_found(EF_SHIP, oldsp->shp_own, own,
		       sp->shp_uid, sp->shp_x, sp->shp_y);

    sp->shp_own = own;
}

char *
prship(struct shpstr *sp)
{
    return prbuf("%s %s(#%d)",
		 mchr[(int)sp->shp_type].m_name, sp->shp_name,
		 sp->shp_uid);
}
