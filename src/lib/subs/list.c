/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  list.c: List ships, planes, units at a given x,y
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "news.h"
#include "ship.h"
#include "land.h"
#include "sect.h"
#include "nuke.h"
#include "plane.h"
#include "nat.h"
#include "item.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "prototypes.h"

int
shipsatxy(coord x, coord y, int wantflags, int nowantflags)
{
    int first;
    int ships;
    struct nstr_item ni;
    struct mchrstr *mp;
    struct shpstr ship;

    first = 1;
    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, (s_char *)&ship)) {
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

/* This one only shows owned or allied ships */

int
carriersatxy(coord x, coord y, int wantflags, int nowantflags, natid own)
{
    int first;
    int ships;
    struct nstr_item ni;
    struct mchrstr *mp;
    struct shpstr ship;
    int allied;

    first = 1;
    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, (s_char *)&ship)) {
	if (ship.shp_effic < SHIP_MINEFF || ship.shp_own == 0)
	    continue;
	allied = (getrel(getnatp(ship.shp_own), own) == ALLIED);
	if ((ship.shp_own != own) && !allied)
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
    while (nxtitem(&ni, (s_char *)&land)) {
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
planesatxy(coord x, coord y, int wantflags, int nowantflags,
	   struct emp_qelem *list)
{
    int first;
    int planes;
    struct plnstr plane;
    struct nstr_item ni;
    struct plchrstr *plp;

    planes = 0;
    first = 1;
    snxtitem_xy(&ni, EF_PLANE, x, y);
    while (nxtitem(&ni, (s_char *)&plane)) {
	if (plane.pln_effic < PLANE_MINEFF || plane.pln_own == 0)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	/* Is this plane one of the ones flying somewhere? */
	if (ac_isflying(&plane, list))
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
	      struct plnstr *pp, struct shiplook *head)
{
    int first;
    int ships;
    struct nstr_item ni;
    struct mchrstr *mp;
    struct shpstr ship;

    first = 1;
    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, (s_char *)&ship)) {
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
	set_have_found(ship.shp_uid, head);
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
num_shipsatxy(coord x, coord y, int wantflags, int nowantflags)
{
    int ships;
    struct nstr_item ni;
    struct mchrstr *mp;
    struct shpstr ship;

    ships = 0;
    snxtitem_xy(&ni, EF_SHIP, x, y);
    while (nxtitem(&ni, (s_char *)&ship)) {
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
	ships++;
    }
    return ships;
}

/*
 * is p a list of ships/planes/units?
 *
 */

int
islist(s_char *p)
{
    register int x;

    x = 0;

    while (*(p + x)) {
	if (!isdigit(*(p + x)) && (*(p + x) != '/'))
	    return 0;

	x++;
    }

    return 1;
}
