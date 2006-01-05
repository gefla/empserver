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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  rada.c: Do radar from a ship/unit/sector
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "xy.h"
#include "ship.h"
#include "land.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"

int
rada(void)
{
    s_char *cp;
    double tf;
    double tech;
    struct nstr_item ni;
    struct nstr_sect ns;
    struct shpstr ship;
    struct lndstr land;
    struct sctstr sect;
    int from_unit;
    s_char buf[1024];

    from_unit = (**player->argp == 'l');

    if (!from_unit)
	cp = getstarg(player->argp[1],
		      "Radar from (ship # or sector(s)) : ", buf);
    else
	cp = getstarg(player->argp[1],
		      "Radar from (unit # or sector(s)) : ", buf);
    if (cp == 0)
	return RET_SYN;
    switch (sarg_type(cp)) {
    case NS_AREA:
	if (!snxtsct(&ns, cp))
	    return RET_SYN;
	tech = tfact(player->cnum, 8.0);
	if (tech > ((double)WORLD_Y / 4.0))
	    tech = ((double)WORLD_Y / 4.0);
	if (tech > ((double)WORLD_X / 8.0))
	    tech = ((double)WORLD_X / 8.0);
	while (nxtsct(&ns, &sect)) {
	    if (sect.sct_type != SCT_RADAR)
		continue;
	    if (!player->owner)
		continue;
	    radmap(sect.sct_x, sect.sct_y, (int)sect.sct_effic,
		   (int)(tech * 2.0), 0.0);
	}
	break;
    case NS_LIST:
    case NS_GROUP:
	if (!from_unit) {
	    /* assumes a NS_LIST return is a shipno */
	    if (!snxtitem(&ni, EF_SHIP, cp)) {
		pr("Specify at least one ship\n");
		return RET_SYN;
	    }
	    while (nxtitem(&ni, &ship)) {
		if (!player->owner)
		    continue;
		if (mchr[(int)ship.shp_type].m_flags & M_SONAR)
		    tf = techfact(ship.shp_tech, 1.0);
		else
		    tf = 0.0;
		pr("%s at ", prship(&ship));
		tech = techfact(ship.shp_tech,
				(double)mchr[(int)ship.shp_type].m_vrnge);
		if (tech > ((double)WORLD_Y / 2.0))
		    tech = ((double)WORLD_Y / 2.0);
		if (tech > ((double)WORLD_X / 4.0))
		    tech = ((double)WORLD_X / 4.0);
		radmap(ship.shp_x, ship.shp_y, ship.shp_effic,
		       (int)tech, tf);
	    }
	} else {
	    /* from a land unit */
	    if (!snxtitem(&ni, EF_LAND, cp)) {
		pr("Specify at least one unit\n");
		return RET_SYN;
	    }
	    while (nxtitem(&ni, &land)) {
		if (!player->owner)
		    continue;
		if (!(lchr[(int)land.lnd_type].l_flags & L_RADAR)) {
		    pr("%s can't use radar!\n", prland(&land));
		    continue;
		}
		if (land.lnd_ship >= 0) {
		    pr("Units on ships can't use radar!\n");
		    continue;
		}
		tf = 0.0;
		pr("%s at ", prland(&land));
		tech = techfact(land.lnd_tech, (double)land.lnd_spy);
		if (tech > ((double)WORLD_Y / 2.0))
		    tech = ((double)WORLD_Y / 2.0);
		if (tech > ((double)WORLD_X / 4.0))
		    tech = ((double)WORLD_X / 4.0);
		radmap(land.lnd_x, land.lnd_y, land.lnd_effic,
		       (int)tech, tf);
	    }
	}
	break;
    default:
	if (!from_unit)
	    pr("Must use a ship or sector specifier\n");
	else
	    pr("Must use a unit or sector specifier\n");
	return RET_SYN;
    }
    return RET_OK;
}
