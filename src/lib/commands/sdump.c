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
 *  sdump.c: Dump ship information
 * 
 *  Known contributors to this file:
 *     John Yockey, 1997
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "ship.h"
#include "nat.h"
#include "nsc.h"
#include "file.h"
#include "optlist.h"
#include "commands.h"

int
sdump(void)
{
    int nships;
    struct nstr_item ni;
    struct shpstr ship;
    int field[128];
    struct natstr *np;
    int n, i;
    time_t now;

    if (!snxtitem(&ni, EF_SHIP, player->argp[1]))
	return RET_SYN;
    prdate();

    if (!player->argp[2]) {
	for (n = 1; n <= 34; n++)
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
	    } else if (!strcmp("flt", player->argp[n])) {
		field[i++] = 4;
	    } else if (!strcmp("eff", player->argp[n])) {
		field[i++] = 5;
	    } else if (!strcmp("civ", player->argp[n])) {
		field[i++] = 6;
	    } else if (!strcmp("mil", player->argp[n])) {
		field[i++] = 7;
	    } else if (!strcmp("uw", player->argp[n])) {
		field[i++] = 8;
	    } else if (!strcmp("food", player->argp[n])) {
		field[i++] = 9;
	    } else if (!strcmp("pln", player->argp[n])) {
		field[i++] = 10;
	    } else if (!strcmp("he", player->argp[n])) {
		field[i++] = 11;
	    } else if (!strcmp("xl", player->argp[n])) {
		field[i++] = 12;
	    } else if (!strcmp("land", player->argp[n])) {
		field[i++] = 13;
	    } else if (!strcmp("mob", player->argp[n])) {
		field[i++] = 14;
	    } else if (!strcmp("fuel", player->argp[n])) {
		field[i++] = 15;
	    } else if (!strcmp("tech", player->argp[n])) {
		field[i++] = 16;
	    } else if (!strcmp("shell", player->argp[n])) {
		field[i++] = 17;
	    } else if (!strcmp("gun", player->argp[n])) {
		field[i++] = 18;
	    } else if (!strcmp("petrol", player->argp[n])) {
		field[i++] = 19;
	    } else if (!strcmp("iron", player->argp[n])) {
		field[i++] = 20;
	    } else if (!strcmp("dust", player->argp[n])) {
		field[i++] = 21;
	    } else if (!strcmp("bar", player->argp[n])) {
		field[i++] = 22;
	    } else if (!strcmp("oil", player->argp[n])) {
		field[i++] = 23;
	    } else if (!strcmp("lcm", player->argp[n])) {
		field[i++] = 24;
	    } else if (!strcmp("hcm", player->argp[n])) {
		field[i++] = 25;
	    } else if (!strcmp("rad", player->argp[n])) {
		field[i++] = 26;
	    } else if (!strcmp("def", player->argp[n])) {
		field[i++] = 27;
	    } else if (!strcmp("spd", player->argp[n])) {
		field[i++] = 28;
	    } else if (!strcmp("vis", player->argp[n])) {
		field[i++] = 29;
	    } else if (!strcmp("rng", player->argp[n])) {
		field[i++] = 30;
	    } else if (!strcmp("fir", player->argp[n])) {
		field[i++] = 31;
	    } else if (!strcmp("origx", player->argp[n])) {
		field[i++] = 32;
	    } else if (!strcmp("origy", player->argp[n])) {
		field[i++] = 33;
	    } else if (!strcmp("name", player->argp[n])) {
		field[i++] = 34;
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
    pr("DUMP SHIPS %d\n", now);
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
	    pr(" flt");
	    break;
	case 5:
	    pr(" eff");
	    break;
	case 6:
	    pr(" civ");
	    break;
	case 7:
	    pr(" mil");
	    break;
	case 8:
	    pr(" uw");
	    break;
	case 9:
	    pr(" food");
	    break;
	case 10:
	    pr(" pln");
	    break;
	case 11:
	    pr(" he");
	    break;
	case 12:
	    pr(" xl");
	    break;
	case 13:
	    pr(" land");
	    break;
	case 14:
	    pr(" mob");
	    break;
	case 15:
	    pr(" fuel");
	    break;
	case 16:
	    pr(" tech");
	    break;
	case 17:
	    pr(" shell");
	    break;
	case 18:
	    pr(" gun");
	    break;
	case 19:
	    pr(" petrol");
	    break;
	case 20:
	    pr(" iron");
	    break;
	case 21:
	    pr(" dust");
	    break;
	case 22:
	    pr(" bar");
	    break;
	case 23:
	    pr(" oil");
	    break;
	case 24:
	    pr(" lcm");
	    break;
	case 25:
	    pr(" hcm");
	    break;
	case 26:
	    pr(" rad");
	    break;
	case 27:
	    pr(" def");
	    break;
	case 28:
	    pr(" spd");
	    break;
	case 29:
	    pr(" vis");
	    break;
	case 30:
	    pr(" rng");
	    break;
	case 31:
	    pr(" fir");
	    break;
	case 32:
	    pr(" origx");
	    break;
	case 33:
	    pr(" origy");
	    break;
	case 34:
	    pr(" name");
	    break;
	}
	n++;
    }
    pr("\n");

    nships = 0;
    np = getnatp(player->cnum);
    while (nxtitem(&ni, (s_char *)&ship)) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (ship.shp_type < 0 || ship.shp_type > shp_maxno) {
	    pr("bad ship type %d (#%d)\n", ship.shp_type, ni.cur);
	    continue;
	}
	count_planes(&ship);
	count_units(&ship);
	nships++;
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%d", ni.cur);
	n = 0;
	while (field[n]) {
	    switch (field[n++]) {
	    case 1:
		pr(" %0.4s", mchr[(int)ship.shp_type].m_name);
		break;
	    case 2:
		pr(" %d", xrel(np, ship.shp_x));
		break;
	    case 3:
		pr(" %d", yrel(np, ship.shp_y));
		break;
	    case 4:
		if (ship.shp_fleet == ' ')
		    pr(" ~");
		else
		    pr(" %c", ship.shp_fleet);
		break;
	    case 5:
		pr(" %d", ship.shp_effic);
		break;
	    case 6:
		pr(" %d", ship.shp_item[I_CIVIL]);
		break;
	    case 7:
		pr(" %d", ship.shp_item[I_MILIT]);
		break;
	    case 8:
		pr(" %d", ship.shp_item[I_UW]);
		break;
	    case 9:
		pr(" %d", ship.shp_item[I_FOOD]);
		break;
	    case 10:
		pr(" %d", ship.shp_nplane);
		break;
	    case 11:
		pr(" %d", ship.shp_nchoppers);
		break;
	    case 12:
		pr(" %d", ship.shp_nxlight);
		break;
	    case 13:
		pr(" %d", ship.shp_nland);
		break;
	    case 14:
		pr(" %d", ship.shp_mobil);
		break;
	    case 15:
		if (opt_FUEL)
		    pr(" %d", ship.shp_fuel);
		else
		    pr(" 0");
		break;
	    case 16:
		pr(" %d", ship.shp_tech);
		break;
	    case 17:
		pr(" %d", ship.shp_item[I_SHELL]);
		break;
	    case 18:
		pr(" %d", ship.shp_item[I_GUN]);
		break;
	    case 19:
		pr(" %d", ship.shp_item[I_PETROL]);
		break;
	    case 20:
		pr(" %d", ship.shp_item[I_IRON]);
		break;
	    case 21:
		pr(" %d", ship.shp_item[I_DUST]);
		break;
	    case 22:
		pr(" %d", ship.shp_item[I_BAR]);
		break;
	    case 23:
		pr(" %d", ship.shp_item[I_OIL]);
		break;
	    case 24:
		pr(" %d", ship.shp_item[I_LCM]);
		break;
	    case 25:
		pr(" %d", ship.shp_item[I_HCM]);
		break;
	    case 26:
		pr(" %d", ship.shp_item[I_RAD]);
		break;
	    case 27:
		pr(" %d", ship.shp_armor);
		break;
	    case 28:
		pr(" %d", ship.shp_speed);
		break;
	    case 29:
		pr(" %d", ship.shp_visib);
		break;
	    case 30:
		pr(" %d", ship.shp_frnge);
		break;
	    case 31:
		pr(" %d", ship.shp_glim);
		break;
	    case 32:
		/*
		 * Disclosing construction site allows pirates to find
		 * harbors.  Disclose it only to the original owner
		 * and to deities.  Else dump illegal coordinates
		 * 1,0
		 */
		if (ship.shp_own == ship.shp_orig_own || player->god)
		    pr(" %d", xrel(np, ship.shp_orig_x));
		else
		    pr(" 1");
		break;
	    case 33:
		/* see case 32 */
		if (ship.shp_own == ship.shp_orig_own || player->god)
		    pr(" %d", yrel(np, ship.shp_orig_y));
		else
		    pr(" 0");
		break;
	    case 34:
		pr(" %c%s%c", '"', ship.shp_name, '"');
		break;
	    }
	}
	pr("\n");
    }
    if (nships == 0) {
	if (player->argp[1])
	    pr("%s: No ship(s)\n", player->argp[1]);
	else
	    pr("%s: No ship(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d ship%s\n", nships, splur(nships));
    return RET_OK;
}
