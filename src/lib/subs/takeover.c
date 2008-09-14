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
 *  takeover.c: Take over from another country
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996-2000
 */

#include <config.h>

#include "file.h"
#include "game.h"
#include "land.h"
#include "lost.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"
#include "plane.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

void
takeover(struct sctstr *sp, natid newown)
{
    struct plnstr *pp;
    struct lndstr *lp;
    int civ;
    int che_count;
    int oldche;
    int n;
    struct nstr_item ni;
    struct plnstr p;
    struct lndstr land;

    /* Wipe all the distribution info */
    memset(sp->sct_dist, 0, sizeof(sp->sct_dist));
    memset(sp->sct_del, 0, sizeof(sp->sct_del));
    if (sp->sct_own == 0)
	sp->sct_off = 0;
    else
	sp->sct_off = 1;
    sp->sct_dist_x = sp->sct_x;
    sp->sct_dist_y = sp->sct_y;

    pp = &p;
    /* Take over planes */
    snxtitem_xy(&ni, EF_PLANE, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, pp)) {
	if (pp->pln_own != sp->sct_own)
	    continue;
	takeover_plane(pp, newown);
    }

    /* Take over land units */
    lp = &land;
    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, lp)) {
	if ((lp->lnd_own == newown) || (lp->lnd_own == 0))
	    continue;
	if (lp->lnd_own != sp->sct_own)
	    continue;
	if (lp->lnd_ship >= 0 || lp->lnd_land >= 0)
	    continue;
	/* Spies get a chance to hide */
	if (lchr[(int)lp->lnd_type].l_flags & L_SPY) {
	    if (!(chance(LND_SPY_DETECT_CHANCE(lp->lnd_effic))))
		continue;
	}
	n = lp->lnd_effic - (30 + (random() % 100));
	if (n < 0)
	    n = 0;
	lp->lnd_effic = n;
	if (lp->lnd_effic < LAND_MINEFF) {
	    lp->lnd_effic = 0;
	    mpr(newown, "%s blown up by the crew!\n", prland(lp));
	    wu(0, lp->lnd_own,
	       "%s blown up by the crew when %s took %s!\n",
	       prland(lp),
	       cname(newown), xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));
	} else {
	    mpr(newown, "We have captured %s!\n", prland(lp));
	    wu(0, lp->lnd_own,
	       "%s captured when %s took %s!\n",
	       prland(lp),
	       cname(newown), xyas(lp->lnd_x, lp->lnd_y, lp->lnd_own));
	}
	takeover_land(lp, newown);
    }

    sp->sct_avail = 0;
    civ = sp->sct_item[I_CIVIL];
    oldche = sp->sct_che;
    /*
     * create guerrillas from civilians
     * how spunky are these guys?
     * n: random number from -25:75 + (50 - loyalty)
     */
    n = (50 - sp->sct_loyal) + ((random() % 100) - 25);
    che_count = 0;
    if (n > 0 && sp->sct_own == sp->sct_oldown) {
	che_count = (civ * n / 3000) + 5;
	if (che_count * 2 > civ)
	    che_count = civ / 2;
	che_count /= hap_fact(getnatp(newown), getnatp(sp->sct_own));
	if (che_count + oldche > CHE_MAX)
	    che_count = CHE_MAX - oldche;
	if (che_count > 0) {
	    civ -= che_count;
	    che_count += oldche;
	} else
	    che_count = oldche;
    } else
	che_count = oldche;
    sp->sct_che = che_count;
    if (newown != sp->sct_oldown)
	sp->sct_che_target = newown;
    if (sp->sct_che_target == 0)
	sp->sct_che = 0;
    sp->sct_item[I_CIVIL] = civ;
    if (sp->sct_oldown == newown || civ == 0) {
	/*
	 * taking over one of your old sectors
	 */
	sp->sct_loyal = 0;
	sp->sct_oldown = newown;
    } else {
	/*
	 * taking over someone else's sector
	 */
	sp->sct_loyal = 50;
    }
    sp->sct_own = newown;
    if (opt_MOB_ACCESS) {
	game_tick_to_now(&sp->sct_access);
	sp->sct_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	sp->sct_mobil = 0;
    }
}

