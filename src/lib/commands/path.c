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
 *  path.c: Show empire distribution paths
 * 
 *  Known contributors to this file:
 *     David Muir Sharnoff, 1986
 *     (unknown rewrite), 1989
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "sect.h"
#include "path.h"
#include "map.h"
#include "commands.h"
#include "optlist.h"

int
path(void)
{

    struct nstr_sect ns;
    struct natstr *natp;
    struct range absrange;
    struct range relrange;
    struct sctstr sect, dsect;
    coord cx, cy;
    int i;
    int y;
    s_char *pp, *p;
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static s_char *mapbuf = (s_char *)0;
    static s_char **map = (s_char **)0;
    double move_cost;
    s_char buf[1024];

    if (!(p = getstarg(player->argp[1], "from sector : ", buf)) ||
	!sarg_xy(p, &cx, &cy) || !getsect(cx, cy, &sect))
	return RET_SYN;
    if ((sect.sct_own != player->cnum) && !player->god) {
	pr("Not yours\n");
	return RET_FAIL;
    }
    getsect(sect.sct_dist_x, sect.sct_dist_y, &dsect);
    pp = BestDistPath(buf, &sect, &dsect, &move_cost, MOB_ROAD);
    if (pp == (s_char *)0) {
	pr("No path possible from %s to distribution sector %s\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum),
	   xyas(dsect.sct_x, dsect.sct_y, player->cnum));
	return RET_FAIL;
    }
    if (!mapbuf)
	mapbuf =
	    (s_char *)malloc((WORLD_Y * MAPWIDTH(3)) * sizeof(s_char));
    if (!map) {
	map = (s_char **)malloc(WORLD_Y * sizeof(s_char *));
	if (map && mapbuf) {
	    for (i = 0; i < WORLD_Y; i++)
		map[i] = &mapbuf[MAPWIDTH(3) * i];
	} else if (map) {
	    free((s_char *)map);
	    map = (s_char **)0;
	}
    }
    if (!mapbuf || !map) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in path\n");
	return RET_FAIL;
    }
    pathrange(cx, cy, pp, 1, &absrange);
    snxtsct_area(&ns, &absrange);
    natp = getnatp(player->cnum);
    xyrelrange(natp, &absrange, &relrange);
    blankfill((s_char *)mapbuf, &ns.range, 3);
    while (*pp && (i = chkdir(*pp, DIR_STOP, DIR_LAST)) >= 0) {
	memcpy(&map[deltay(cy, ns.range.ly)][deltax(cx, ns.range.lx) * 2],
	       routech[i][0],
	       3);
	cx += diroff[i][0];
	cy += diroff[i][1];
	++pp;
    }
    border(&relrange, "     ", " ");
    while (nxtsct(&ns, &sect)) {
	if (!player->owner)
	    continue;
	map[ns.dy][ns.dx * 2 + 1] = dchr[sect.sct_type].d_mnem;
    }
    for (y = ns.range.ly, i = 0; i < ns.range.height; y++, i++) {
	cy = yrel(natp, y);
	pr("%4d %s %-4d\n", cy, map[i], cy);
	if (y >= WORLD_Y)
	    y -= WORLD_Y;
    }
    border(&relrange, "     ", " ");
    return RET_OK;
}
