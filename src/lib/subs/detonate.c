/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  detonate.c: Detonate a nuclear device in a sector.
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2009
 */

#include <config.h>

#include "file.h"
#include "land.h"
#include "lost.h"
#include "misc.h"
#include "nat.h"
#include "news.h"
#include "nsc.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

static void kaboom(int x, int y, int rad, natid cn);

int
detonate(struct nukstr *np, coord x, coord y, int airburst)
{
    int nuketype = np->nuk_type;
    natid bombown = np->nuk_own;
    struct nchrstr *ncp;
    struct plnstr plane;
    struct sctstr sect;
    struct shpstr ship;
    struct lndstr land;
    struct nukstr nuke;
    char *bp;
    char buf[128];
    char buf2[128];
    natid own;
    int type;
    int damage;
    int fallout;
    int rad;
    struct nstr_sect ns;
    struct nstr_item ni;
    int issea;

    mpr(bombown, "Releasing RV's for %s detonation...\n",
	airburst ? "airburst" : "groundburst");

    getsect(x, y, &sect);
    issea = sect.sct_type == SCT_WATER;
    ncp = &nchr[nuketype];
    kaboom(x, y, ncp->n_blast, bombown);
    rad = ncp->n_blast;
    if (!airburst)
	rad = rad * 2 / 3;
    np->nuk_effic = 0;
    putnuke(np->nuk_uid, np);

    snxtsct_dist(&ns, x, y, rad);
    while (nxtsct(&ns, &sect)) {
	/* Nukes falling on water affect only 1 sector */
	if ((sect.sct_x != x) && issea)
	    continue;
	if ((sect.sct_y != y) && issea)
	    continue;
	own = sect.sct_own;
	type = sect.sct_type;
	if ((damage = nukedamage(ncp, ns.curdist, airburst)) <= 0)
	    continue;
	if (type == SCT_SANCT) {
	    mpr(bombown, "bounced off %s\n", xyas(ns.x, ns.y, bombown));
	    mpr(own, "%s nuclear device bounced off %s\n",
		cname(bombown), xyas(ns.x, ns.y, bombown));
	    nreport(bombown, N_NUKE, own, 1);
	    continue;
	}
	sect_damage(&sect, damage);
	if (opt_FALLOUT) {
	    fallout = sect.sct_fallout;
	    if (ncp->n_flags & N_NEUT)
		fallout += damage * 30;
	    else
		fallout += damage * 3;
	    sect.sct_fallout = MIN(fallout, FALLOUT_MAX);
	}
	if (damage > 100) {
	    sect.sct_oldown = 0;
	    sect.sct_own = 0;
	    if (type == SCT_WATER || type == SCT_BSPAN ||
		type == SCT_BTOWER) {
		bp = "left nothing but water in %s\n";
		if (type != SCT_WATER) {
		    sect.sct_newtype = SCT_WATER;
		    sect.sct_type = SCT_WATER;
		}
	    } else {
		sect.sct_newtype = SCT_WASTE;
		sect.sct_type = SCT_WASTE;
		bp = "turned %s into a radioactive wasteland\n";
	    }
	} else {
	    sprintf(buf, "did %d%%%% damage in %%s\n", damage);
	    bp = buf;
	}
	(void)putsect(&sect);
	if (type != SCT_WATER)
	    nreport(bombown, N_NUKE, own, 1);
	mpr(bombown, bp, xyas(ns.x, ns.y, bombown));
	if (own != bombown && own != 0) {
	    (void)sprintf(buf2, bp, xyas(ns.x, ns.y, own));
	    mpr(own, "%s nuclear device %s\n", cname(bombown), buf2);
	}
    }

    snxtitem_dist(&ni, EF_PLANE, x, y, rad);
    while (nxtitem(&ni, &plane)) {
	/* Nukes falling on water affect only 1 sector */
	if ((plane.pln_x != x) && issea)
	    continue;
	if ((plane.pln_y != y) && issea)
	    continue;
	if ((own = plane.pln_own) == 0)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	damage = nukedamage(ncp, ni.curdist, airburst) - plane.pln_harden;
	if (damage <= 0)
	    continue;
	if (plane.pln_ship >= 0) {
	    /* Are we on a sub? */
	    getship(plane.pln_ship, &ship);

	    if (mchr[(int)ship.shp_type].m_flags & M_SUB) {
		struct sctstr sect1;

		/* Should we damage this sub? */
		getsect(ship.shp_x, ship.shp_y, &sect1);

		if (sect1.sct_type == SCT_BSPAN ||
		    sect1.sct_type == SCT_BTOWER ||
		    sect1.sct_type == SCT_WATER) {
		    /* Ok, we're not in a harbor or trapped
		       inland.  Now, did we get pasted
		       directly? */
		    if (ship.shp_x != x || ship.shp_y != y) {
			/* Nope, so don't mess with it */
			continue;
		    }
		}
	    }
	}
	planedamage(&plane, damage);
	if (own == bombown) {
	    mpr(bombown, "%s at %s reports %d%% damage\n",
		prplane(&plane),
		xyas(plane.pln_x, plane.pln_y, own), damage);
	} else {
	    mpr(own, "%s nuclear device did %d%% damage to %s at %s\n",
		cname(bombown), damage,
		prplane(&plane), xyas(plane.pln_x, plane.pln_y, own));
	}
	putplane(ni.cur, &plane);
    }

    snxtitem_dist(&ni, EF_LAND, x, y, rad);
    while (nxtitem(&ni, &land)) {
	/* Nukes falling on water affect only 1 sector */
	if ((land.lnd_x != x) && issea)
	    continue;
	if ((land.lnd_y != y) && issea)
	    continue;
	if ((own = land.lnd_own) == 0)
	    continue;
	if ((damage = nukedamage(ncp, ni.curdist, airburst)) <= 0)
	    continue;

	if (land.lnd_ship >= 0) {
	    /* Are we on a sub? */
	    getship(land.lnd_ship, &ship);

	    if (mchr[(int)ship.shp_type].m_flags & M_SUB) {
		struct sctstr sect1;

		/* Should we damage this sub? */
		getsect(ship.shp_x, ship.shp_y, &sect1);

		if (sect1.sct_type == SCT_BSPAN ||
		    sect1.sct_type == SCT_BTOWER ||
		    sect1.sct_type == SCT_WATER) {
		    /* Ok, we're not in a harbor or trapped
		       inland.  Now, did we get pasted
		       directly? */
		    if (ship.shp_x != x || ship.shp_y != y) {
			/* Nope, so don't mess with it */
			continue;
		    }
		}
	    }
	}
	land_damage(&land, damage);
	if (own == bombown) {
	    mpr(bombown, "%s at %s reports %d%% damage\n",
		prland(&land), xyas(land.lnd_x, land.lnd_y, own), damage);
	} else {
	    mpr(own, "%s nuclear device did %d%% damage to %s at %s\n",
		cname(bombown), damage,
		prland(&land), xyas(land.lnd_x, land.lnd_y, own));
	}
	putland(land.lnd_uid, &land);
    }

    snxtitem_dist(&ni, EF_SHIP, x, y, rad);
    while (nxtitem(&ni, &ship)) {
	/* Nukes falling on water affect only 1 sector */
	if ((ship.shp_x != x) && issea)
	    continue;
	if ((ship.shp_y != y) && issea)
	    continue;
	if ((own = ship.shp_own) == 0)
	    continue;
	if ((damage = nukedamage(ncp, ni.curdist, airburst)) <= 0)
	    continue;
	if (mchr[(int)ship.shp_type].m_flags & M_SUB) {
	    struct sctstr sect1;

	    /* Should we damage this sub? */
	    getsect(ship.shp_x, ship.shp_y, &sect1);

	    if (sect1.sct_type == SCT_BSPAN ||
		sect1.sct_type == SCT_BTOWER ||
		sect1.sct_type == SCT_WATER) {
		/* Ok, we're not in a harbor or trapped
		   inland.  Now, did we get pasted
		   directly? */
		if (ship.shp_x != x || ship.shp_y != y) {
		    /* Nope, so don't mess with it */
		    continue;
		}
	    }
	}
	ship_damage(&ship, damage);
	if (own == bombown) {
	    mpr(bombown, "%s at %s reports %d%% damage\n",
		prship(&ship), xyas(ship.shp_x, ship.shp_y, own), damage);
	} else {
	    mpr(own, "%s nuclear device did %d%% damage to %s at %s\n",
		cname(bombown), damage, prship(&ship),
		xyas(ship.shp_x, ship.shp_y, own));
	}
	putship(ship.shp_uid, &ship);
    }

    snxtitem_dist(&ni, EF_NUKE, x, y, rad);
    while (nxtitem(&ni, &nuke)) {
	/* Nukes falling on water affect only 1 sector */
	if ((nuke.nuk_x != x) && issea)
	    continue;
	if ((nuke.nuk_y != y) && issea)
	    continue;
	if ((own = nuke.nuk_own) == 0)
	    continue;
	if ((damage = nukedamage(ncp, ni.curdist, airburst)) <= 0)
	    continue;
	if (roll(100) >= damage)
	    continue;
	nuke.nuk_effic = 0;
	if (own == bombown) {
	    mpr(bombown, "%s at %s destroyed\n",
		prnuke(&nuke), xyas(nuke.nuk_x, nuke.nuk_y, own));
	} else {
	    mpr(own, "%s at %s destroyed\n",
		prnuke(&nuke), xyas(nuke.nuk_x, nuke.nuk_y, own));
	}
	putnuke(ni.cur, &nuke);
    }

    return nukedamage(ncp, 0, airburst);
}


/*
 * silly to be sure.
 */
static void
kaboom(int x, int y, int rad, natid cn)
{
    mpr(cn, "\n\nK A B O O ");
    while (rad-- > 1)
	mpr(cn, "O O ");
    mpr(cn, "M ! in %s\n\n", xyas(x, y, cn));
}
