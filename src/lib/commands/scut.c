/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  scut.c: Scuttle ships, planes or land units
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "empobj.h"
#include "optlist.h"

static void scuttle_land(struct lndstr *);

int
scut(void)
{
    struct nstr_item ni;
    union empobj_storage item;
    int type;
    struct mchrstr *mp;
    char *p;
    char prompt[128];
    char buf[1024];

    if (!(p = getstarg(player->argp[1], "Ship, land, or plane? ", buf)))
	return RET_SYN;
    switch (*p) {
    case 's':
	type = EF_SHIP;
	break;
    case 'p':
	type = EF_PLANE;
	break;
    case 'l':
	type = EF_LAND;
	break;
    default:
	pr("Ships, land units, or planes only! (s, l, p)\n");
	return RET_SYN;
    }
    sprintf(prompt, "%s(s)? ", ef_nameof(type));
    if ((p = getstarg(player->argp[2], prompt, buf)) == 0)
	return RET_SYN;
    if (!snxtitem(&ni, type, p))
	return RET_SYN;
    if (p && (isalpha(*p) || (*p == '*') || (*p == '~') || issector(p)
	      || islist(p))) {
	char y_or_n[80], bbuf[80];

	if (type == EF_SHIP) {
	    if (*p == '*')
		sprintf(bbuf, "all ships");
	    else if (*p == '~')
		sprintf(bbuf, "all unassigned ships");
	    else if (issector(p))
		sprintf(bbuf, "all ships in %s", p);
	    else if (isalpha(*p))
		sprintf(bbuf, "fleet %c", *p);
	    else
		sprintf(bbuf, "ships %s", p);
	} else if (type == EF_LAND) {
	    if (*p == '*')
		sprintf(bbuf, "all land units");
	    else if (*p == '~')
		sprintf(bbuf, "all unassigned land units");
	    else if (issector(p))
		sprintf(bbuf, "all units in %s", p);
	    else if (isalpha(*p))
		sprintf(bbuf, "army %c", *p);
	    else
		sprintf(bbuf, "units %s", p);
	} else {
	    if (*p == '*')
		sprintf(bbuf, "all planes");
	    else if (*p == '~')
		sprintf(bbuf, "all unassigned planes");
	    else if (issector(p))
		sprintf(bbuf, "all planes in %s", p);
	    else if (isalpha(*p))
		sprintf(bbuf, "wing %c", *p);
	    else
		sprintf(bbuf, "planes %s", p);
	}
	sprintf(y_or_n, "Really scuttle %s? ", bbuf);
	if (!confirm(y_or_n))
	    return RET_FAIL;
    }
    while (nxtitem(&ni, &item)) {
	if (!player->owner)
	    continue;
	if (opt_MARKET) {
	    if (ontradingblock(type, &item.ship)) {
		pr("You cannot scuttle an item on the trading block!\n");
		continue;
	    }
	}

	if (type == EF_SHIP) {
	    mp = &mchr[(int)item.ship.shp_type];
	    if (opt_TRADESHIPS) {
		if (mp->m_flags & M_TRADE)
		    if (!scuttle_tradeship(&item.ship, 1))
			continue;
	    }
	    pr("%s", prship(&item.ship));
	    scuttle_ship(&item.ship);
	} else if (type == EF_LAND) {
	    if (item.land.lnd_ship >= 0) {
		pr("%s is on a ship, and cannot be scuttled!\n",
		   prland(&item.land));
		continue;
	    }
	    pr("%s", prland(&item.land));
	    scuttle_land(&item.land);
	} else {
	    pr("%s", prplane(&item.plane));
	    if (item.plane.pln_ship >= 0) {
		struct shpstr ship;

		getship(item.plane.pln_ship, &ship);
		take_plane_off_ship(&item.plane, &ship);
	    }
	    item.plane.pln_effic = 0;
	    putplane(item.plane.pln_uid, &item.plane);
	}
	pr(" scuttled in %s\n",
	   xyas(item.ship.shp_x, item.ship.shp_y, player->cnum));
    }

    return RET_OK;
}

