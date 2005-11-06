/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ldump.c: Dump land unit information
 * 
 *  Known contributors to this file:
 *     John Yockey, 1997
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "land.h"
#include "nat.h"
#include "nsc.h"
#include "file.h"
#include "optlist.h"
#include "commands.h"

int
ldump(void)
{
    int nunits;
    int field[128];
    struct nstr_item ni;
    struct lndstr land;
    int n, i;
    struct natstr *np;
    time_t now;

    if (!snxtitem(&ni, EF_LAND, player->argp[1]))
	return RET_SYN;
    prdate();
    nunits = 0;

    if (!player->argp[2]) {
	for (n = 1; n <= 39; n++)
	    field[n - 1] = n;
	field[n - 1] = 0;
    } else {
	n = 2;
	i = 0;
	while (player->argp[n]) {
	    if (!strcmp("type", player->argp[n])) {
		field[i++] = 1;
	    } else if (!strcmp("x", player->argp[n])) {
		field[i++] = 2;
	    } else if (!strcmp("y", player->argp[n])) {
		field[i++] = 3;
	    } else if (!strcmp("army", player->argp[n])) {
		field[i++] = 4;
	    } else if (!strcmp("eff", player->argp[n])) {
		field[i++] = 5;
	    } else if (!strcmp("mil", player->argp[n])) {
		field[i++] = 6;
	    } else if (!strcmp("fort", player->argp[n])) {
		field[i++] = 7;
	    } else if (!strcmp("mob", player->argp[n])) {
		field[i++] = 8;
	    } else if (!strcmp("food", player->argp[n])) {
		field[i++] = 9;
	    } else if (!strcmp("fuel", player->argp[n])) {
		field[i++] = 10;
	    } else if (!strcmp("tech", player->argp[n])) {
		field[i++] = 11;
	    } else if (!strcmp("retr", player->argp[n])) {
		field[i++] = 12;
	    } else if (!strcmp("react", player->argp[n])) {
		field[i++] = 13;
	    } else if (!strcmp("xl", player->argp[n])) {
		field[i++] = 14;
	    } else if (!strcmp("nland", player->argp[n])) {
		field[i++] = 15;
	    } else if (!strcmp("land", player->argp[n])) {
		field[i++] = 16;
	    } else if (!strcmp("ship", player->argp[n])) {
		field[i++] = 17;
	    } else if (!strcmp("shell", player->argp[n])) {
		field[i++] = 18;
	    } else if (!strcmp("gun", player->argp[n])) {
		field[i++] = 19;
	    } else if (!strcmp("petrol", player->argp[n])) {
		field[i++] = 20;
	    } else if (!strcmp("iron", player->argp[n])) {
		field[i++] = 21;
	    } else if (!strcmp("dust", player->argp[n])) {
		field[i++] = 22;
	    } else if (!strcmp("bar", player->argp[n])) {
		field[i++] = 23;
	    } else if (!strcmp("oil", player->argp[n])) {
		field[i++] = 24;
	    } else if (!strcmp("lcm", player->argp[n])) {
		field[i++] = 25;
	    } else if (!strcmp("hcm", player->argp[n])) {
		field[i++] = 26;
	    } else if (!strcmp("rad", player->argp[n])) {
		field[i++] = 27;
	    } else if (!strcmp("att", player->argp[n])) {
		field[i++] = 28;
	    } else if (!strcmp("def", player->argp[n])) {
		field[i++] = 29;
	    } else if (!strcmp("vul", player->argp[n])) {
		field[i++] = 30;
	    } else if (!strcmp("spd", player->argp[n])) {
		field[i++] = 31;
	    } else if (!strcmp("vis", player->argp[n])) {
		field[i++] = 32;
	    } else if (!strcmp("spy", player->argp[n])) {
		field[i++] = 33;
	    } else if (!strcmp("radius", player->argp[n])) {
		field[i++] = 34;
	    } else if (!strcmp("frg", player->argp[n])) {
		field[i++] = 35;
	    } else if (!strcmp("acc", player->argp[n])) {
		field[i++] = 36;
	    } else if (!strcmp("dam", player->argp[n])) {
		field[i++] = 37;
	    } else if (!strcmp("amm", player->argp[n])) {
		field[i++] = 38;
	    } else if (!strcmp("aaf", player->argp[n])) {
		field[i++] = 39;
	    } else {
		pr("Unrecognized field %s\n", player->argp[n]);
	    }
	    if (n++ > 100) {
		pr("Too many fields\n");
		return RET_FAIL;
	    }
	}
	field[i] = 0;
    }

    if (player->god)
	pr("   ");
    time(&now);
    pr("DUMP LAND UNITS %ld\n", (long)now);
    if (player->god)
	pr("own ");
    pr("id");
    n = 0;
    while (field[n]) {
	switch (field[n]) {
	case 1:
	    pr(" type");
	    break;
	case 2:
	    pr(" x");
	    break;
	case 3:
	    pr(" y");
	    break;
	case 4:
	    pr(" army");
	    break;
	case 5:
	    pr(" eff");
	    break;
	case 6:
	    pr(" mil");
	    break;
	case 7:
	    pr(" fort");
	    break;
	case 8:
	    pr(" mob");
	    break;
	case 9:
	    pr(" food");
	    break;
	case 10:
	    pr(" fuel");
	    break;
	case 11:
	    pr(" tech");
	    break;
	case 12:
	    pr(" retr");
	    break;
	case 13:
	    pr(" react");
	    break;
	case 14:
	    pr(" xl");
	    break;
	case 15:
	    pr(" nland");
	    break;
	case 16:
	    pr(" land");
	    break;
	case 17:
	    pr(" ship");
	    break;
	case 18:
	    pr(" shell");
	    break;
	case 19:
	    pr(" gun");
	    break;
	case 20:
	    pr(" petrol");
	    break;
	case 21:
	    pr(" iron");
	    break;
	case 22:
	    pr(" dust");
	    break;
	case 23:
	    pr(" bar");
	    break;
	case 24:
	    pr(" oil");
	    break;
	case 25:
	    pr(" lcm");
	    break;
	case 26:
	    pr(" hcm");
	    break;
	case 27:
	    pr(" rad");
	    break;
	case 28:
	    pr(" att");
	    break;
	case 29:
	    pr(" def");
	    break;
	case 30:
	    pr(" vul");
	    break;
	case 31:
	    pr(" spd");
	    break;
	case 32:
	    pr(" vis");
	    break;
	case 33:
	    pr(" spy");
	    break;
	case 34:
	    pr(" radius");
	    break;
	case 35:
	    pr(" frg");
	    break;
	case 36:
	    pr(" acc");
	    break;
	case 37:
	    pr(" dam");
	    break;
	case 38:
	    pr(" amm");
	    break;
	case 39:
	    pr(" aaf");
	    break;
	}
	n++;
    }
    pr("\n");

    np = getnatp(player->cnum);
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == 0)
	    continue;
	if (!player->owner && !player->god)
	    continue;
	count_land_planes(&land);
	lnd_count_units(&land);

	nunits++;
	if (player->god)
	    pr("%d ", land.lnd_own);
	pr("%d", ni.cur);
	n = 0;
	while (field[n]) {
	    switch (field[n++]) {
	    case 1:
		pr(" %.4s", lchr[(int)land.lnd_type].l_name);
		break;
	    case 2:
		pr(" %d", xrel(np, land.lnd_x));
		break;
	    case 3:
		pr(" %d", yrel(np, land.lnd_y));
		break;
	    case 4:
		if (land.lnd_army == ' ')
		    pr(" ~");
		else
		    pr(" %c", land.lnd_army);
		break;
	    case 5:
		pr(" %d", land.lnd_effic);
		break;
	    case 6:
		pr(" %d", lnd_getmil(&land));
		break;
	    case 7:
		pr(" %d", land.lnd_harden);
		break;
	    case 8:
		pr(" %d", land.lnd_mobil);
		break;
	    case 9:
		pr(" %d", land.lnd_item[I_FOOD]);
		break;
	    case 10:
		if (opt_FUEL)
		    pr(" %d", land.lnd_fuel);
		else
		    pr(" 0");
		break;
	    case 11:
		pr(" %d", land.lnd_tech);
		break;
	    case 12:
		pr(" %d", land.lnd_retreat);
		break;
	    case 13:
		pr(" %d", land.lnd_rad_max);
		break;
	    case 14:
		pr(" %d", land.lnd_nxlight);
		break;
	    case 15:
		pr(" %d", land.lnd_nland);
		break;
	    case 16:
		pr(" %d", land.lnd_land);
		break;
	    case 17:
		pr(" %d", land.lnd_ship);
		break;
	    case 18:
		pr(" %d", land.lnd_item[I_SHELL]);
		break;
	    case 19:
		pr(" %d", land.lnd_item[I_GUN]);
		break;
	    case 20:
		pr(" %d", land.lnd_item[I_PETROL]);
		break;
	    case 21:
		pr(" %d", land.lnd_item[I_IRON]);
		break;
	    case 22:
		pr(" %d", land.lnd_item[I_DUST]);
		break;
	    case 23:
		pr(" %d", land.lnd_item[I_BAR]);
		break;
	    case 24:
		pr(" %d", land.lnd_item[I_OIL]);
		break;
	    case 25:
		pr(" %d", land.lnd_item[I_LCM]);
		break;
	    case 26:
		pr(" %d", land.lnd_item[I_HCM]);
		break;
	    case 27:
		pr(" %d", land.lnd_item[I_RAD]);
		break;
	    case 28:
		pr(" %1.2f", land.lnd_att);
		break;
	    case 29:
		pr(" %1.2f", land.lnd_def);
		break;
	    case 30:
		pr(" %d", land.lnd_vul);
		break;
	    case 31:
		pr(" %d", land.lnd_spd);
		break;
	    case 32:
		pr(" %d", land.lnd_vis);
		break;
	    case 33:
		pr(" %d", land.lnd_spy);
		break;
	    case 34:
		pr(" %d", land.lnd_rad);
		break;
	    case 35:
		pr(" %d", land.lnd_frg);
		break;
	    case 36:
		pr(" %d", land.lnd_acc);
		break;
	    case 37:
		pr(" %d", land.lnd_dam);
		break;
	    case 38:
		pr(" %d", land.lnd_ammo);
		break;
	    case 39:
		pr(" %d", land.lnd_aaf);
		break;
	    }
	}
	pr("\n");
    }
    if (nunits == 0) {
	if (player->argp[1])
	    pr("%s: No unit(s)\n", player->argp[1]);
	else
	    pr("%s: No unit(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d unit%s\n", nunits, splur(nunits));
    return RET_OK;
}
