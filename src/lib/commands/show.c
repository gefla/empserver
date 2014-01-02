/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  show.c: Give info on empire objects, planes, boats, nukes, etc.
 *
 *  Known contributors to this file:
 *     Julian Onions, 1988
 *     Steve McClure, 1997
 */

#include <config.h>

#include "commands.h"
#include "nuke.h"
#include "optlist.h"

int
show(void)
{
    char *p;
    void (*cfunc)(int);
    void (*sfunc)(int);
    void (*bfunc)(int);
    struct natstr *natp;
    int tlev;
    char buf[1024];
    int rlev;

again:
    p = getstarg(player->argp[1], "Show what ('?' to list options)? ", buf);
    if (!p || !*p)
	return RET_SYN;
    if (*p == '?') {
	pr("bridge, item, land, news, nuke, plane, sect, ship, product, tower, updates\n");
	goto again;
    }

    natp = getnatp(player->cnum);
    rlev = (int)(1.25 * natp->nat_level[NAT_RLEV]);

    if (!player->argp[3]) {
	tlev = (int)(1.25 * natp->nat_level[NAT_TLEV]);
	if (player->god)
	    tlev = 1000;
    } else {
	tlev = (int)atoi(player->argp[3]);
	if (tlev > (int)(1.25 * natp->nat_level[NAT_TLEV]) && !player->god)
	    tlev = (int)(1.25 * natp->nat_level[NAT_TLEV]);
    }
    if (player->god)
	rlev = 1000;
    switch (*p) {
    case 'b':
	show_bridge(99999);
	return RET_OK;
    case 't':
	show_tower(99999);
	return RET_OK;
    case 'i':
	show_item(99999);
	return RET_OK;
    case 'n':
	if (*(p + 1) == 'e') {
	    show_news(99999);
	    return RET_OK;
	}
	if (drnuke_const > MIN_DRNUKE_CONST)
	    tlev = ((rlev / drnuke_const) > tlev ? tlev :
		    (rlev / drnuke_const));
	bfunc = show_nuke_build;
	cfunc = show_nuke_capab;
	sfunc = show_nuke_stats;
	break;
    case 'l':
	bfunc = show_land_build;
	sfunc = show_land_stats;
	cfunc = show_land_capab;
	break;
    case 'p':
	if (p[1] == 'r') {
	    show_product(99999);
	    return RET_OK;
	}
	bfunc = show_plane_build;
	sfunc = show_plane_stats;
	cfunc = show_plane_capab;
	break;
    case 's':
	if (*(p + 1) == 'e') {
	    bfunc = show_sect_build;
	    sfunc = show_sect_stats;
	    cfunc = show_sect_capab;
	} else {
	    bfunc = show_ship_build;
	    sfunc = show_ship_stats;
	    cfunc = show_ship_capab;
	}
	break;
    case 'u':
	show_updates(player->argp[2] ? atoi(player->argp[2]) : 8);
	return RET_OK;
    default:
	return RET_SYN;
    }

    p = getstarg(player->argp[2],
		 "Build, stats, or capability data (b,s,c)? ", buf);
    if (!p || !*p)
	return RET_SYN;
    pr("Printing for tech level '%d'\n", tlev);
    if (*p == 'B' || *p == 'b')
	bfunc(tlev);
    else if (*p == 'C' || *p == 'c')
	cfunc(tlev);
    else if (*p == 'S' || *p == 's')
	sfunc(tlev);
    else
	return RET_SYN;
    return RET_OK;
}
