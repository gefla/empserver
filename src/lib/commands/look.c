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
 *  look.c: Lookout from a ship or land unit
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "nsc.h"
#include "nat.h"
#include "path.h"
#include "file.h"
#include <fcntl.h>
#include "commands.h"
#include "optlist.h"

static void look_ship(struct shpstr *lookship);
static void look_land(struct lndstr *lookland);

int
look(void)
{
    int i;
    struct nstr_item ni;
    struct shpstr myship;
    struct sctstr sect;
    int x, y;
    int civ;
    int mil;
    unsigned char *bitmap;
    int changed = 0;

    if (!snxtitem(&ni, EF_SHIP, player->argp[1]))
	return RET_SYN;
    if ((bitmap = malloc((WORLD_X * WORLD_Y) / 8)) == 0) {
	logerror("malloc failed in look\n");
	pr("Memory error.  Tell the deity.\n");
	return RET_FAIL;
    }
    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
    while (nxtitem(&ni, &myship)) {
	if (!player->owner)
	    continue;
	look_ship(&myship);
	for (i = 0; i <= 6; i++) {
	    x = diroff[i][0] + myship.shp_x;
	    y = diroff[i][1] + myship.shp_y;
	    if (emp_getbit(x, y, bitmap))
		continue;
	    emp_setbit(x, y, bitmap);
	    getsect(x, y, &sect);
	    if (sect.sct_type == SCT_WATER)
		continue;
	    if (player->owner)
		pr("Your ");
	    else
		pr("%s (#%d) ", cname(sect.sct_own), sect.sct_own);
	    pr(dchr[sect.sct_type].d_name);
	    changed += map_set(player->cnum, x, y,
			       dchr[sect.sct_type].d_mnem, 0);
	    pr(" %d%% efficient ", player->owner ? sect.sct_effic :
	       roundintby((int)sect.sct_effic, 10));
	    civ = sect.sct_item[I_CIVIL];
	    mil = sect.sct_item[I_MILIT];
	    if (civ)
		pr("with %s%d civ ", player->owner ? "" : "approx ",
		   player->owner ? civ : roundintby(civ, 10));
	    if (mil)
		pr("with %s%d mil ", player->owner ? "" : "approx ",
		   player->owner ? mil : roundintby(mil, 10));
	    pr("@ %s\n", xyas(x, y, player->cnum));
	    if (opt_HIDDEN) {
		setcont(player->cnum, sect.sct_own, FOUND_LOOK);
	    }
	}
    }
    if (changed)
	writemap(player->cnum);
    free(bitmap);
    return RET_OK;
}

static void
look_ship(struct shpstr *lookship)
{
    struct shpstr *sp;
    struct mchrstr *smcp;
    struct mchrstr *tmcp;
    struct sctstr sect;
    int range;
    int vrange;
    int i;
    int dist;

    range = (int)techfact(lookship->shp_tech,
			  (double)mchr[(int)lookship->shp_type].m_vrnge);
    range = range * (lookship->shp_effic / 100.0);
    smcp = &mchr[(int)lookship->shp_type];
    if (smcp->m_flags & M_SUB)
	range = MIN(range, 1);
    for (i = 0; NULL != (sp = getshipp(i)); i++) {
	if (sp->shp_own == player->cnum || sp->shp_own == 0)
	    continue;
	dist = mapdist(sp->shp_x, sp->shp_y,
		       lookship->shp_x, lookship->shp_y);
	if (dist > ship_max_interdiction_range)
	    continue;
	tmcp = &mchr[(int)sp->shp_type];
	if (smcp->m_flags & M_SUB)
	    vrange = (int)(sp->shp_visib * range / 30.0);
	else
	    vrange = (int)(sp->shp_visib * range / 20.0);
	getsect(sp->shp_x, sp->shp_y, &sect);
	if (sect.sct_type != SCT_WATER)
	    vrange = MAX(1, vrange);
	if (dist > vrange)
	    continue;
	if (smcp->m_flags & M_SUB) {
	    if (tmcp->m_flags & M_SONAR && dist < 2) {
		if (sp->shp_own != 0)
		    wu(0, sp->shp_own,
		       "%s detected surfacing noises in %s.\n",
		       prship(sp),
		       xyas(lookship->shp_x, lookship->shp_y,
			    sp->shp_own));
	    }
	    if (dist == 0 && (tmcp->m_flags & M_SUB) == 0)
		if (sp->shp_own != 0)
		    wu(0, sp->shp_own,
		       "Periscope spotted in %s by %s\n",
		       xyas(lookship->shp_x, lookship->shp_y,
			    sp->shp_own), prship(sp));
	}
	/* subs at sea only seen by sonar */
	if (tmcp->m_flags & M_SUB && sect.sct_type == SCT_WATER)
	    continue;
	pr("%s (#%d) %s @ %s\n",
	   cname(sp->shp_own), sp->shp_own, prship(sp),
	   xyas(sp->shp_x, sp->shp_y, player->cnum));
	if (opt_HIDDEN)
	    setcont(player->cnum, sp->shp_own, FOUND_LOOK);
    }
}

