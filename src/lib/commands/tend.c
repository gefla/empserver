/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  tend.c: Transfer goodies from one ship to another.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1992
 *     Steve McClure, 2000
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "land.h"
#include "plague.h"
#include "plane.h"
#include "ship.h"

static void expose_ship(struct shpstr *s1, struct shpstr *s2);
static int tend_land(struct shpstr *tenderp, char *units);

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
    char *p;
    char prompt[512];
    char buf[1024];

    p = getstarg(player->argp[1], "Tend what commodity (or 'land')? ",
		 buf);
    if (!p || !*p)
	return RET_SYN;

    if (!strncmp(p, "land", 4))
	type = EF_LAND;
    else if (NULL != (ip = item_by_name(p)))
	type = EF_SECTOR;
    else {
	pr("Can't tend '%s'\n", p);
	return RET_SYN;
    }

    if (!snxtitem(&tenders, EF_SHIP, player->argp[2], "Tender(s)? "))
	return RET_SYN;

    while (nxtitem(&tenders, &tender)) {
	if (!player->owner)
	    continue;
	if (type == EF_LAND) {
	    sprintf(prompt, "Land unit(s) to tend from %s? ",
		    prship(&tender));
	    p = getstarg(player->argp[3], prompt, buf);
	    if (!p)
		return RET_FAIL;
	    if (!*p)
		continue;
	    if (!check_ship_ok(&tender))
		return RET_SYN;
	    if (0 != (retval = tend_land(&tender, p)))
		return retval;
	    continue;
	}
	sprintf(prompt, "Number of %s to tend from %s? ",
		ip->i_name, prship(&tender));
	p = getstarg(player->argp[3], prompt, buf);
	if (!p)
	    return RET_FAIL;
	if (!*p)
	    continue;
	if (!check_ship_ok(&tender))
	    return RET_SYN;
	if (!(amt = atoi(p))) {
	    pr("Amount must be non-zero!\n");
	    return RET_SYN;
	}
	ontender = tender.shp_item[ip->i_uid];
	if (ontender == 0 && amt > 0) {
	    pr("No %s on %s\n", ip->i_name, prship(&tender));
	    return RET_FAIL;
	}
	vbase = &mchr[(int)tender.shp_type];
	maxtender = vbase->m_item[ip->i_uid];
	if (maxtender == 0) {
	    pr("A %s cannot hold any %s\n",
	       mchr[(int)tender.shp_type].m_name, ip->i_name);
	    break;
	}
	if (!snxtitem(&targets, EF_SHIP,
		      player->argp[4], "Ships to be tended? "))
	    return RET_FAIL;
	if (!check_ship_ok(&tender))
	    return RET_SYN;
	total = 0;
	while (nxtitem(&targets, &target)) {
	    if (!player->owner &&
		(getrel(getnatp(target.shp_own), player->cnum) < FRIENDLY))
		continue;
	    if (target.shp_uid == tender.shp_uid)
		continue;
	    if (tender.shp_x != target.shp_x ||
		tender.shp_y != target.shp_y)
		continue;
	    ontarget = target.shp_item[ip->i_uid];
	    if (ontarget == 0 && amt < 0) {
		pr("No %s on %s\n", ip->i_name, prship(&target));
		continue;
	    }
	    vbase = &mchr[(int)target.shp_type];
	    maxtarget = vbase->m_item[ip->i_uid];
	    if (amt < 0) {
		if (!player->owner)
		    amt = 0;

		/* take from target and give to tender */
		transfer = MIN(ontarget, -amt);
		transfer = MIN(maxtender - ontender, transfer);
		if (transfer == 0)
		    continue;
		target.shp_item[ip->i_uid] = ontarget - transfer;
		ontender += transfer;
		total += transfer;
	    } else {
		/* give to target from tender */
		transfer = MIN(ontender, amt);
		transfer = MIN(transfer, maxtarget - ontarget);
		if (transfer == 0)
		    continue;
		target.shp_item[ip->i_uid] = ontarget + transfer;
		ontender -= transfer;
		total += transfer;
		if (transfer && target.shp_own != player->cnum) {
		    wu(0, target.shp_own, "%s tended %d %s to %s\n",
		       cname(player->cnum), total, ip->i_name,
		       prship(&target));
		}
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
	tender.shp_item[ip->i_uid] = ontender;
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

static int
tend_land(struct shpstr *tenderp, char *units)
{
    struct nstr_item lni;
    struct nstr_item targets;
    struct shpstr target;
    struct lndstr land;
    char buf[1024];

    if (!snxtitem(&lni, EF_LAND, units, NULL))
	return RET_SYN;

    while (nxtitem(&lni, &land)) {
	if (!player->owner)
	    continue;
	if (land.lnd_ship != tenderp->shp_uid) {
	    pr("%s is not on %s!\n", prland(&land), prship(tenderp));
	    continue;
	}
	if (!(lchr[(int)land.lnd_type].l_flags & L_ASSAULT)) {
	    pr("%s does not have \"assault\" capability and can't be tended\n",
	       prland(&land));
	    continue;
	}
	if (!snxtitem(&targets, EF_SHIP,
		      player->argp[4], "Ship to be tended? "))
	    return RET_FAIL;
	if (!check_ship_ok(tenderp) || !check_land_ok(&land))
	    return RET_SYN;
	while (nxtitem(&targets, &target)) {
	    if (!player->owner &&
		(getrel(getnatp(target.shp_own), player->cnum) < FRIENDLY))
		continue;
	    if (target.shp_uid == tenderp->shp_uid)
		continue;
	    if (tenderp->shp_x != target.shp_x ||
		tenderp->shp_y != target.shp_y)
		continue;

	    /* Fit unit on ship */
	    getship(target.shp_uid, &target);

	    if ((!(lchr[(int)land.lnd_type].l_flags & L_LIGHT)) &&
		(!((mchr[(int)target.shp_type].m_flags & M_SUPPLY) &&
		   (!(mchr[(int)target.shp_type].m_flags & M_SUB))))) {
		pr("You can only load light units onto ships,\n"
		   "unless the ship is a non-sub supply ship\n"
		   "%s not tended\n", prland(&land));
		continue;
	    }

	    if ((mchr[(int)target.shp_type].m_flags & M_SUB) &&
		(lchr[(int)land.lnd_type].l_flags & L_SPY) &&
		!mchr[(int)target.shp_type].m_nland) {
		if (shp_nland(&target) > 1) {
		    pr("%s doesn't have room for more than two spy units!\n",
		       prship(&target));
		    continue;
		}
	    } else if (shp_nland(&target) >= mchr[target.shp_type].m_nland) {
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
	    gift(target.shp_own, player->cnum, &land, buf);
	    land.lnd_ship = target.shp_uid;
	    land.lnd_harden = 0;
	    putland(land.lnd_uid, &land);
	    expose_ship(tenderp, &target);
	    putship(target.shp_uid, &target);
	    putship(tenderp->shp_uid, tenderp);
	}
    }
    return 0;
}