int
scuttle_tradeship(struct shpstr *sp, int interactive)
{
    float cash = 0;
    float ally_cash = 0;
    int dist;
    struct sctstr sect;
    struct mchrstr *mp;
    struct natstr *np;
    char buf[512];
    struct natstr *natp;

    mp = &mchr[(int)sp->shp_type];
    getsect(sp->shp_x, sp->shp_y, &sect);
    if (sect.sct_own && sect.sct_type == SCT_HARBR) {
	dist = mapdist(sp->shp_x, sp->shp_y,
		       sp->shp_orig_x, sp->shp_orig_y);
	/* Don't disclose distance to to pirates */
	if (sp->shp_own == sp->shp_orig_own) {
	    if (interactive)
		pr("%s has gone %d sects\n", prship(sp), dist);
	    else
		wu(0, sp->shp_own, "%s has gone %d sects\n",
		   prship(sp), dist);
	}
	if (dist < trade_1_dist)
	    cash = 0;
	else if (dist < trade_2_dist)
	    cash = 1.0 + trade_1 * dist;
	else if (dist < trade_3_dist)
	    cash = 1.0 + trade_2 * dist;
	else
	    cash = 1.0 + trade_3 * dist;
	cash *= mp->m_cost;
	cash *= sp->shp_effic / 100.0;

	if (sect.sct_own != sp->shp_own) {
	    ally_cash = cash * trade_ally_cut;
	    cash *= (1.0 + trade_ally_bonus);
	}
    }

    if (!interactive && cash) {
	natp = getnatp(sp->shp_own);
	natp->nat_money += cash;
	putnat(natp);
	wu(0, sp->shp_own, "You just made $%d.\n", (int)cash);
    } else if (!cash && !interactive) {
	wu(0, sp->shp_own, "Unfortunately, you make $0 on this trade.\n");
    } else if (cash && interactive) {
	player->dolcost -= cash;
    } else if (interactive && sp->shp_own == sp->shp_orig_own) {
	pr("You won't get any money if you scuttle in %s!",
	   xyas(sp->shp_x, sp->shp_y, player->cnum));
	sprintf(buf, "Are you sure you want to scuttle %s? ", prship(sp));
	return confirm(buf);
    }

    if (ally_cash) {
	np = getnatp(sect.sct_own);
	np->nat_money += ally_cash;
	putnat(np);
	wu(0, sect.sct_own,
	   "Trade with %s nets you $%d at %s\n",
	   cname(sp->shp_own),
	   (int)ally_cash, xyas(sect.sct_x, sect.sct_y, sect.sct_own));
	if (sp->shp_own != sp->shp_orig_own)
	    nreport(sp->shp_own, N_PIRATE_TRADE, sp->shp_orig_own, 1);
	else
	    nreport(sp->shp_own, N_TRADE, sect.sct_own, 1);
    } else if (sp->shp_own != sp->shp_orig_own)
	nreport(sp->shp_own, N_PIRATE_KEEP, sp->shp_orig_own, 1);

    return 1;
}

void
scuttle_ship(struct shpstr *sp)
{
    struct nstr_item ni;
    struct sctstr sect;
    struct plnstr plane;
    struct lndstr land;

    getsect(sp->shp_x, sp->shp_y, &sect);
    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own == 0)
	    continue;
	if (plane.pln_ship == sp->shp_uid) {
	    plane.pln_ship = -1;
	    if (sect.sct_own != sp->shp_own) {
		wu(0, plane.pln_own, "Plane %d scuttled in %s\n",
		   plane.pln_uid,
		   xyas(plane.pln_x, plane.pln_y, plane.pln_own));
		plane.pln_effic = 0;
	    } else {
		wu(0, plane.pln_own,
		   "Plane %d transferred off ship %d to %s\n",
		   plane.pln_uid, sp->shp_uid,
		   xyas(plane.pln_x, plane.pln_y, plane.pln_own));
	    }
	    putplane(plane.pln_uid, &plane);
	}
    }
    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == 0)
	    continue;
	if (land.lnd_ship == sp->shp_uid) {
	    land.lnd_ship = -1;
	    if (sect.sct_own == sp->shp_own) {
		wu(0, land.lnd_own,
		   "Land unit %d transferred off ship %d to %s\n",
		   land.lnd_uid, sp->shp_uid,
		   xyas(land.lnd_x, land.lnd_y, land.lnd_own));
		putland(land.lnd_uid, &land);
	    } else
		scuttle_land(&land);
	}
    }
    sp->shp_effic = 0;
    putship(sp->shp_uid, sp);
}

static void
scuttle_land(struct lndstr *lp)
{
    struct nstr_item ni;
    struct sctstr sect;
    struct plnstr plane;
    struct lndstr land;

    getsect(lp->lnd_x, lp->lnd_y, &sect);
    snxtitem_all(&ni, EF_PLANE);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own == 0)
	    continue;
	if (plane.pln_land == lp->lnd_uid) {
	    plane.pln_land = -1;
	    if (sect.sct_own != lp->lnd_own) {
		wu(0, plane.pln_own, "Plane %d scuttled in %s\n",
		   plane.pln_uid,
		   xyas(plane.pln_x, plane.pln_y, plane.pln_own));
		plane.pln_effic = 0;
	    } else {
		wu(0, plane.pln_own,
		   "Plane %d transferred off unit %d to %s\n",
		   plane.pln_uid, lp->lnd_uid,
		   xyas(plane.pln_x, plane.pln_y, plane.pln_own));
	    }
	    putplane(plane.pln_uid, &plane);
	}
    }
    snxtitem_all(&ni, EF_LAND);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == 0)
	    continue;
	if (land.lnd_land == lp->lnd_uid) {
	    land.lnd_land = -1;
	    if (sect.sct_own == lp->lnd_own) {
		wu(0, land.lnd_own,
		   "Land unit %d transferred off unit %d to %s\n",
		   land.lnd_uid, lp->lnd_uid,
		   xyas(land.lnd_x, land.lnd_y, land.lnd_own));
		putland(land.lnd_uid, &land);
	    } else
		scuttle_land(&land);
	}
    }
    lp->lnd_effic = 0;
    putland(lp->lnd_uid, lp);
}