int
llook(void)
{
    int i;
    struct nstr_item ni;
    struct lndstr myland;
    struct sctstr sect;
    int x, y;
    int civ;
    int mil;
    unsigned char *bitmap;
    int changed = 0;

    if (!snxtitem(&ni, EF_LAND, player->argp[1]))
	return RET_SYN;
    if ((bitmap = malloc((WORLD_X * WORLD_Y) / 8)) == 0) {
	logerror("malloc failed in llook\n");
	pr("Memory error.  Tell the deity.\n");
	return RET_FAIL;
    }
    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
    while (nxtitem(&ni, &myland)) {
	if (!player->owner)
	    continue;
	if (myland.lnd_ship >= 0)
	    continue;
	if (myland.lnd_land >= 0)
	    continue;
	/* Spies don't need military to do a "llook".  Other
	   units do */
	if ((lnd_getmil(&myland) <= 0) &&
	    !(lchr[(int)myland.lnd_type].l_flags & L_SPY))
	    continue;
	look_land(&myland);
	for (i = 0; i <= 6; i++) {
	    x = diroff[i][0] + myland.lnd_x;
	    y = diroff[i][1] + myland.lnd_y;
	    if (emp_getbit(x, y, bitmap))
		continue;
	    emp_setbit(x, y, bitmap);
	    getsect(x, y, &sect);
	    if (sect.sct_type == SCT_WATER)
		continue;
	    if (player->owner)
		pr("Your ");
	    else
		pr("%s (#%d) ", cname(sect.sct_own), sect.sct_own);
	    pr(dchr[sect.sct_type].d_name);
	    changed += map_set(player->cnum, x, y,
			       dchr[sect.sct_type].d_mnem, 0);
	    pr(" %d%% efficient ", player->owner ? sect.sct_effic :
	       roundintby((int)sect.sct_effic, 10));
	    civ = sect.sct_item[I_CIVIL];
	    mil = sect.sct_item[I_MILIT];
	    if (civ)
		pr("with %s%d civ ", player->owner ? "" :
		   "approx ", player->owner ? civ : roundintby(civ, 10));
	    if (mil)
		pr("with %s%d mil ", player->owner ? "" :
		   "approx ", player->owner ? mil : roundintby(mil, 10));
	    pr("@ %s\n", xyas(x, y, player->cnum));
	    if (opt_HIDDEN) {
		setcont(player->cnum, sect.sct_own, FOUND_LOOK);
	    }
	}
    }
    if (changed)
	writemap(player->cnum);
    free(bitmap);
    return RET_OK;
}

static void
look_land(struct lndstr *lookland)
{
    struct plnstr *pp;
    struct lndstr *lp;
    double drange;
    int range;
    int vrange;
    int i;
    int dist;

    drange = techfact(lookland->lnd_tech, (double)lookland->lnd_spy);
    drange = (drange * ((double)lookland->lnd_effic / 100.0));
    range = ldround(drange, 1);

    if (range == 0)
	return;

    for (i = 0; NULL != (lp = getlandp(i)); i++) {
	if (lp->lnd_own == player->cnum || lp->lnd_own == 0)
	    continue;
	if (lp->lnd_ship >= 0)
	    continue;
	/* Don't always see spies */
	if (lchr[(int)lp->lnd_type].l_flags & L_SPY) {
	    /* If it's on a ship or unit, assume it's hidden
	       enough not to be seen */
	    if (lp->lnd_ship >= 0 || lp->lnd_land >= 0)
		continue;
	    if (!(chance(LND_SPY_DETECT_CHANCE(lp->lnd_effic))))
		continue;
	}
	vrange = ldround((double)((lp->lnd_vis * range) / 20.0), 1);
	dist = mapdist(lp->lnd_x, lp->lnd_y,
		       lookland->lnd_x, lookland->lnd_y);
	if (dist > vrange)
	    continue;

	pr("%s (#%d) %s (approx %d mil) @ %s\n",
	   cname(lp->lnd_own), lp->lnd_own,
	   prland(lp), ldround((double)total_mil(lp), 20),
	   xyas(lp->lnd_x, lp->lnd_y, player->cnum));
	if (opt_HIDDEN)
	    setcont(player->cnum, lp->lnd_own, FOUND_LOOK);
    }
    for (i = 0; NULL != (pp = getplanep(i)); i++) {
	if (pp->pln_own == player->cnum || pp->pln_own == 0)
	    continue;
	if (pp->pln_ship >= 0)
	    continue;
	if (pp->pln_flags & PLN_LAUNCHED)
	    continue;
	vrange = ldround((double)((10 * range) / 20.0), 1);
	dist = mapdist(pp->pln_x, pp->pln_y,
		       lookland->lnd_x, lookland->lnd_y);
	if (dist > vrange)
	    continue;

	pr("%s (#%d) %s @ %s\n",
	   cname(pp->pln_own), pp->pln_own,
	   prplane(pp), xyas(pp->pln_x, pp->pln_y, player->cnum));
	if (opt_HIDDEN)
	    setcont(player->cnum, pp->pln_own, FOUND_LOOK);
    }
}
