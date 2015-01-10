/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 *     Markus Armbruster, 2008-2015
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

static int retreat(int);
static int retreat_show(int, struct nstr_item *);

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
retreat(int type)
{
    char *pq, *fl;
    int nunits;
    struct nstr_item ni;
    union empobj_storage unit;
    int i, rflags, ch, j;
    char *rflagsc, *p;
    char buf1[1024];
    char buf2[1024];

    if (CANT_HAPPEN(type != EF_LAND && type != EF_SHIP))
	type = EF_SHIP;
    rflagsc = type == EF_SHIP ? shp_rflagsc : lnd_rflagsc;

    if (!snxtitem(&ni, type, player->argp[1], NULL))
	return RET_SYN;

    if (player->argp[1] && !player->argp[2]) {
	pr("Omitting the second argument is deprecated and will cease to work in a\n"
	   "future release.  Please use '%s q' to query retreat orders.\n\n",
	   player->combuf);
	pq = "q";
    } else {
	/*
	 * TODO getpath() or similar would be nice once the deprecated
	 * syntax is gone.
	 */
	pq = getstarg(player->argp[2], "Retreat path, or q to query? ",
		      buf1);
	if (!pq || !*pq)
	    return RET_SYN;
    }

    if (*pq == 'q')
	return retreat_show(type, &ni);

    for (i = 0; i < RET_LEN - 1 && pq[i]; i++) {
	if (chkdir(pq[i], DIR_STOP, DIR_LAST) < 0) {
	    pr("'%c' is not a valid direction...\n", pq[i]);
	    direrr(NULL, NULL, NULL);
	    return RET_SYN;
	}
    }
    for (i--; i >= 0 && pq[i] == dirch[DIR_STOP]; i--)
	pq[i] = 0;

    rflags = 0;
    if (*pq) {
    again:
	fl = getstarg(player->argp[3],
		      "Retreat conditions ('?' to list available ones)? ",
		      buf2);
	if (!fl)
	    return RET_SYN;

	for (i = 0; fl[i]; i++) {
	    ch = tolower(fl[i]);
	    if (ch == 'c') {
		/* Deprecated, but keeping it around doesn't hurt */
		*pq = 0;
		break;
	    }
	    if (ch == '?' && !player->argp[3]) {
		for (j = 1; rflagsc[j]; j++) {
		    if (rflagsc[j] != 'X')
			pr("%c\tretreat when %s\n",
			   rflagsc[j],
			   symbol_by_value(1 << j, retreat_flags));
		}
		goto again;
	    }
	    p = strchr(rflagsc, ch);
	    if (!p) {
		pr("Bad retreat condition '%c'\n", fl[i]);
		return RET_SYN;
	    }
	    rflags |= 1 << (p - rflagsc);
	}
	if (*pq && !rflags)
	    return RET_SYN;
	if (ni.sel == NS_GROUP && ni.group)
	    rflags |= RET_GROUP;
	if (!*pq)
	    rflags = 0;
    }

    nunits = 0;
    while (nxtitem(&ni, &unit)) {
	if (!player->owner || unit.gen.own == 0)
	    continue;
	if (type == EF_SHIP) {
	    strncpy(unit.ship.shp_rpath, pq, RET_LEN - 1);
	    unit.ship.shp_rflags = rflags;
	} else {
	    strncpy(unit.land.lnd_rpath, pq, RET_LEN - 1);
	    unit.land.lnd_rflags = rflags;
	}
	put_empobj(type, unit.gen.uid, &unit);
	nunits++;
    }
    if (rflags) {
	symbol_set_fmt(buf2, sizeof(buf2), rflags & ~RET_GROUP,
		       retreat_flags, ", ", 0);
	pr("%d %s%s ordered to retreat%s along path %s when %s\n",
	   nunits, ef_nameof_pretty(type), splur(nunits),
	   rflags & RET_GROUP ? " as group" : "", pq, buf2);
    } else
	pr("%d %s%s ordered not to retreat\n",
	   nunits, ef_nameof_pretty(type), splur(nunits));
    return RET_OK;
}

static int
retreat_show(int type, struct nstr_item *np)
{
    char *rflagsc = type == EF_SHIP ? shp_rflagsc : lnd_rflagsc;
    union empobj_storage unit;
    int nunits;
    char *name, *rpath, *what;
    int *rflagsp, rflags, i;

    nunits = 0;
    while (nxtitem(np, &unit)) {
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
	if (player->god)
	    pr("%3d ", unit.gen.own);
	pr("%4d ", np->cur);
	pr("%-16.16s ", name);
	prxy("%4d,%-4d ", unit.gen.x, unit.gen.y);
	pr("%1.1s", &unit.gen.group);
	pr(" %-11s", rpath);
	rflags = *rflagsp;
	if (rflags & RET_GROUP)
	    pr("Yes      ");
	else
	    pr("         ");
	for (i = 1; rflagsc[i]; i++) {
	    if ((1 << i) & rflags) {
		if (CANT_HAPPEN(rflagsc[i] == 'X'))
		    continue;
		pr("%c", rflagsc[i]);
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
