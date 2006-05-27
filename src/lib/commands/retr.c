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
 *  retr.c: Set retreat conditionals for ships
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 2000
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "retreat.h"
#include "ship.h"
#include "land.h"
#include "nsc.h"
#include "file.h"
#include "path.h"
#include "commands.h"

int
retr(void)
{
    char *pq, *fl;
    int nships;
    struct nstr_item ni;
    struct shpstr ship;
    int isfleet = 0;
    int rflags = -2;
    int zero;
    unsigned i;
    char buf1[1024];
    char buf2[1024];

    if (!snxtitem(&ni, EF_SHIP, player->argp[1]))
	return RET_SYN;
    nships = 0;
    if (player->argp[1] != NULL)
	if (isalpha(player->argp[1][0]))
	    isfleet = RET_GROUP;
    if (player->argp[2] != NULL)
	pq = getstarg(player->argp[2], "Retreat path? ", buf1);
    else
	pq = NULL;

    if (pq != NULL) {
	fl = getstarg(player->argp[3],
		      "Retreat conditions [i|t|s|h|b|d|u|c]? ", buf2);
	if (!fl)
	    return RET_SYN;
	rflags = 0 | isfleet;

	for (i = 0; i < strlen(fl); i++) {
	    switch (fl[i]) {
	    case 'I':
	    case 'i':
		rflags |= RET_INJURED;
		break;
	    case 'T':
	    case 't':
		rflags |= RET_TORPED;
		break;
	    case 'S':
	    case 's':
		rflags |= RET_SONARED;
		break;
	    case 'H':
	    case 'h':
		rflags |= RET_HELPLESS;
		break;
	    case 'B':
	    case 'b':
		rflags |= RET_BOMBED;
		break;
	    case 'D':
	    case 'd':
		rflags |= RET_DCHRGED;
		break;
	    case 'U':
	    case 'u':
		rflags |= RET_BOARDED;
		break;
	    case 'C':
	    case 'c':
		rflags = -1;
		break;
	    default:
		pr("bad condition\n");
		/* fall through */
	    case '?':
		pr("i\tretreat when injured\n");
		pr("t\tretreat when torped\n");
		pr("s\tretreat when sonared\n");
		pr("h\tretreat when helpless\n");
		pr("b\tretreat when bombed\n");
		pr("d\tretreat when depth-charged\n");
		pr("u\tretreat when boarded\n");
	    }
	}
	if (rflags == isfleet) {
	    pr("Must give retreat conditions!\n");
	    return RET_FAIL;
	}
    }

    if (rflags == -1)
	pq = NULL;

    zero = (rflags == -1);
    if (zero)
	rflags = 0;

    while (nxtitem(&ni, &ship)) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (zero)
	    memset(ship.shp_rpath, 0, sizeof(ship.shp_rpath));

	if (pq != NULL) {
	    strncpy(ship.shp_rpath, pq, sizeof(ship.shp_rpath) - 1);
	    putship(ship.shp_uid, &ship);
	}
	if (rflags >= 0) {
	    ship.shp_rflags = rflags;
	    putship(ship.shp_uid, &ship);
	}
	if (nships++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("shp#     ship type       x,y   fl path       as flt? flags\n");
	}
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%4d ", ni.cur);
	pr("%-16.16s ", mchr[(int)ship.shp_type].m_name);
	prxy("%4d,%-4d ", ship.shp_x, ship.shp_y, player->cnum);
	pr("%1.1s", &ship.shp_fleet);
	pr(" %-11s", ship.shp_rpath);
	if (ship.shp_rflags & RET_GROUP)
	    pr("Yes     ");
	else
	    pr("        ");
	if (ship.shp_rflags & RET_INJURED)
	    pr("I");
	if (ship.shp_rflags & RET_TORPED)
	    pr("T");
	if (ship.shp_rflags & RET_SONARED)
	    pr("S");
	if (ship.shp_rflags & RET_HELPLESS)
	    pr("H");
	if (ship.shp_rflags & RET_BOMBED)
	    pr("B");
	if (ship.shp_rflags & RET_DCHRGED)
	    pr("D");
	if (ship.shp_rflags & RET_BOARDED)
	    pr("U");
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

int
lretr(void)
{
    char *pq, *fl;
    int nunits;
    struct nstr_item ni;
    struct lndstr land;
    int isarmy = 0;
    int rflags = -2;
    int zero;
    char buf1[1024];
    char buf2[1024];
    unsigned i;

    if (!snxtitem(&ni, EF_LAND, player->argp[1]))
	return RET_SYN;
    nunits = 0;
    if (player->argp[1] != NULL)
	if (isalpha(player->argp[1][0]))
	    isarmy = RET_GROUP;
    if (player->argp[2] != NULL)
	pq = getstarg(player->argp[2], "Retreat path? ", buf1);
    else
	pq = NULL;
    if (pq != NULL) {
	fl = getstarg(player->argp[3], "Retreat conditions [i|h|b|c]? ",
		      buf2);
	if (!fl)
	    return RET_SYN;
	rflags = 0 | isarmy;

	for (i = 0; i < strlen(fl); i++) {
	    switch (fl[i]) {
	    case 'I':
	    case 'i':
		rflags |= RET_INJURED;
		break;
	    case 'H':
	    case 'h':
		rflags |= RET_HELPLESS;
		break;
	    case 'B':
	    case 'b':
		rflags |= RET_BOMBED;
		break;
	    case 'C':
	    case 'c':
		rflags = -1;
		break;
	    default:
		pr("bad condition\n");
		/* fall through */
	    case '?':
		pr("i\tretreat when injured\n");
		pr("h\tretreat when helpless\n");
		pr("b\tretreat when bombed\n");
	    }
	}
	if (rflags == isarmy) {
	    pr("Must give retreat conditions!\n");
	    return RET_FAIL;
	}
    }

    if (rflags == -1)
	pq = NULL;

    zero = (rflags == -1);
    if (zero)
	rflags = 0;

    while (nxtitem(&ni, &land)) {
	if (!player->owner || land.lnd_own == 0)
	    continue;
	if (zero)
	    memset(land.lnd_rpath, 0, sizeof(land.lnd_rpath));

	if (pq != NULL) {
	    strncpy(land.lnd_rpath, pq, sizeof(land.lnd_rpath) - 1);
	    putland(land.lnd_uid, &land);
	}
	if (rflags >= 0) {
	    land.lnd_rflags = rflags;
	    putland(land.lnd_uid, &land);
	}

	if (nunits++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("lnd#     unit type       x,y   ar path       as army? flags\n");
	}
	if (player->god)
	    pr("%3d ", land.lnd_own);
	pr("%4d ", ni.cur);
	pr("%-16.16s ", lchr[(int)land.lnd_type].l_name);
	prxy("%4d,%-4d ", land.lnd_x, land.lnd_y, player->cnum);
	pr("%1.1s", &land.lnd_army);
	pr(" %-11s", land.lnd_rpath);
	if (land.lnd_rflags & RET_GROUP)
	    pr("Yes      ");
	else
	    pr("         ");
	if (land.lnd_rflags & RET_INJURED)
	    pr("I");
	if (land.lnd_rflags & RET_TORPED)
	    pr("T");
	if (land.lnd_rflags & RET_SONARED)
	    pr("S");
	if (land.lnd_rflags & RET_HELPLESS)
	    pr("H");
	if (land.lnd_rflags & RET_BOMBED)
	    pr("B");
	if (land.lnd_rflags & RET_DCHRGED)
	    pr("D");
	if (land.lnd_rflags & RET_BOARDED)
	    pr("U");
	pr("\n");
    }
    if (nunits == 0) {
	if (player->argp[1])
	    pr("%s: No unit(s)\n", player->argp[1]);
	else
	    pr("%s: No unit(s)\n", "");
	return RET_FAIL;
    }
    pr("%d unit%s\n", nunits, splur(nunits));
    return RET_OK;
}