void
takeover_plane(struct plnstr *pp, natid newown)
{
    int n;

    if ((pp->pln_own == newown) || (pp->pln_own == 0))
	return;
    if (pp->pln_flags & PLN_LAUNCHED)
	return;
    if (pp->pln_ship >= 0 || pp->pln_land >= 0)
	return;
    /*
     * XXX If this was done right, planes could escape,
     * flying to a nearby friendly airport.
     */
    n = pp->pln_effic - (30 + (random() % 100));
    if (n < 0)
	n = 0;
    pp->pln_effic = n;
    if (pp->pln_effic < PLANE_MINEFF || pp->pln_harden > 0) {
	pp->pln_effic = 0;
	mpr(newown, "%s blown up by the crew!\n", prplane(pp));
	wu(0, pp->pln_own,
	   "%s blown up by the crew to avoid capture by %s at %s!\n",
	   prplane(pp),
	   cname(newown), xyas(pp->pln_x, pp->pln_y, pp->pln_own));
    } else {
	mpr(newown, "We have captured %s!\n", prplane(pp));
	wu(0, pp->pln_own,
	   "%s captured by %s at %s!\n",
	   prplane(pp),
	   cname(newown), xyas(pp->pln_x, pp->pln_y, pp->pln_own));
    }
    if (opt_MARKET)
	trdswitchown(EF_PLANE, pp, newown);
    if (pp->pln_mobil > 0)
	pp->pln_mobil = 0;
    pp->pln_own = newown;
    pp->pln_mission = 0;
    pp->pln_wing = 0;
    putplane(pp->pln_uid, pp);
}

void
takeover_ship(struct shpstr *sp, natid newown)
{
    struct plnstr *pp;
    struct lndstr *lp;
    struct nstr_item ni;
    struct plnstr p;
    struct lndstr llp;

    if (opt_MARKET)
	trdswitchown(EF_SHIP, sp, newown);
    sp->shp_own = newown;
    sp->shp_mission = 0;
    sp->shp_fleet = 0;
    sp->shp_rflags = 0;
    memset(sp->shp_rpath, 0, sizeof(sp->shp_rpath));
    pp = &p;
    lp = &llp;
    /* Take over planes */
    snxtitem_cargo(&ni, EF_PLANE, EF_SHIP, sp->shp_uid);
    while (nxtitem(&ni, pp)) {
	if (pp->pln_effic > PLANE_MINEFF)
	    pp->pln_effic = PLANE_MINEFF;
	pp->pln_mobil = 0;
	if (opt_MOB_ACCESS)
	    game_tick_to_now(&pp->pln_access);
	if (opt_MARKET)
	    trdswitchown(EF_PLANE, pp, newown);
	pp->pln_mission = 0;
	pp->pln_own = newown;
	putplane(pp->pln_uid, pp);
    }
    /* Take over land units */
    snxtitem_cargo(&ni, EF_LAND, EF_SHIP, sp->shp_uid);
    while (nxtitem(&ni, lp))
	takeover_land(lp, newown);
    putship(sp->shp_uid, sp);
}

void
takeover_land(struct lndstr *landp, natid newown)
{
    struct plnstr *pp;
    struct lndstr *lp;
    struct nstr_item ni;
    struct plnstr p;
    struct lndstr llp;

    if (landp->lnd_effic < LAND_MINEFF) {
	putland(landp->lnd_uid, landp);
	return;
    }
    landp->lnd_army = 0;
    landp->lnd_mobil = 0;
    if (opt_MOB_ACCESS)
	game_tick_to_now(&landp->lnd_access);
    landp->lnd_harden = 0;
    if (opt_MARKET)
	trdswitchown(EF_LAND, landp, newown);
    landp->lnd_mission = 0;
    landp->lnd_own = newown;
    pp = &p;
    lp = &llp;
    /* Take over planes */
    snxtitem_cargo(&ni, EF_PLANE, EF_LAND, landp->lnd_uid);
    while (nxtitem(&ni, pp)) {
	if (pp->pln_effic > PLANE_MINEFF)
	    pp->pln_effic = PLANE_MINEFF;
	pp->pln_mobil = 0;
	if (opt_MOB_ACCESS)
	    game_tick_to_now(&pp->pln_access);
	if (opt_MARKET)
	    trdswitchown(EF_PLANE, pp, newown);
	pp->pln_mission = 0;
	pp->pln_own = newown;
	putplane(pp->pln_uid, pp);
    }
    /* Take over land units */
    snxtitem_cargo(&ni, EF_LAND, EF_LAND, landp->lnd_uid);
    while (nxtitem(&ni, lp))
	takeover_land(lp, newown);
    putland(landp->lnd_uid, landp);
}
