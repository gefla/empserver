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
 *  rada.c: Do radar from a ship/unit/sector
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2006
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"
#include "empobj.h"

int
rada(void)
{
    return radar(EF_SHIP);
}

int
lrad(void)
{
    return radar(EF_LAND);
}

int
radar(int type)
{
    char *cp;
    double tf;
    double tlev;
    int spy;
    struct nstr_item ni;
    struct nstr_sect ns;
    union empobj_storage item;
    char buf[1024];
    char prompt[80];

    if (CANT_HAPPEN(type != EF_LAND && type != EF_SHIP))
	type = EF_SHIP;

    sprintf(prompt, "Radar from (%s # or sector(s)) : ", ef_nameof(type));
    cp = getstarg(player->argp[1], prompt, buf);

    if (!cp)
	return RET_SYN;
    switch (sarg_type(cp)) {
    case NS_AREA:
	if (!snxtsct(&ns, cp))
	    return RET_SYN;
	tlev = getnatp(player->cnum)->nat_level[NAT_TLEV];
	while (nxtsct(&ns, &item.sect)) {
	    if (item.sect.sct_type != SCT_RADAR)
		continue;
	    if (!player->owner)
		continue;
	    radmap(item.sect.sct_x, item.sect.sct_y, item.sect.sct_effic,
		   tlev, 16, 0.0);
	}
	break;
    case NS_LIST:
    case NS_GROUP:
	/* assumes a NS_LIST return is a unit no */
	if (!snxtitem(&ni, type, cp, NULL)) {
	    pr("Specify at least one %s\n", ef_nameof(type));
	    return RET_SYN;
	}
	while (nxtitem(&ni, &item)) {
	    if (!player->owner)
		continue;
	    tf = 0.0;
	    if (type == EF_SHIP) {
		if (mchr[(int)item.ship.shp_type].m_flags & M_SONAR)
		    tf = techfact(item.ship.shp_tech, 1.0);
		spy = mchr[item.ship.shp_type].m_vrnge;
	    } else {
		if (!(lchr[(int)item.land.lnd_type].l_flags & L_RADAR)) {
		    pr("%s can't use radar!\n", prland(&item.land));
		    continue;
		}
		if (item.land.lnd_ship >= 0) {
		    pr("Units on ships can't use radar!\n");
		    continue;
		}
		spy = lchr[item.land.lnd_type].l_spy;
	    }

	    pr("%s at ", obj_nameof(&item.gen));
	    radmap(item.gen.x, item.gen.y, item.gen.effic,
		   item.gen.tech, spy, tf);
	}
	break;
    default:
	pr("Must use a %s or sector specifier\n", ef_nameof(type));
	return RET_SYN;
    }
    return RET_OK;
}
