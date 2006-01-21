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
 *  fuel.c: fuel ships/land units
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "plague.h"
#include "xy.h"
#include "ship.h"
#include "plane.h"
#include "land.h"
#include "nat.h"
#include "nsc.h"
#include "file.h"
#include "sect.h"
#include "optlist.h"
#include "commands.h"


union item_u {
    struct shpstr ship;
    struct lndstr land;
};

int
fuel(void)
{
    static int shp_or_lnd[] = { EF_SHIP, EF_LAND, EF_BAD };
    struct nstr_item ni;
    union item_u item, item2;
    int type;
    struct mchrstr *mp;
    struct lchrstr *lcp;
    s_char *p;
    int fueled;
    int land_fuel, ship_fuel;
    int oil_amt, pet_amt, fuel_amt, tot_fuel, max_amt;
    int move_amt;
    float extra;
    struct sctstr sect;
    struct natstr *natp;
    int harbor, sector;
    int fuelled_ship = -1;
    struct nstr_item tender, ltender;
    s_char prompt[128];
    s_char buf[1024];

    if (opt_FUEL == 0) {
	pr("Option 'FUEL' not enabled\n");
	return RET_SYN;
    }
    if ((p =
	 getstarg(player->argp[1], "Ship or land unit (s,l)? ", buf)) == 0)
	return RET_SYN;
    type = ef_byname_from(p, shp_or_lnd);
    if (type < 0) {
	pr("Ships or land units only! (s, l)\n");
	return RET_SYN;
    }
    sprintf(prompt, "%s(s)? ", ef_nameof(type));
    p = getstarg(player->argp[2], prompt, buf);
    if (!snxtitem(&ni, type, p))
	return RET_SYN;
    if (isdigit(*p))
	fuelled_ship = atoi(p);
    p = getstarg(player->argp[3], "Amount: ", buf);
    if (p == 0 || *p == 0)
	return RET_SYN;
    fuel_amt = atoi(p);
    if (fuel_amt <= 0) {
	pr("Fuel amount must be positive!\n");
	return RET_FAIL;
    }

    while (nxtitem(&ni, &item)) {
	fueled = 0;
	if (type == EF_SHIP) {
	    if (item.ship.shp_own != player->cnum) {
		int rel;

		if (item.ship.shp_uid != fuelled_ship)
		    continue;
		natp = getnatp(player->cnum);
		rel = getrel(natp, item.ship.shp_own);
		if (rel < FRIENDLY)
		    continue;
	    }
	    if (!getsect(item.ship.shp_x, item.ship.shp_y, &sect))
		continue;
	    if (!item.ship.shp_own)
		continue;

	    if ((sect.sct_type != SCT_HARBR)
		&& (sect.sct_type != SCT_WATER)
		&& (sect.sct_type != SCT_BSPAN)
		&& (!IS_BIG_CITY(sect.sct_type))) {
		pr("Sector %s is not a harbor, bridge span, or sea.\n",
		   xyas(item.ship.shp_x, item.ship.shp_y,
			item.ship.shp_own));
		continue;
	    }

	    mp = &mchr[(int)item.ship.shp_type];

	    harbor = 0;
	    if (sect_has_dock(&sect)) {
		harbor = 1;
		oil_amt = sect.sct_item[I_OIL];
		pet_amt = sect.sct_item[I_PETROL];
		if ((oil_amt + pet_amt) == 0)
		    harbor = 0;

		if (sect.sct_effic < 2) {
		    pr("The harbor at %s is not 2%% efficient yet.\n",
		       xyas(item.ship.shp_x,
			    item.ship.shp_y, player->cnum));
		    harbor = 0;
		}
		if ((sect.sct_own != player->cnum) && sect.sct_own)
		    harbor = 0;
	    }

	    if ((mp->m_fuelu == 0) && (item.ship.shp_own == player->cnum)) {
		pr("%s does not use fuel!\n", prship(&item.ship));
		continue;
	    }

	    if (harbor) {
		ship_fuel = item.ship.shp_fuel;
		oil_amt = sect.sct_item[I_OIL];
		pet_amt = sect.sct_item[I_PETROL];
		max_amt = mp->m_fuelc - ship_fuel;

		if (max_amt == 0) {
		    pr("%s already has a full fuel load.\n",
		       prship(&item.ship));
		    continue;
		}
		tot_fuel = (oil_amt * 50 + pet_amt * 5);
		if (tot_fuel == 0) {
		    pr("No fuel in the harbor at %s!\n",
		       xyas(sect.sct_x, sect.sct_y, player->cnum));
		    continue;
		}
		move_amt = MIN(tot_fuel, fuel_amt);
		move_amt = MIN(move_amt, max_amt);

		if (move_amt == 0)
		    continue;

		item.ship.shp_fuel += move_amt;

		fueled = 1;
		if ((pet_amt * 5) >= move_amt) {
		    extra = ((float)move_amt / 5.0) - (move_amt / 5);
		    if (extra > 0.0)
			sect.sct_item[I_PETROL]
			    = MAX((pet_amt - move_amt / 5) - 1, 0);
		    else
			sect.sct_item[I_PETROL]
			    = MAX((pet_amt - move_amt / 5), 0);
		} else {
		    sect.sct_item[I_PETROL] = 0;
		    move_amt -= pet_amt * 5;
		    extra = ((float)move_amt / 50.0) - (move_amt / 50);
		    sect.sct_item[I_OIL] = MAX(oil_amt - move_amt / 50, 0);
		    if (extra > 0.0)
			sect.sct_item[I_OIL]
			    = MAX((oil_amt - move_amt / 50) - 1, 0);
		    else
			sect.sct_item[I_OIL]
			    = MAX((oil_amt - move_amt / 50), 0);
		}

		/* load plague */
		if (sect.sct_pstage == PLG_INFECT
		    && item.ship.shp_pstage == PLG_HEALTHY)
		    item.ship.shp_pstage = PLG_EXPOSED;

		putsect(&sect);
		putship(item.ship.shp_uid, &item.ship);
	    } else {		/* not in a harbor */
		if (!player->argp[4])
		    pr("%s is not in a supplied, efficient harbor\n",
		       prship(&item.ship));
		if (!snxtitem (&tender, EF_SHIP,
			       getstarg(player->argp[4], "Oiler? ", buf)))
		    continue;

		if (!check_ship_ok(&item.ship))
		    continue;

		if (!nxtitem(&tender, &item2))
		    continue;

		if (!(mchr[(int)item2.ship.shp_type].m_flags & M_OILER)) {
		    pr("%s is not an oiler!\n", prship(&item2.ship));
		    continue;
		}
		if (item2.ship.shp_own != player->cnum) {
		    pr("You don't own that oiler!\n");
		    continue;
		}

		if ((item2.ship.shp_x != item.ship.shp_x) ||
		    (item2.ship.shp_y != item.ship.shp_y)) {
		    pr("Not in the same sector!\n");
		    continue;
		}
		ship_fuel = item.ship.shp_fuel;
		oil_amt = item2.ship.shp_item[I_OIL];
		pet_amt = item2.ship.shp_item[I_PETROL];
		max_amt = mp->m_fuelc - ship_fuel;

		if (max_amt == 0) {
		    pr("%s already has a full fuel load.\n",
		       prship(&item.ship));
		    continue;
		}
		tot_fuel = oil_amt * 50 + pet_amt * 5;
		move_amt = MIN(tot_fuel, fuel_amt);
		move_amt = MIN(move_amt, max_amt);

		if (move_amt == 0)
		    continue;

		item.ship.shp_fuel += move_amt;

		fueled = 1;
		if ((pet_amt * 5) >= move_amt) {
		    extra = ((float)move_amt / 5.0) - (move_amt / 5);
		    if (extra > 0.0)
			item2.ship.shp_item[I_PETROL]
			    = MAX((pet_amt - move_amt / 5) - 1, 0);
		    else
			item2.ship.shp_item[I_PETROL]
			    = MAX((pet_amt - move_amt / 5), 0);
		} else {
		    item2.ship.shp_item[I_PETROL] = 0;
		    move_amt -= pet_amt * 5;
		    extra = ((float)move_amt / 50.0) - (move_amt / 50);
		    item2.ship.shp_item[I_OIL]
			= MAX(oil_amt - (move_amt / 50), 0);
		    if (extra > 0.0)
			item2.ship.shp_item[I_OIL]
			    = MAX((oil_amt - move_amt / 50) - 1, 0);
		    else
			item2.ship.shp_item[I_OIL]
			    = MAX((oil_amt - move_amt / 50), 0);
		}

		/* load plague */
		if (item2.ship.shp_pstage == PLG_INFECT
		    && item.ship.shp_pstage == PLG_HEALTHY)
		    item.ship.shp_pstage = PLG_EXPOSED;

		putship(item.ship.shp_uid, &item.ship);
		/* quick hack -KHS */
		if (item.ship.shp_uid == item2.ship.shp_uid)
		    item2.ship.shp_fuel = item.ship.shp_fuel;
		putship(item2.ship.shp_uid, &item2.ship);
	    }
	    pr("%s", prship(&item.ship));
	} else {
	    if (item.land.lnd_own != player->cnum)
		continue;

	    if (!getsect(item.land.lnd_x, item.land.lnd_y, &sect))
		continue;

	    if (!player->owner)
		continue;

	    lcp = &lchr[(int)item.land.lnd_type];

	    sector = 1;
	    oil_amt = sect.sct_item[I_OIL];
	    pet_amt = sect.sct_item[I_PETROL];

	    if ((oil_amt + pet_amt) == 0)
		sector = 0;

	    if ((item.land.lnd_fuelu == 0)
		&& (item.land.lnd_own == player->cnum)) {
		pr("%s does not use fuel!\n", prland(&item.land));
		continue;
	    }

	    if (sector) {
		land_fuel = item.land.lnd_fuel;
		oil_amt = sect.sct_item[I_OIL];
		pet_amt = sect.sct_item[I_PETROL];
		max_amt = item.land.lnd_fuelc - land_fuel;

		if (max_amt == 0) {
		    pr("%s already has a full fuel load.\n",
		       prland(&item.land));
		    continue;
		}
		tot_fuel = (oil_amt * 50 + pet_amt * 5);
		if (tot_fuel == 0) {
		    pr("No fuel in the sector at %s!\n",
		       xyas(sect.sct_x, sect.sct_y, player->cnum));
		    continue;
		}
		move_amt = MIN(tot_fuel, fuel_amt);
		move_amt = MIN(move_amt, max_amt);

		if (move_amt == 0)
		    continue;

		item.land.lnd_fuel += move_amt;

		fueled = 1;
		if ((pet_amt * 5) >= move_amt) {
		    extra = ((float)move_amt / 5.0) - (move_amt / 5);
		    if (extra > 0.0)
			sect.sct_item[I_PETROL]
			    = MAX((pet_amt - move_amt / 5) - 1, 0);
		    else
			sect.sct_item[I_PETROL]
			    = MAX((pet_amt - move_amt / 5), 0);
		} else {
		    sect.sct_item[I_PETROL] = 0;
		    move_amt -= pet_amt * 5;
		    extra = ((float)move_amt / 50.0) - (move_amt / 50);
		    sect.sct_item[I_OIL] = MAX(oil_amt - move_amt / 50, 0);
		    if (extra > 0.0)
			sect.sct_item[I_OIL]
			    = MAX((oil_amt - move_amt / 50) - 1, 0);
		    else
			sect.sct_item[I_OIL]
			    = MAX((oil_amt - move_amt / 50), 0);
		}

		/* load plague */
		if (sect.sct_pstage == PLG_INFECT
		    && item.land.lnd_pstage == PLG_HEALTHY)
		    item.land.lnd_pstage = PLG_EXPOSED;

		putsect(&sect);
		putland(item.land.lnd_uid, &item.land);
	    } else {		/* not in a sector */
		if (!player->argp[4])
		    pr("%s is not in a supplied sector\n",
		       prland(&item.land));
		if (!snxtitem(&ltender, EF_LAND,
			      getstarg(player->argp[4], "Supply unit? ",
				       buf)))
		    continue;

		if (!check_land_ok(&item.land))
		    continue;

		if (!nxtitem(&ltender, &item2))
		    continue;

		if (!(lchr[(int)item2.land.lnd_type].l_flags & L_SUPPLY)) {
		    pr("%s is not a supply unit!\n", prland(&item2.land));
		    continue;
		}
		if (item2.land.lnd_own != player->cnum) {
		    pr("You don't own that unit!\n");
		    continue;
		}

		if ((item2.land.lnd_x != item.land.lnd_x) ||
		    (item2.land.lnd_y != item.land.lnd_y)) {
		    pr("Not in the same sector!\n");
		    continue;
		}
		land_fuel = item.land.lnd_fuel;
		oil_amt = item2.land.lnd_item[I_OIL];
		pet_amt = item2.land.lnd_item[I_PETROL];
		max_amt = item.land.lnd_fuelc - land_fuel;

		if (max_amt == 0) {
		    pr("%s already has a full fuel load.\n",
		       prland(&item.land));
		    continue;
		}
		tot_fuel = oil_amt * 50 + pet_amt * 5;
		move_amt = MIN(tot_fuel, fuel_amt);
		move_amt = MIN(move_amt, max_amt);

		if (move_amt == 0)
		    continue;

		item.land.lnd_fuel += move_amt;

		fueled = 1;
		if ((pet_amt * 5) >= move_amt) {
		    extra = ((float)move_amt / 5.0) - (move_amt / 5);
		    if (extra > 0.0)
			item2.land.lnd_item[I_PETROL]
			    = MAX((pet_amt - move_amt / 5) - 1, 0);
		    else
			item2.land.lnd_item[I_PETROL]
			    = MAX((pet_amt - move_amt / 5), 0);
		} else {
		    item2.land.lnd_item[I_PETROL] = 0;
		    move_amt -= pet_amt * 5;
		    extra = ((float)move_amt / 50.0) - (move_amt / 50);
		    item2.land.lnd_item[I_OIL]
			= MAX(oil_amt - move_amt / 50, 0);
		    if (extra > 0.0)
			item2.land.lnd_item[I_OIL]
			    = MAX((oil_amt - move_amt / 50) - 1, 0);
		    else
			item2.land.lnd_item[I_OIL]
			    = MAX((oil_amt - move_amt / 50), 0);
		}

		/* load plague */
		if (item2.land.lnd_pstage == PLG_INFECT
		    && item.land.lnd_pstage == PLG_HEALTHY)
		    item.land.lnd_pstage = PLG_EXPOSED;

		putland(item.land.lnd_uid, &item.land);
		/* quick hack -KHS */
		if (item2.land.lnd_uid == item.land.lnd_uid)
		    item2.land.lnd_fuel = item.land.lnd_fuel;
		putland(item2.land.lnd_uid, &item2.land);
	    }
	    pr("%s", prland(&item.land));
	}
	if (fueled) {
	    pr(" takes on %d fuel in %s\n",
	       move_amt,
	       xyas(item.ship.shp_x, item.ship.shp_y, player->cnum));
	    if (player->cnum != item.ship.shp_own)
		wu(0, item.ship.shp_own,
		   "%s takes on %d fuel in %s courtesy of %s\n",
		   prship(&item.ship),
		   move_amt,
		   xyas(item.ship.shp_x, item.ship.shp_y,
			item.ship.shp_own), cname(player->cnum));
	}
    }
    return RET_OK;
}
