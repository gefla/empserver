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
 *     Markus Armbruster, 2008
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "empobj.h"
#include "land.h"
#include "path.h"
#include "retreat.h"
#include "ship.h"

/*
 * Retreat flag characters
 * 'X' means flag is not available
 * Must agree with RET_ defines.
 */
static char shp_rflagsc[] = "Xitshbdu";
static char lnd_rflagsc[] = "XiXXhbXX";

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
    int rflags, ch, j;
    unsigned i;
    char *rflagsc, *p, *name, *rpath, *what;
    int *rflagsp;
    char buf1[1024];
    char buf2[1024];

    if (CANT_HAPPEN(type != EF_LAND && type != EF_SHIP))
	type = EF_SHIP;
    rflagsc = type == EF_SHIP ? shp_rflagsc : lnd_rflagsc;

    if (!snxtitem(&ni, type, player->argp[1], NULL))
	return RET_SYN;
    nunits = 0;
    if (player->argp[2] != NULL)
	pq = getstarg(player->argp[2], "Retreat path? ", buf1);
    else
	pq = NULL;

    rflags = 0;
    if (pq != NULL) {
    again:
	fl = getstarg(player->argp[3],
		      "Retreat conditions ('?' to list available ones)? ",
		      buf2);
	if (!fl)
	    return RET_SYN;

	for (i = 0; fl[i]; i++) {
	    ch = tolower(fl[i]);
	    if (ch == 'C') {
		*pq = 0;
		return 0;
	    }
	    if (ch == '?') {
		for (j = 1; rflagsc[j]; j++) {
		    if (rflagsc[j] != 'X')
			pr("%c\tretreat when %s\n",
			   rflagsc[j],
			   symbol_by_value(1 << j, retreat_flags));
		}
		pr("c\tcancel retreat order\n");
		goto again;
	    }
	    p = strchr(rflagsc, ch);
	    if (!p) {
		pr("Bad retreat condition '%c'\n", fl[i]);
		return RET_SYN;
	    }
	    rflags |= 1 << (p - rflagsc);
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
	for (j = 1; rflagsc[j]; j++) {
	    if ((1 << j) & rflags) {
		if (CANT_HAPPEN(rflagsc[j] == 'X'))
		    continue;
		pr("%c", rflagsc[j]);
	    }
	}
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
