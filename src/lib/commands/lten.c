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
 *  lten.c: Transfer commodity from a ship to a land unit the ship is
 *          carrying
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "land.h"
#include "plague.h"
#include "plane.h"
#include "ship.h"

static void expose_land(struct shpstr *s1, struct lndstr *l1);

int
ltend(void)
{
    struct nstr_item targets;
    struct nstr_item tenders;
    struct shpstr tender;
    struct lndstr target;
    struct ichrstr *ip;
    struct mchrstr *vbase;
    struct lchrstr *lbase;
    int amt;
    int ontender;
    int ontarget;
    int maxtender;
    int maxtarget;
    int transfer;
    int total;
    char *p;
    char buf[1024];

    if (!(ip = whatitem(player->argp[1], "Transfer what commodity? ")))
	return RET_SYN;

    if (!snxtitem(&tenders, EF_SHIP, player->argp[2], "Tender(s)? "))
	return RET_SYN;
    while (nxtitem(&tenders, &tender)) {
	if (!player->owner)
	    continue;
	if ((p =
	     getstarg(player->argp[3], "Amount to transfer? ", buf)) == 0)
	    return RET_SYN;
	if (!check_ship_ok(&tender))
	    return RET_FAIL;
	if ((amt = atoi(p)) == 0)
	    break;
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
	    return RET_FAIL;
	}
	if (!snxtitem(&targets, EF_LAND,
		      player->argp[4], "Units to be tended? "))
	    return RET_SYN;
	if (!check_ship_ok(&tender))
	    return RET_FAIL;
	total = 0;
	while (nxtitem(&targets, &target)) {
	    if (!player->owner)
		continue;

	    if (target.lnd_ship != tender.shp_uid)
		continue;
	    ontarget = target.lnd_item[ip->i_uid];
	    if (ontarget == 0 && amt < 0) {
		pr("No %s on %s\n",
		   ip->i_name, prland(&target));
		continue;
	    }
	    lbase = &lchr[(int)target.lnd_type];
	    maxtarget = lbase->l_item[ip->i_uid];
	    if (amt < 0) {
		if (!player->owner)
		    amt = 0;

		/* take from target and give to tender */
		transfer = MIN(ontarget, -amt);
		transfer = MIN(maxtender - ontender, transfer);
		if (transfer == 0)
		    continue;
		target.lnd_item[ip->i_uid] = ontarget - transfer;
		ontender += transfer;
		total += transfer;
	    } else {
		/* give to target from tender */
		transfer = MIN(ontender, amt);
		transfer = MIN(transfer, maxtarget - ontarget);
		if (transfer == 0)
		    continue;
		target.lnd_item[ip->i_uid] = ontarget + transfer;
		ontender -= transfer;
		total += transfer;
	    }
	    expose_land(&tender, &target);
	    putland(target.lnd_uid, &target);
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
expose_land(struct shpstr *s1, struct lndstr *l1)
{
    if (s1->shp_pstage == PLG_INFECT && l1->lnd_pstage == PLG_HEALTHY)
	l1->lnd_pstage = PLG_EXPOSED;
    if (l1->lnd_pstage == PLG_INFECT && s1->shp_pstage == PLG_HEALTHY)
	s1->shp_pstage = PLG_EXPOSED;
}
