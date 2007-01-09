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
 *  scra.c: Scrap ships, planes or land units
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "empobj.h"
#include "optlist.h"

int
scra(void)
{
    struct nstr_item ni;
    union empobj_storage item;
    int type;
    struct sctstr sect;
    struct mchrstr *mp;
    struct plchrstr *pp;
    struct lchrstr *lp;
    char *p;
    i_type i;
    struct nstr_item ni2;
    struct plnstr plane;
    struct lndstr land;
    struct sctstr sect2;
    char prompt[128];
    char buf[1024];
    float eff;

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

	memset(y_or_n, 0, sizeof(y_or_n));
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

	sprintf(y_or_n, "Really scrap %s [n]? ", bbuf);
	if (!confirm(y_or_n))
	    return RET_FAIL;
    }
    while (nxtitem(&ni, &item)) {
	if (!player->owner)
	    continue;

	if (opt_MARKET) {
	    if (ontradingblock(type, &item.ship)) {
		pr("You cannot scrap an item on the trading block!\n");
		continue;
	    }
	}

	if (type == EF_SHIP) {
	    getsect(item.ship.shp_x, item.ship.shp_y, &sect);
	    if (sect.sct_type != SCT_HARBR)
		continue;
	    if (sect.sct_effic < 60 || sect.sct_own != player->cnum)
		continue;
	} else if (type == EF_LAND) {
	    if (item.land.lnd_ship >= 0) {
		pr("%s is on a ship, and cannot be scrapped!\n",
		   prland(&item.land));
		continue;
	    }
	    getsect(item.land.lnd_x, item.land.lnd_y, &sect);
	} else {
	    getsect(item.plane.pln_x, item.plane.pln_y, &sect);
	    if (sect.sct_type != SCT_AIRPT)
		continue;
	    if (sect.sct_effic < 60 ||
		(sect.sct_own != player->cnum &&
		 getrel(getnatp(sect.sct_own), player->cnum) < FRIENDLY))
		continue;
	}
	if (type == EF_SHIP) {
	    eff = item.ship.shp_effic / 100.0;
	    mp = &mchr[(int)item.ship.shp_type];
	    if (opt_TRADESHIPS) {
		if (mp->m_flags & M_TRADE) {
		    pr("WARNING: You only collect money from trade ships if you \"scuttle\" them!\n");
		    sprintf(prompt,
			    "Are you really sure that you want to scrap %s (n)? ",
			    prship(&item.ship));
		    if (!confirm(prompt)) {
			pr("%s not scrapped\n", prship(&item.ship));
			continue;
		    }
		}
	    }
	    pr("%s", prship(&item.ship));
	    for (i = I_NONE + 1; i <= I_MAX; i++) {
		sect.sct_item[i] += item.ship.shp_item[i];
	    }
	    sect.sct_item[I_LCM] += mp->m_lcm * 2 / 3 * eff;
	    sect.sct_item[I_HCM] += mp->m_hcm * 2 / 3 * eff;
	    getsect(item.ship.shp_x, item.ship.shp_y, &sect2);
	    snxtitem_all(&ni2, EF_PLANE);
	    while (nxtitem(&ni2, &plane)) {
		if (plane.pln_own == 0)
		    continue;
		if (plane.pln_ship == item.ship.shp_uid) {
		    wu(0, plane.pln_own,
		       "Plane %d transferred off ship %d to %s\n",
		       ni2.cur, item.ship.shp_uid,
		       xyas(plane.pln_x, plane.pln_y, player->cnum));
		    plane.pln_ship = -1;
		    if (sect2.sct_own != plane.pln_own) {
			wu(0, plane.pln_own,
			   "%s given to %s\n", prplane(&plane),
			   cname(sect2.sct_own));
			wu(0, sect2.sct_own,
			   "%s given to you by %s\n", prplane(&plane),
			   cname(player->cnum));
		    }
		    makelost(EF_PLANE, plane.pln_own, plane.pln_uid,
			     plane.pln_x, plane.pln_y);
		    plane.pln_own = sect2.sct_own;
		    makenotlost(EF_PLANE, plane.pln_own, plane.pln_uid,
				plane.pln_x, plane.pln_y);
		    putplane(plane.pln_uid, &plane);
		}
	    }
	    snxtitem_all(&ni2, EF_LAND);
	    while (nxtitem(&ni2, &land)) {
		if (land.lnd_own == 0)
		    continue;
		if (land.lnd_ship == item.ship.shp_uid) {
		    wu(0, land.lnd_own,
		       "Land unit %d transferred off ship %d to %s\n",
		       ni2.cur, item.ship.shp_uid,
		       xyas(land.lnd_x, land.lnd_y, player->cnum));
		    land.lnd_ship = -1;
		    if (sect2.sct_own != land.lnd_own) {
			wu(0, land.lnd_own,
			   "%s given to %s\n", prland(&land),
			   cname(sect2.sct_own));
			wu(0, sect2.sct_own,
			   "%s given to you by %s\n", prland(&land),
			   cname(player->cnum));
		    }
		    makelost(EF_LAND, land.lnd_own, land.lnd_uid,
			     land.lnd_x, land.lnd_y);
		    land.lnd_own = sect2.sct_own;
		    makenotlost(EF_LAND, land.lnd_own, land.lnd_uid,
				land.lnd_x, land.lnd_y);
		    putland(land.lnd_uid, &land);
		}
	    }
	    item.ship.shp_effic = 0;
	    putship(item.ship.shp_uid, &item.ship);
	} else if (type == EF_LAND) {
	    eff = item.land.lnd_effic / 100.0;
	    lp = &lchr[(int)item.land.lnd_type];
	    pr("%s", prland(&item.land));
	    for (i = I_NONE + 1; i <= I_MAX; i++) {
		sect.sct_item[i] += item.land.lnd_item[i];
	    }
	    sect.sct_item[I_LCM] += lp->l_lcm * 2 / 3 * eff;
	    sect.sct_item[I_HCM] += lp->l_hcm * 2 / 3 * eff;
	    getsect(item.land.lnd_x, item.land.lnd_y, &sect2);

	    snxtitem_all(&ni2, EF_LAND);
	    while (nxtitem(&ni2, &land)) {
		if (land.lnd_own == 0)
		    continue;
		if (land.lnd_land == item.land.lnd_uid) {
		    wu(0, land.lnd_own,
		       "Land unit %d transferred off land unit %d to %s\n",
		       land.lnd_uid, item.land.lnd_uid,
		       xyas(land.lnd_x, land.lnd_y, player->cnum));
		    land.lnd_land = -1;
		    if (sect2.sct_own != land.lnd_own) {
			wu(0, land.lnd_own,
			   "%s given to %s\n", prland(&land),
			   cname(sect2.sct_own));
			wu(0, sect2.sct_own,
			   "%s given to you by %s\n", prland(&land),
			   cname(player->cnum));
		    }
		    makelost(EF_LAND, land.lnd_own, land.lnd_uid,
			     land.lnd_x, land.lnd_y);
		    land.lnd_own = sect2.sct_own;
		    makenotlost(EF_LAND, land.lnd_own, land.lnd_uid,
				land.lnd_x, land.lnd_y);
		    putland(land.lnd_uid, &land);
		}
	    }

	    snxtitem_all(&ni2, EF_PLANE);
	    while (nxtitem(&ni2, &plane)) {
		if (plane.pln_own == 0)
		    continue;
		if (plane.pln_land == item.land.lnd_uid) {
		    wu(0, plane.pln_own,
		       "Plane %d transferred off land unit %d to %s\n",
		       ni2.cur, item.land.lnd_uid,
		       xyas(plane.pln_x, plane.pln_y, player->cnum));
		    plane.pln_land = -1;
		    if (sect2.sct_own != plane.pln_own) {
			wu(0, plane.pln_own,
			   "%s given to %s\n", prplane(&plane),
			   cname(sect2.sct_own));
			wu(0, sect2.sct_own,
			   "%s given to you by %s\n", prplane(&plane),
			   cname(player->cnum));
		    }
		    makelost(EF_PLANE, plane.pln_own, plane.pln_uid,
			     plane.pln_x, plane.pln_y);
		    plane.pln_own = sect2.sct_own;
		    makenotlost(EF_PLANE, plane.pln_own, plane.pln_uid,
				plane.pln_x, plane.pln_y);
		    putplane(plane.pln_uid, &plane);
		}
	    }
	    item.land.lnd_effic = 0;
	    putland(item.land.lnd_uid, &item.land);
	} else {
	    eff = item.land.lnd_effic / 100.0;
	    pp = &plchr[(int)item.plane.pln_type];
	    pr("%s", prplane(&item.plane));
	    sect.sct_item[I_LCM] += pp->pl_lcm * 2 / 3 * eff;
	    sect.sct_item[I_HCM] += pp->pl_hcm * 2 / 3 * eff;
	    sect.sct_item[I_MILIT] += pp->pl_crew;
	    item.plane.pln_effic = 0;
	    putplane(item.plane.pln_uid, &item.plane);
	}
	pr(" scrapped in %s\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum));
	for (i = I_NONE + 1; i <= I_MAX; i++) {
	    if (sect.sct_item[i] > ITEM_MAX)
		sect.sct_item[i] = ITEM_MAX;
	}
	putsect(&sect);
    }
    return RET_OK;
}
