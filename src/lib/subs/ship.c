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
 *  ship.c: Ship pre-write data massage
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 */

#include "misc.h"
#include "player.h"
#include "file.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "nsc.h"
#include "prototypes.h"
#include "optlist.h"

/*ARGSUSED*/
int
shp_postread(int n, s_char *ptr)
{
    struct shpstr *sp = (struct shpstr *)ptr;

    if (sp->shp_uid != n) {
	logerror("shp_postread: Error - %d != %d, zeroing.\n", sp->shp_uid,
		 n);
	memset(sp, 0, sizeof(struct shpstr));
    }

    if (opt_MOB_ACCESS)
	shp_do_upd_mob(sp);
    player->owner = (player->god || sp->shp_own == player->cnum);
    return 1;
}

int
shp_prewrite(int n, s_char *ptr)
{
    struct shpstr *sp = (struct shpstr *)ptr;
    struct shpstr ship;
    struct lndstr *lp;
    struct plnstr *pp;
    int i;

    sp->ef_type = EF_SHIP;
    sp->shp_uid = n;

    time(&sp->shp_timestamp);

    if (sp->shp_own != 0 && sp->shp_effic < SHIP_MINEFF) {
	mpr(sp->shp_own, "\t%s sunk!\n", prship(sp));
	makelost(EF_SHIP, sp->shp_own, sp->shp_uid, sp->shp_x, sp->shp_y);
	sp->shp_own = 0;

	getship(n, &ship);

	for (i = 0; NULL != (lp = getlandp(i)); i++) {
	    if (lp->lnd_own && lp->lnd_ship == n) {
		mpr(lp->lnd_own, "%s sunk!\n", prland(lp));
		makelost(EF_LAND, lp->lnd_own, lp->lnd_uid, lp->lnd_x,
			 lp->lnd_y);
		lp->lnd_own = 0;
		lp->lnd_effic = 0;
		lp->lnd_ship = -1;
		lp->lnd_land = -1;
		putland(lp->lnd_uid, lp);
	    }
	}
	for (i = 0; NULL != (pp = getplanep(i)); i++) {
	    if (pp->pln_own && pp->pln_ship == n) {
		mpr(pp->pln_own, "%s sunk!\n", prplane(pp));
		makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x,
			 pp->pln_y);
		pp->pln_own = 0;
		pp->pln_effic = 0;
		pp->pln_ship = -1;
		pp->pln_land = -1;
		putplane(pp->pln_uid, pp);
	    }
	}
    } else {
	item_prewrite(sp->shp_item);
	getship(n, &ship);
    }

    return 1;
}

void
shp_init(int n, s_char *ptr)
{
    struct shpstr *sp = (struct shpstr *)ptr;

    sp->ef_type = EF_SHIP;
    sp->shp_uid = n;
    sp->shp_own = 0;
}

s_char *
prship(struct shpstr *sp)
{
    return prbuf("%s %s(#%d)",
		 mchr[(int)sp->shp_type].m_name, sp->shp_name,
		 sp->shp_uid);
}
