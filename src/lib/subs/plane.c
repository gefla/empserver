/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plane.c: Plane post-read and pre-write data massage
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "plane.h"
#include "ship.h"
#include "land.h"
#include "file.h"
#include "prototypes.h"
#include "optlist.h"

int
pln_postread(int n, void *ptr)
{
    struct plnstr *pp = ptr;
    struct shpstr theship;
    struct lndstr theland;

    if (pp->pln_uid != n) {
	logerror("pln_postread: Error - %d != %d, zeroing.\n", pp->pln_uid,
		 n);
	memset(pp, 0, sizeof(struct plnstr));
    }

    if (pp->pln_ship >= 0 && pp->pln_own && pp->pln_effic >= PLANE_MINEFF) {
	if (getship(pp->pln_ship, &theship) &&
	    (theship.shp_effic >= SHIP_MINEFF)) {
	    /* wooof!  Carriers are a pain */
	    if (pp->pln_mission) {
		/*
		 *  If the plane is on a mission centered
		 *  on it's loc, the op-area travels with
		 *  the plane.
		 */
		if ((pp->pln_opx == pp->pln_x) &&
		    (pp->pln_opy == pp->pln_y)) {
		    pp->pln_opx = theship.shp_x;
		    pp->pln_opy = theship.shp_y;
		}
	    }
	    if (pp->pln_x != theship.shp_x || pp->pln_y != theship.shp_y)
		time(&pp->pln_timestamp);
	    pp->pln_x = theship.shp_x;
	    pp->pln_y = theship.shp_y;
	}
    }
    if (pp->pln_land >= 0 && pp->pln_own && pp->pln_effic >= PLANE_MINEFF) {
	if (getland(pp->pln_land, &theland) &&
	    (theland.lnd_effic >= LAND_MINEFF)) {
	    /* wooof!  Units are a pain, too */
	    if (pp->pln_mission) {
		/*
		 *  If the plane is on a mission centered
		 *  on it's loc, the op-area travels with
		 *  the plane.
		 */
		if ((pp->pln_opx == pp->pln_x) &&
		    (pp->pln_opy == pp->pln_y)) {
		    pp->pln_opx = theland.lnd_x;
		    pp->pln_opy = theland.lnd_y;
		}
	    }
	    if (pp->pln_x != theland.lnd_x || pp->pln_y != theland.lnd_y)
		time(&pp->pln_timestamp);
	    pp->pln_x = theland.lnd_x;
	    pp->pln_y = theland.lnd_y;
	}
    }
    player->owner = (player->god || pp->pln_own == player->cnum);
    if (opt_MOB_ACCESS)
	pln_do_upd_mob(pp);
    return 1;
}

int
pln_prewrite(int n, void *ptr)
{
    struct plnstr *pp = ptr;
    struct plnstr plane;
    struct nukstr *np;
    int i;

    if (pp->pln_effic < PLANE_MINEFF) {
	if (pp->pln_own)
	    makelost(EF_PLANE, pp->pln_own, pp->pln_uid,
		     pp->pln_x, pp->pln_y);
	pp->pln_own = 0;
	pp->pln_effic = 0;
	for (i = 0; NULL != (np = getnukep(i)); i++) {
	    if (np->nuk_own && np->nuk_plane == n) {
		mpr(np->nuk_own, "%s lost!\n", prnuke(np));
		makelost(EF_NUKE, np->nuk_own, np->nuk_uid,
			 np->nuk_x, np->nuk_y);
		np->nuk_own = 0;
		np->nuk_effic = 0;
		np->nuk_plane = -1;
		putnuke(np->nuk_uid, np);
	    }
	}
    }
    pp->ef_type = EF_PLANE;
    pp->pln_uid = n;

    time(&pp->pln_timestamp);

    getplane(n, &plane);

    return 1;
}

void
pln_init(int n, void *ptr)
{
    struct plnstr *pp = ptr;

    pp->ef_type = EF_PLANE;
    pp->pln_uid = n;
    pp->pln_own = 0;
}

char *
prplane(struct plnstr *pp)
{
    return prbuf("%s #%d", plchr[(int)pp->pln_type].pl_name, pp->pln_uid);
}
