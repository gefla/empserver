/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  tend.c: Transfer goodies from one ship to another.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1992
 *     Steve McClure, 2000
 */

#include <string.h>
#include "misc.h"
#include "player.h"
#include "plague.h"
#include "xy.h"
#include "file.h"
#include "ship.h"
#include "item.h"
#include "nsc.h"
#include "nat.h"
#include "land.h"
#include "plane.h"
#include "genitem.h"
#include "commands.h"

static void expose_ship(struct shpstr *s1, struct shpstr *s2);
static int tend_land(struct shpstr *tenderp, s_char *units);

int
tend(void)
{
    struct nstr_item targets;
    struct nstr_item tenders;
    struct shpstr tender;
    struct shpstr target;
    struct ichrstr *ip;
    struct mchrstr *vbase;
    int amt;
    int retval;
    int ontender;
    int ontarget;
    int maxtender;
    int maxtarget;
    int transfer;
    int total;
    int type;
    s_char *p;
    s_char prompt[512];
    s_char buf[1024];

    if (!(p = getstarg(player->argp[1],
		       "Tend what commodity (or 'land')? ", buf)) || !*p)
	return RET_SYN;

    if (!strncmp(p, "land", 4))
	type = EF_LAND;
    else if (NULL != (ip = item_by_name(p)))
	type = EF_SECTOR;
    else {
	pr("Can't tend '%s'\n", p);
	return RET_SYN;
    }

    if (!snxtitem(&tenders, EF_SHIP,
		  getstarg(player->argp[2], "Tender(s)? ", buf)))
	return RET_SYN;

    while (nxtitem(&tenders, (s_char *)&tender)) {
	if (!player->owner)
	    continue;
	if (type == EF_LAND) {
	    sprintf(prompt, "Land unit(s) to tend from %s? ",
		    prship(&tender));
	    if (!(p = getstarg(player->argp[3], prompt, buf)) || !*p)
		continue;
	    if (!check_ship_ok(&tender))
		return RET_SYN;
	    if (0 != (retval = tend_land(&tender, p)))
		return retval;
	    continue;
	}
	sprintf(prompt, "Number of %s to tend from %s? ",
		ip->i_name, prship(&tender));
	if (!(p = getstarg(player->argp[3], prompt, buf)) || !*p)
	    continue;
	if (!check_ship_ok(&tender))
	    return RET_SYN;
	if (!(amt = atoi(p))) {
	    pr("Amount must be non-zero!\n");
	    return RET_SYN;
	}
	ontender = tender.shp_item[ip->i_vtype];
	if (ontender == 0 && amt > 0) {
	    pr("No %s on %s\n", ip->i_name, prship(&tender));
	    return RET_FAIL;
	}
	vbase = &mchr[(int)tender.shp_type];
	maxtender = vbase->m_item[ip->i_vtype];
	if (maxtender == 0) {
	    pr("A %s cannot hold any %s\n",
	       mchr[(int)tender.shp_type].m_name, ip->i_name);
	    break;
	}
	if (!snxtitem(&targets, EF_SHIP,
		      getstarg(player->argp[4], "Ships to be tended? ",
			       buf)))
	    break;
	if (!check_ship_ok(&tender))
	    return RET_SYN;
	total = 0;
	while (tend_nxtitem(&targets, (s_char *)&target)) {
	    if (!player->owner &&
		(getrel(getnatp(target.shp_own), player->cnum) < FRIENDLY))
		continue;
	    if (target.shp_uid == tender.shp_uid)
		continue;
	    if (tender.shp_x != target.shp_x ||
		tender.shp_y != target.shp_y)
		continue;
	    ontarget = target.shp_item[ip->i_vtype];
	    if (ontarget == 0 && amt < 0) {
		pr("No %s on %s\n", ip->i_name, prship(&target));
		continue;
	    }
	    vbase = &mchr[(int)target.shp_type];
	    maxtarget = vbase->m_item[ip->i_vtype];
	    if (amt < 0) {
		if (!player->owner)
		    amt = 0;

		/* take from target and give to tender */
		transfer = min(ontarget, -amt);
		transfer = min(maxtender - ontender, transfer);
		if (transfer == 0)
		    continue;
		target.shp_item[ip->i_vtype] = ontarget - transfer;
		ontender += transfer;
		total += transfer;
	    } else {
		/* give to target from tender */
		transfer = min(ontender, amt);
		transfer = min(transfer, maxtarget - ontarget);
		if (transfer == 0)
		    continue;
		target.shp_item[ip->i_vtype] = ontarget + transfer;
		ontender -= transfer;
		total += transfer;
	    }
	    expose_ship(&tender, &target);
	    putship(target.shp_uid, &target);
	    if (amt > 0 && ontender == 0) {
		pr("%s out of %s\n", prship(&tender), ip->i_name);
		break;
	    }
	}
	pr("%d total %s transferred %s %s\n",
	   total, ip->i_name, (amt > 0) ? "off of" : "to",
	   prship(&tender));
	tender.shp_item[ip->i_vtype] = ontender;
	tender.shp_mission = 0;
	putship(tender.shp_uid, &tender);
    }
    return RET_OK;
}

static void
expose_ship(struct shpstr *s1, struct shpstr *s2)
{
    if (s1->shp_pstage == PLG_INFECT && s2->shp_pstage == PLG_HEALTHY)
	s2->shp_pstage = PLG_EXPOSED;
    if (s2->shp_pstage == PLG_INFECT && s1->shp_pstage == PLG_HEALTHY)
	s1->shp_pstage = PLG_EXPOSED;
}

/*
 * tend_nxtitem.c
 *
 * get next item from list. Stolen from nxtitem to make 1 itsy-bitsy change
 *
 * Dave Pare, 1989
 */

