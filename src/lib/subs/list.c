/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2003-2016
 */

#include <config.h>

#include <ctype.h>
#include "chance.h"
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

static void
list_ship(struct shpstr *sp, int first)
{
    if (first)
	pr(" #          owner           eff       type\n");
    pr("(#%3d) %10.10s  %12.12s  %s\n", sp->shp_uid,
       cname(sp->shp_own), effadv(sp->shp_effic), prship(sp));
}

int
shipsatxy(coord x, coord y, int wantflags, int nowantflags, int only_count)
{
    int ships;
    struct nstr_item ni;
    struct mchrstr *mp;
    struct shpstr ship;

    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, &ship)) {
	if (player->owner)
	    continue;
	if (!ship.shp_own)
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
	if (!only_count)
	    list_ship(&ship, !ships);
	ships++;
    }
    return ships;
}

/* This one only shows owned or allied ships */

int
carriersatxy(coord x, coord y, natid own)
{
    int ships;
    struct nstr_item ni;
    struct shpstr ship;

    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, &ship)) {
	if (!ship.shp_own)
	    continue;
	if (relations_with(ship.shp_own, own) != ALLIED)
	    continue;
	if ((carrier_planes(&ship, 0) & (P_L | P_K)) == 0)
	    continue;
	list_ship(&ship, !ships);
	ships++;
    }
    return ships;
}

int
unitsatxy(coord x, coord y, int wantflags, int nowantflags)
{
    int units;
    struct nstr_item ni;
    struct lchrstr *lp;
    struct lndstr land;

    units = 0;
    snxtitem_xy(&ni, EF_LAND, x, y);
    while (nxtitem(&ni, &land)) {
	if (!land.lnd_own)
	    continue;
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

	if (!units)
	    pr(" #          owner           eff       type\n");
	pr("(#%3d) %10.10s  %12.12s  %s\n", ni.cur,
	   cname(land.lnd_own), effadv(land.lnd_effic), prland(&land));
	units++;
    }
    return units;
}

int
planesatxy(coord x, coord y, int wantflags, int nowantflags)
{
    int planes;
    struct plnstr plane;
    struct nstr_item ni;
    struct plchrstr *plp;

    planes = 0;
    snxtitem_xy(&ni, EF_PLANE, x, y);
    while (nxtitem(&ni, &plane)) {
	if (!plane.pln_own)
	    continue;
	if (plane.pln_ship >= 0 || plane.pln_land >= 0)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	plp = &plchr[(int)plane.pln_type];
	if (!planes)
	    pr(" #          owner           eff       type\n");
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
    int ships;
    struct nstr_item ni;
    struct mchrstr *mp;
    struct shpstr ship;

    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, &ship)) {
	if (player->owner)
	    continue;
	if (!ship.shp_own)
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
	    if (!pct_chance(pln_hitchance(pp,
					  shp_hardtarget(&ship), EF_SHIP)))
		continue;
	}
	add_shiplist(ship.shp_uid, head);
	list_ship(&ship, !ships);
	ships++;
    }
    return ships;
}

void
print_shiplist(struct shiplist *head)
{
    struct shiplist *s;
    struct shpstr ship;

    for (s = head; s; s = s->next) {
	getship(s->uid, &ship);
	list_ship(&ship, s == head);
    }
}

int
adj_units(coord x, coord y, natid own)
{
    int i;
    struct sctstr sect;

    for (i = DIR_FIRST; i <= DIR_LAST; i++) {
	getsect(x + diroff[i][0], y + diroff[i][1], &sect);
	if (has_units(sect.sct_x, sect.sct_y, own))
	    return 1;
    }
    return 0;
}

int
has_units(coord x, coord y, natid cn)
{
    int n;
    struct lndstr land;

    for (n = 0; ef_read(EF_LAND, n, &land); n++) {
	if (land.lnd_x != x || land.lnd_y != y)
	    continue;
	if (land.lnd_own == cn)
	    return 1;
    }

    return 0;
}
