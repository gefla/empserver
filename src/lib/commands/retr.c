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
 *  retr.c: Set retreat conditionals for ships and land units
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 2000
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "empobj.h"
#include "land.h"
#include "path.h"
#include "retreat.h"
#include "ship.h"

static int retreat(short);

int
retr(void)
{
    return retreat(EF_SHIP);
}

int
lretr(void)
{
    return retreat(EF_LAND);
}

static int
retreat(short type)
{
    char *pq, *fl;
    int nunits;
    struct nstr_item ni;
    union empobj_storage unit;
    int rflags;
    unsigned i;
    char *name, *rpath, *what;
    int *rflagsp;
    char buf1[1024];
    char buf2[1024];

    if (CANT_HAPPEN(type != EF_LAND && type != EF_SHIP))
	type = EF_SHIP;

    if (!snxtitem(&ni, type, player->argp[1]))
	return RET_SYN;
    nunits = 0;
    if (player->argp[2] != NULL)
	pq = getstarg(player->argp[2], "Retreat path? ", buf1);
    else
	pq = NULL;

    rflags = 0;
    if (pq != NULL) {
	fl = getstarg(player->argp[3],
		      type == EF_SHIP
		      ? "Retreat conditions [i|t|s|h|b|d|u|c]? "
		      : "Retreat conditions [i|h|b|c]? ",
		      buf2);
	if (!fl)
	    return RET_SYN;

	for (i = 0; fl[i]; i++) {
	    switch (fl[i]) {
	    case 'I':
	    case 'i':
		rflags |= RET_INJURED;
		break;
	    case 'T':
	    case 't':
		if (type == EF_LAND)
		    goto badflag;
		rflags |= RET_TORPED;
		break;
	    case 'S':
	    case 's':
		if (type == EF_LAND)
		    goto badflag;
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
		if (type == EF_LAND)
		    goto badflag;
		rflags |= RET_DCHRGED;
		break;
	    case 'U':
	    case 'u':
		if (type == EF_LAND)
		    goto badflag;
		rflags |= RET_BOARDED;
		break;
	    case 'C':
	    case 'c':
		pq = "";
		break;
	    default:
	    badflag:
		pr("bad condition\n");
		/* fall through */
	    case '?':
		pr("i\tretreat when injured\n");
		if (type == EF_SHIP) {
		    pr("t\tretreat when torped\n");
		    pr("s\tretreat when sonared\n");
		}
		pr("h\tretreat when helpless\n");
		pr("b\tretreat when bombed\n");
		if (type == EF_SHIP) {
		    pr("d\tretreat when depth-charged\n");
		    pr("u\tretreat when boarded\n");
		}
	    }
	}
	if (*pq && !rflags) {
	    pr("Must give retreat conditions!\n");
	    return RET_FAIL;
	}
	if (ni.sel == NS_GROUP && ni.group)
	    rflags |= RET_GROUP;
	if (!*pq)
	    rflags = 0;
    }

    while (nxtitem(&ni, &unit)) {
	if (!player->owner || unit.gen.own == 0)
	    continue;
	if (type == EF_SHIP) {
	    if (nunits++ == 0) {
		if (player->god)
		    pr("own ");
		pr("shp#     ship type       x,y   fl path       as flt?  flags\n");
	    }
	    name = mchr[unit.ship.shp_type].m_name;
	    rpath = unit.ship.shp_rpath;
	    rflagsp = &unit.ship.shp_rflags;
	} else {
	    if (nunits++ == 0) {
		if (player->god)
		    pr("own ");
		pr("lnd#     unit type       x,y   ar path       as army? flags\n");
	    }
	    name = lchr[unit.land.lnd_type].l_name;
	    rpath = unit.land.lnd_rpath;
	    rflagsp = &unit.land.lnd_rflags;
	}
	if (pq) {
	    strncpy(rpath, pq, RET_LEN - 1);
	    *rflagsp = rflags;
	    put_empobj(type, unit.gen.uid, &unit);
	}
	if (player->god)
	    pr("%3d ", unit.gen.own);
	pr("%4d ", ni.cur);
	pr("%-16.16s ", name);
	prxy("%4d,%-4d ", unit.gen.x, unit.gen.y, player->cnum);
	pr("%1.1s", &unit.gen.group);
	pr(" %-11s", rpath);
	rflags = *rflagsp;
	if (rflags & RET_GROUP)
	    pr("Yes      ");
	else
	    pr("         ");
	if (rflags & RET_INJURED)
	    pr("I");
	if (rflags & RET_TORPED)
	    pr("T");
	if (rflags & RET_SONARED)
	    pr("S");
	if (rflags & RET_HELPLESS)
	    pr("H");
	if (rflags & RET_BOMBED)
	    pr("B");
	if (rflags & RET_DCHRGED)
	    pr("D");
	if (rflags & RET_BOARDED)
	    pr("U");
	pr("\n");
    }
    what = type == EF_SHIP ? "ship" : "unit";
    if (nunits == 0) {
	if (player->argp[1])
	    pr("%s: No %s(s)\n", player->argp[1], what);
	else
	    pr("%s: No %s(s)\n", "", what);
	return RET_FAIL;
    } else
	pr("%d %s%s\n", nunits, what, splur(nunits));
    return RET_OK;
}
