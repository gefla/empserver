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
 *  lten.c: Transfer commodity from a ship to a land unit the ship is
 *          carrying
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "file.h"
#include "ship.h"
#include "item.h"
#include "nsc.h"
#include "nat.h"
#include "plane.h"
#include "nuke.h"
#include "land.h"
#include "genitem.h"
#include "commands.h"

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
    s_char *p;
    s_char buf[1024];

    if (!(ip = whatitem(player->argp[1], "Transfer what commodity? ")))
	return RET_SYN;

    if (!snxtitem
	(&tenders, EF_SHIP, getstarg(player->argp[2], "Tender(s)? ", buf)))
	return RET_SYN;
    while (nxtitem(&tenders, (s_char *)&tender)) {
	if (!player->owner)
	    continue;
	if ((p =
	     getstarg(player->argp[3], "Amount to transfer? ", buf)) == 0)
	    break;
	if (!check_ship_ok(&tender))
	    return RET_FAIL;
	if ((amt = atoi(p)) == 0)
	    break;
	ontender = getvar(ip->i_vtype, (s_char *)&tender, EF_SHIP);
	if (ontender == 0 && amt > 0) {
	    pr("No %s on %s\n", ip->i_name, prship(&tender));
	    return RET_FAIL;
	}
	vbase = &mchr[(int)tender.shp_type];
	maxtender = vl_find(ip->i_vtype, vbase->m_vtype,
			    vbase->m_vamt, (int)vbase->m_nv);
	if (maxtender == 0) {
	    pr("A %s cannot hold any %s\n",
	       mchr[(int)tender.shp_type].m_name, ip->i_name);
	    break;
	}
	if (!snxtitem(&targets, EF_LAND,
		      getstarg(player->argp[4], "Units to be tended? ",
			       buf)))
	    break;
	if (!check_ship_ok(&tender))
	    return RET_FAIL;
	total = 0;
	while (tend_nxtitem(&targets, (s_char *)&target)) {
	    if (!player->owner)
		continue;

	    if (target.lnd_ship != tender.shp_uid)
		continue;
	    ontarget = getvar(ip->i_vtype, (s_char *)&target, EF_LAND);
	    if (ontarget == 0 && amt < 0) {
		pr("No %s on %s\n",
		   ip->i_name, prship((struct shpstr *)&target));
		continue;
	    }
	    lbase = &lchr[(int)target.lnd_type];
	    maxtarget = vl_find(ip->i_vtype, lbase->l_vtype,
				lbase->l_vamt, (int)lbase->l_nv);
	    if (amt < 0) {
		if (!player->owner)
		    amt = 0;

		/* take from target and give to tender */
		transfer = min(ontarget, -amt);
		transfer = min(maxtender - ontender, transfer);
		if (transfer == 0)
		    continue;
		putvar(ip->i_vtype, ontarget - transfer,
		       (s_char *)&target, EF_LAND);
		ontender += transfer;
		total += transfer;
	    } else {
		/* give to target from tender */
		transfer = min(ontender, amt);
		transfer = min(transfer, maxtarget - ontarget);
		if (transfer == 0)
		    continue;
		putvar(ip->i_vtype, ontarget + transfer,
		       (s_char *)&target, EF_LAND);
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
	putvar(ip->i_vtype, ontender, (s_char *)&tender, EF_SHIP);
	tender.shp_mission = 0;
	putship(tender.shp_uid, &tender);
    }
    return RET_OK;
}

static void
expose_land(struct shpstr *s1, struct lndstr *l1)
{
    if (getvar(V_PSTAGE, (s_char *)s1, EF_SHIP) == PLG_INFECT &&
	getvar(V_PSTAGE, (s_char *)l1, EF_LAND) == PLG_HEALTHY)
	putvar(V_PSTAGE, PLG_EXPOSED, (s_char *)l1, EF_LAND);
    if (getvar(V_PSTAGE, (s_char *)l1, EF_LAND) == PLG_INFECT &&
	getvar(V_PSTAGE, (s_char *)s1, EF_SHIP) == PLG_HEALTHY)
	putvar(V_PSTAGE, PLG_EXPOSED, (s_char *)s1, EF_SHIP);
}