int
tend_nxtitem(struct nstr_item *np, void *ptr)
{
    struct genitem *gp;
    int selected;

    if (np->sel == NS_UNDEF)
	return 0;
    gp = (struct genitem *)ptr;
    do {
	if (np->sel == NS_LIST) {
	    np->index++;
	    if (np->index >= np->size)
		return 0;
	    np->cur = np->list[np->index];
	} else {
	    np->cur++;
	}
	if (!np->read(np->type, np->cur, ptr)) {
	    /* if read fails, fatal */
	    return 0;
	}
	selected = 1;
	switch (np->sel) {
	case NS_LIST:
	    /* The change is to take the player->owner check out here */
	    break;
	case NS_EVERYTHING:
	    /* XXX maybe combine NS_LIST and NS_EVERYTHING later */
	    break;
	case NS_DIST:
	    if (!xyinrange(gp->x, gp->y, &np->range)) {
		selected = 0;
		break;
	    }
	    np->curdist = mapdist((int)gp->x, (int)gp->y,
				  (int)np->cx, (int)np->cy);
	    if (np->curdist > np->dist)
		selected = 0;
	    break;
	case NS_AREA:
	    if (!xyinrange(gp->x, gp->y, &np->range))
		selected = 0;
	    if (gp->x == np->range.hx || gp->y == np->range.hy)
		selected = 0;
	    break;
	case NS_XY:
	    if (gp->x != np->cx || gp->y != np->cy)
		selected = 0;
	    break;
	case NS_GROUP:
	    if (np->group != gp->group)
		selected = 0;
	    break;
	default:
	    CANT_HAPPEN("bad np->sel");
	    return 0;
	}
	if (selected && np->ncond) {
	    /* nstr_exec is expensive, so we do it last */
	    if (!nstr_exec(np->cond, np->ncond, ptr))
		selected = 0;
	}
    } while (!selected);
    return 1;
}

static int
tend_land(struct shpstr *tenderp, s_char *units)
{
    struct nstr_item lni;
    struct nstr_item targets;
    struct shpstr target;
    struct lndstr land;
    struct plnstr plane;
    struct nstr_item pni;
    s_char buf[1024];

    if (!snxtitem(&lni, EF_LAND, units))
	return RET_SYN;

    while (nxtitem(&lni, (s_char *)&land)) {
	if (!player->owner)
	    continue;
	if (land.lnd_ship != tenderp->shp_uid) {
	    pr("%s is not on %s!\n", prland(&land), prship(tenderp));
	    continue;
	}
	if (!(lchr[(int)land.lnd_type].l_flags & L_ASSAULT)) {
	    pr("%s does not have \"assault\" capability and can't be tended\n", prland(&land));
	    continue;
	}
	if (!snxtitem(&targets, EF_SHIP,
		      getstarg(player->argp[4], "Ship to be tended? ",
			       buf)))
	    break;
	if (!check_land_ok(&land))
	    return RET_SYN;
	while (tend_nxtitem(&targets, (s_char *)&target)) {
	    if (!player->owner &&
		(getrel(getnatp(target.shp_own), player->cnum) < FRIENDLY))
		continue;
	    if (target.shp_uid == tenderp->shp_uid)
		continue;
	    if (tenderp->shp_x != target.shp_x ||
		tenderp->shp_y != target.shp_y)
		continue;

	    /* Fit unit on ship */
	    count_units(&target);
	    getship(target.shp_uid, &target);

	    if (target.shp_nland >= mchr[(int)target.shp_type].m_nland) {
		if (mchr[(int)target.shp_type].m_nland)
		    pr("%s doesn't have room for any more land units!\n",
		       prship(&target));
		else
		    pr("%s doesn't carry land units!\n", prship(&target));
		continue;
	    }
	    pr("%s transferred from %s to %s\n",
	       prland(&land), prship(tenderp), prship(&target));
	    sprintf(buf, "loaded on your %s at %s",
		    prship(&target), xyas(target.shp_x, target.shp_y,
					  target.shp_own));
	    gift(target.shp_own, player->cnum, (s_char *)&land,
		 EF_LAND, buf);
	    makelost(EF_LAND, land.lnd_own, land.lnd_uid, land.lnd_x,
		     land.lnd_y);
	    land.lnd_own = target.shp_own;
	    makenotlost(EF_LAND, land.lnd_own, land.lnd_uid, land.lnd_x,
			land.lnd_y);
	    land.lnd_ship = target.shp_uid;
	    land.lnd_harden = 0;
	    land.lnd_mission = 0;
	    target.shp_nland++;
	    putland(land.lnd_uid, &land);
	    expose_ship(tenderp, &target);
	    putship(target.shp_uid, &target);
	    count_units(tenderp);
	    putship(tenderp->shp_uid, tenderp);
	    snxtitem_xy(&pni, EF_PLANE, land.lnd_x, land.lnd_y);
	    while (nxtitem(&pni, (s_char *)&plane)) {
		if (plane.pln_flags & PLN_LAUNCHED)
		    continue;
		if (plane.pln_land != land.lnd_uid)
		    continue;
		sprintf(buf, "loaded on %s", prship(&target));
		gift(target.shp_own, player->cnum, (s_char *)&plane,
		     EF_PLANE, buf);
		makelost(EF_PLANE, plane.pln_own, plane.pln_uid,
			 plane.pln_x, plane.pln_y);
		plane.pln_own = target.shp_own;
		makenotlost(EF_PLANE, plane.pln_own, plane.pln_uid,
			    plane.pln_x, plane.pln_y);
		plane.pln_mission = 0;
		putplane(plane.pln_uid, &plane);
	    }
	}
    }
    return 0;
}
