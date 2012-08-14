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
 *  list.c: List ships, planes, units at a given x,y
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include <ctype.h>
#include "chance.h"
#include "file.h"
#include "land.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "path.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

int
shipsatxy(coord x, coord y, int wantflags, int nowantflags, int only_count)
{
    int first;
    int ships;
    struct nstr_item ni;
    struct mchrstr *mp;
    struct shpstr ship;

    first = 1;
    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, &ship)) {
	if (player->owner)
	    continue;
	if (ship.shp_effic < SHIP_MINEFF || ship.shp_own == 0)
	    continue;
	mp = &mchr[(int)ship.shp_type];
	if (wantflags) {
	    if ((mp->m_flags & wantflags) == 0)
		continue;
	}
	if (nowantflags) {
	    if (mp->m_flags & nowantflags)
		continue;
	}
	if (!only_count) {
	    if (first) {
		pr(" #          owner           eff       type\n");
		first = 0;
	    }
	    pr("(#%3d) %10.10s  %12.12s  %s\n", ni.cur,
	       cname(ship.shp_own), effadv(ship.shp_effic), prship(&ship));
	}
	ships++;
    }
    return ships;
}

/* This one only shows owned or allied ships */

int
carriersatxy(coord x, coord y, natid own)
{
    int first;
    int ships;
    struct nstr_item ni;
    struct shpstr ship;

    first = 1;
    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, &ship)) {
	if (ship.shp_effic < SHIP_MINEFF || ship.shp_own == 0)
	    continue;
	if (relations_with(ship.shp_own, own) != ALLIED)
	    continue;
	if ((carrier_planes(&ship, 0) & (P_L | P_K)) == 0)
	    continue;
	if (first) {
	    pr(" #          owner           eff       type\n");
	    first = 0;
	}
	pr("(#%3d) %10.10s  %12.12s  %s\n", ni.cur,
	   cname(ship.shp_own), effadv(ship.shp_effic), prship(&ship));
	ships++;
    }
    return ships;
}

int
unitsatxy(coord x, coord y, int wantflags, int nowantflags)
{
    int first;
    int units;
    struct nstr_item ni;
    struct lchrstr *lp;
    struct lndstr land;

    first = 1;
    units = 0;
    snxtitem_xy(&ni, EF_LAND, x, y);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_effic < LAND_MINEFF || land.lnd_own == 0)
	    continue;
	/* Can't bomb units on ships or other units */
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    continue;
	lp = &lchr[(int)land.lnd_type];

	if (wantflags) {
	    if ((lp->l_flags & wantflags) == 0)
		continue;
	}
	if (nowantflags) {
	    if (lp->l_flags & nowantflags)
		continue;
	}

	if (lp->l_flags & L_SPY) {
	    if (!(chance(LND_SPY_DETECT_CHANCE(land.lnd_effic))))
		continue;
	}

	if (first) {
	    pr(" #          owner           eff       type\n");
	    first = 0;
	}
	pr("(#%3d) %10.10s  %12.12s  %s\n", ni.cur,
	   cname(land.lnd_own), effadv(land.lnd_effic), prland(&land));
	units++;
    }
    return units;
}

int
planesatxy(coord x, coord y, int wantflags, int nowantflags)
{
    int first;
    int planes;
    struct plnstr plane;
    struct nstr_item ni;
    struct plchrstr *plp;

    planes = 0;
    first = 1;
    snxtitem_xy(&ni, EF_PLANE, x, y);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_effic < PLANE_MINEFF || plane.pln_own == 0)
	    continue;
	if (plane.pln_ship >= 0 || plane.pln_land >= 0)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	plp = &plchr[(int)plane.pln_type];
	if (first) {
	    pr(" #          owner           eff       type\n");
	    first = 0;
	}
	if (wantflags) {
	    if ((plp->pl_flags & wantflags) == 0)
		continue;
	}
	if (nowantflags) {
	    if (plp->pl_flags & nowantflags)
		continue;
	}
	pr("(#%3d) %10.10s  %12.12s  %s\n", ni.cur,
	   cname(plane.pln_own), effadv(plane.pln_effic), prplane(&plane));
	planes++;
    }
    return planes;
}

int
asw_shipsatxy(coord x, coord y, int wantflags, int nowantflags,
	      struct plnstr *pp, struct shiplist **head)
{
    int first;
    int ships;
    struct nstr_item ni;
    struct mchrstr *mp;
    struct shpstr ship;

    first = 1;
    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, &ship)) {
	if (player->owner)
	    continue;
	if (ship.shp_effic < SHIP_MINEFF || ship.shp_own == 0)
	    continue;
	mp = &mchr[(int)ship.shp_type];
	if (wantflags) {
	    if ((mp->m_flags & wantflags) == 0)
		continue;
	}
	if (nowantflags) {
	    if (mp->m_flags & nowantflags)
		continue;
	}
	if (mp->m_flags & M_SUB) {
	    if (roll(100) > pln_hitchance(pp,
					  shp_hardtarget(&ship), EF_SHIP))
		continue;
	}
	add_shiplist(ship.shp_uid, head);
	if (first) {
	    pr(" #          owner           eff       type\n");
	    first = 0;
	}
	pr("(#%3d) %10.10s  %12.12s  %s\n", ni.cur,
	   cname(ship.shp_own), effadv(ship.shp_effic), prship(&ship));
	ships++;
    }
    return ships;
}

int
adj_units(coord x, coord y, natid own)
{
    int i;
    struct sctstr sect;

    for (i = DIR_FIRST; i <= DIR_LAST; i++) {
	getsect(x + diroff[i][0], y + diroff[i][1], &sect);
	if (has_units(sect.sct_x, sect.sct_y, own, NULL))
	    return 1;
    }
    return 0;
}

int
has_units(coord x, coord y, natid cn, struct lndstr *lp)
{
    int n;
    struct lndstr land;

    for (n = 0; ef_read(EF_LAND, n, &land); n++) {
	if (land.lnd_x != x || land.lnd_y != y)
	    continue;
	if (lp) {
	    /* Check this unit.  If it is this one, we don't want
	       it included in the count. */
	    if (lp->lnd_uid == land.lnd_uid)
		continue;
	}
	if (land.lnd_own == cn)
	    return 1;
    }

    return 0;
}

/*
 * is p a list of ships/planes/units?
 */

int
islist(char *p)
{
    for (; *p; p++) {
	if (!isdigit(*p) && *p != '/')
	    return 0;
    }
    return 1;
}
