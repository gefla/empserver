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
 *  pdump.c: Dump plane information
 * 
 *  Known contributors to this file:
 *     John Yockey, 1997
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "plane.h"
#include "nsc.h"
#include "nuke.h"
#include "file.h"
#include "nat.h"
#include "optlist.h"
#include "commands.h"

int
pdump(void)
{
    int nplanes;
    struct nstr_item np;
    struct plnstr plane;
    int field[128];
    struct natstr *natp;
    int n, i;
    time_t now;

    if (!snxtitem(&np, EF_PLANE, player->argp[1]))
	return RET_SYN;
    prdate();

    if (!player->argp[2]) {
	for (n = 1; n <= 32; n++)
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
	    } else if (!strcmp("wing", player->argp[n])) {
		field[i++] = 4;
	    } else if (!strcmp("eff", player->argp[n])) {
		field[i++] = 5;
	    } else if (!strcmp("mob", player->argp[n])) {
		field[i++] = 6;
	    } else if (!strcmp("tech", player->argp[n])) {
		field[i++] = 7;
	    } else if (!strcmp("att", player->argp[n])) {
		field[i++] = 8;
	    } else if (!strcmp("def", player->argp[n])) {
		field[i++] = 9;
	    } else if (!strcmp("acc", player->argp[n])) {
		field[i++] = 10;
	    } else if (!strcmp("react", player->argp[n])) {
		field[i++] = 11;
	    } else if (!strcmp("range", player->argp[n])) {
		field[i++] = 12;
	    } else if (!strcmp("load", player->argp[n])) {
		field[i++] = 13;
	    } else if (!strcmp("fuel", player->argp[n])) {
		field[i++] = 14;
	    } else if (!strcmp("hard", player->argp[n])) {
		field[i++] = 15;
	    } else if (!strcmp("ship", player->argp[n])) {
		field[i++] = 16;
	    } else if (!strcmp("land", player->argp[n])) {
		field[i++] = 17;
	    } else if (!strcmp("laun", player->argp[n])) {
		field[i++] = 18;
	    } else if (!strcmp("orb", player->argp[n])) {
		field[i++] = 19;
	    } else if (!strcmp("nuke", player->argp[n])) {
		field[i++] = 20;
	    } else if (!strcmp("grd", player->argp[n])) {
		field[i++] = 21;
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
    pr("DUMP PLANES %ld\n", (long)now);
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
	    pr(" wing");
	    break;
	case 5:
	    pr(" eff");
	    break;
	case 6:
	    pr(" mob");
	    break;
	case 7:
	    pr(" tech");
	    break;
	case 8:
	    pr(" att");
	    break;
	case 9:
	    pr(" def");
	    break;
	case 10:
	    pr(" acc");
	    break;
	case 11:
	    pr(" react");
	    break;
	case 12:
	    pr(" range");
	    break;
	case 13:
	    pr(" load");
	    break;
	case 14:
	    pr(" fuel");
	    break;
	case 15:
	    pr(" hard");
	    break;
	case 16:
	    pr(" ship");
	    break;
	case 17:
	    pr(" land");
	    break;
	case 18:
	    pr(" laun");
	    break;
	case 19:
	    pr(" orb");
	    break;
	case 20:
	    pr(" nuke");
	    break;
	case 21:
	    pr(" grd");
	    break;
	}
	n++;
    }
    pr("\n");

    nplanes = 0;
    natp = getnatp(player->cnum);
    while (nxtitem(&np, (s_char *)&plane)) {
	if (!player->owner || plane.pln_own == 0)
	    continue;
	nplanes++;
	if (player->god)
	    pr("%d ", plane.pln_own);
	pr("%d", np.cur);
	n = 0;
	while (field[n]) {
	    switch (field[n++]) {
	    case 1:
		pr(" %.4s", plchr[(int)plane.pln_type].pl_name);
		break;
	    case 2:
		pr(" %d", xrel(natp, plane.pln_x));
		break;
	    case 3:
		pr(" %d", yrel(natp, plane.pln_y));
		break;
	    case 4:
		if (plane.pln_wing == ' ')
		    pr(" ~");
		else
		    pr(" %c", plane.pln_wing);
		break;
	    case 5:
		pr(" %d", plane.pln_effic);
		break;
	    case 6:
		pr(" %d", plane.pln_mobil);
		break;
	    case 7:
		pr(" %d", plane.pln_tech);
		break;
	    case 8:
		pr(" %d", plane.pln_att);
		break;
	    case 9:
		pr(" %d", plane.pln_def);
		break;
	    case 10:
		pr(" %d", plane.pln_acc);
		break;
	    case 11:
		pr(" %d", plane.pln_range);
		break;
	    case 12:
		pr(" %d", plane.pln_range_max);
		break;
	    case 13:
		pr(" %d", plane.pln_load);
		break;
	    case 14:
		pr(" %d", plane.pln_fuel);
		break;
	    case 15:
		pr(" %d", plane.pln_harden);
		break;
	    case 16:
		pr(" %d", plane.pln_ship);
		break;
	    case 17:
		pr(" %d", plane.pln_land);
		break;
	    case 18:
		if ((plchr[(int)plane.pln_type].pl_flags & (P_O | P_M)) ==
		    P_O) {
		    pr(" %c",
		       (plane.pln_flags & PLN_LAUNCHED) ? 'Y' : 'N');
		} else
		    pr(" N");
		break;
	    case 19:
		if ((plchr[(int)plane.pln_type].pl_flags & (P_O | P_M)) ==
		    P_O) {
		    pr(" %c",
		       opt_ORBIT ? (plane.
				    pln_flags & PLN_SYNCHRONOUS) ? 'Y' :
		       'N' : 'N');
		} else
		    pr(" N");
		break;
	    case 20:
		if (plane.pln_nuketype != -1) {
		    pr(" %.5s", nchr[(int)plane.pln_nuketype].n_name);
		    break;
		} else
		    pr(" N/A");
		break;

	    case 21:
		pr(" %c", plane.pln_flags & PLN_AIRBURST ? 'A' : 'G');
		break;
	    }
	}
	pr("\n");
    }
    if (nplanes == 0) {
	if (player->argp[1])
	    pr("%s: No plane(s)\n", player->argp[1]);
	else
	    pr("%s: No plane(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d plane%s\n", nplanes, splur(nplanes));

    return RET_OK;
}
