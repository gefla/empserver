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
 *  takeover.c: Take over from another country
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996-2000
 */

#include "misc.h"
#include "var.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "plane.h"
#include "land.h"
#include "ship.h"
#include "file.h"
#include "prototypes.h"
#include "optlist.h"

#define CHE_MAX                 255

void takeover_land(register struct lndstr *landp, natid newown,
		   int hostile);
void takeover_plane(register struct plnstr *pp, natid newown);

void
takeover(register struct sctstr *sp, natid newown)
{
    struct plnstr *pp;
    struct lndstr *lp;
    int civ;
    int che;
    int che_count;
    int oldche;
    int n, vec[I_MAX + 1];
    struct nstr_item ni;
    struct plnstr p;
    struct lndstr land;

    /* Wipe all the distribution info */
    memset(vec, 0, sizeof(vec));
    putvec(VT_DIST, vec, (s_char *)sp, EF_SECTOR);
    putvec(VT_DEL, vec, (s_char *)sp, EF_SECTOR);
    if (sp->sct_own == 0)
	sp->sct_off = 0;
    else
	sp->sct_off = 1;
    sp->sct_dist_x = sp->sct_x;
    sp->sct_dist_y = sp->sct_y;

    pp = &p;
    /* Take over planes */
    snxtitem_dist(&ni, EF_PLANE, sp->sct_x, sp->sct_y, 0);
    while (nxtitem(&ni, (caddr_t)pp)) {
	if (pp->pln_own != sp->sct_own)
	    continue;
	takeover_plane(pp, newown);
    }

    /* Take over land units */
    lp = &land;
    snxtitem_dist(&ni, EF_LAND, sp->sct_x, sp->sct_y, 0);
    while (nxtitem(&ni, (caddr_t)lp)) {
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
	takeover_land(lp, newown, 1);
    }

    sp->sct_avail = 0;
    civ = getvar(V_CIVIL, (s_char *)sp, EF_SECTOR);
    oldche = get_che_value(getvar(V_CHE, (s_char *)sp, EF_SECTOR));
    /*
     * create guerrillas from civilians
     * how spunky are these guys?
     * n: random number from -25:75 + (50 - loyalty)
     */
    n = (50 - sp->sct_loyal) + ((random() % 100) - 25);
    che_count = 0;
    che = 0;
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
    set_che_value(che, che_count);
    if (newown != sp->sct_oldown)
	set_che_cnum(che, newown);
    (void)putvar(V_CHE, che, (s_char *)sp, EF_SECTOR);
    (void)putvar(V_CIVIL, civ, (s_char *)sp, EF_SECTOR);
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
    makelost(EF_SECTOR, sp->sct_own, 0, sp->sct_x, sp->sct_y);
    makenotlost(EF_SECTOR, newown, 0, sp->sct_x, sp->sct_y);
    sp->sct_own = newown;
    if (opt_MOB_ACCESS) {
	time(&sp->sct_access);
	sp->sct_mobil = -(etu_per_update / sect_mob_neg_factor);
    } else {
	sp->sct_mobil = 0;
    }
}

void
takeover_plane(register struct plnstr *pp, natid newown)
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
    if (pp->pln_effic < PLANE_MINEFF || pp->pln_harden > (s_char)0) {
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
	trdswitchown(EF_PLANE, (int *)pp, newown);
    if (pp->pln_mobil > (s_char)0)
	pp->pln_mobil = 0;
    makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x, pp->pln_y);
    pp->pln_own = newown;
    makenotlost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x, pp->pln_y);
    pp->pln_mission = 0;
    putplane(pp->pln_uid, pp);
}

void
takeover_ship(register struct shpstr *sp, natid newown, int hostile)
{
    register struct plnstr *pp;
    register struct lndstr *lp;
    struct nstr_item ni;
    struct plnstr p;
    struct lndstr llp;

    if (opt_MARKET)
	trdswitchown(EF_SHIP, (int *)sp, newown);
    makelost(EF_SHIP, sp->shp_own, sp->shp_uid, sp->shp_x, sp->shp_y);
    sp->shp_own = newown;
    makenotlost(EF_SHIP, sp->shp_own, sp->shp_uid, sp->shp_x, sp->shp_y);
    sp->shp_mission = 0;
    sp->shp_fleet = ' ';
    sp->shp_rflags = 0;
    /* Keep track of when this was taken over */
    time(&sp->shp_access);
    memset(sp->shp_rpath, 0, sizeof(sp->shp_rpath));
    pp = &p;
    lp = &llp;
    /* Take over planes */
    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, (caddr_t)pp)) {
	if (pp->pln_ship != sp->shp_uid)
	    continue;
	if (pp->pln_own == 0)
	    continue;
	if (hostile) {
	    if (pp->pln_effic > PLANE_MINEFF)
		pp->pln_effic = PLANE_MINEFF;
	}
	pp->pln_mobil = 0;
	/* Keep track of when this was taken over */
	time(&pp->pln_access);
	if (opt_MARKET)
	    trdswitchown(EF_PLANE, (int *)pp, newown);
	pp->pln_mission = 0;
	makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x, pp->pln_y);
	pp->pln_own = newown;
	makenotlost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x,
		    pp->pln_y);
	putplane(pp->pln_uid, pp);
    }
    /* Take over land units */
    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, (caddr_t)lp)) {
	if (lp->lnd_ship != sp->shp_uid)
	    continue;
	if (lp->lnd_own == 0)
	    continue;
	takeover_land(lp, newown, hostile);
    }
    putship(sp->shp_uid, sp);
}

void
takeover_land(register struct lndstr *landp, natid newown, int hostile)
{
    register struct plnstr *pp;
    register struct lndstr *lp;
    struct nstr_item ni;
    struct plnstr p;
    struct lndstr llp;

    if (landp->lnd_effic < LAND_MINEFF) {
	putland(landp->lnd_uid, landp);
	return;
    }
    landp->lnd_army = ' ';
    landp->lnd_mobil = 0;
    landp->lnd_harden = 0;
    /* Keep track of when this was taken over */
    time(&landp->lnd_access);
    if (opt_MARKET)
	trdswitchown(EF_LAND, (int *)landp, newown);
    landp->lnd_mission = 0;
    makelost(EF_LAND, landp->lnd_own, landp->lnd_uid, landp->lnd_x,
	     landp->lnd_y);
    landp->lnd_own = newown;
    makenotlost(EF_LAND, landp->lnd_own, landp->lnd_uid, landp->lnd_x,
		landp->lnd_y);
    pp = &p;
    lp = &llp;
    /* Take over planes */
    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, (caddr_t)pp)) {
	if (pp->pln_land != landp->lnd_uid)
	    continue;
	if (pp->pln_own == 0)
	    continue;
	if (hostile) {
	    if (pp->pln_effic > PLANE_MINEFF)
		pp->pln_effic = PLANE_MINEFF;
	}
	pp->pln_mobil = 0;
	/* Keep track of when this was taken over */
	time(&pp->pln_access);
	if (opt_MARKET)
	    trdswitchown(EF_PLANE, (int *)pp, newown);
	pp->pln_mission = 0;
	makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x, pp->pln_y);
	pp->pln_own = newown;
	makenotlost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x,
		    pp->pln_y);
	putplane(pp->pln_uid, pp);
    }
    /* Take over land units */
    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, (caddr_t)lp)) {
	if (lp->lnd_land != landp->lnd_uid)
	    continue;
	if (lp->lnd_own == 0)
	    continue;
	takeover_land(lp, newown, hostile);
    }
    putland(landp->lnd_uid, landp);
}
