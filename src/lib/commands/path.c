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
 *  path.c: Show empire distribution paths
 *
 *  Known contributors to this file:
 *     David Muir Sharnoff, 1986
 *     (unknown rewrite), 1989
 *     Markus Armbruster, 2005-2011
 */

#include <config.h>

#include "commands.h"
#include "map.h"
#include "optlist.h"
#include "path.h"

int
path(void)
{

    struct nstr_sect ns;
    struct natstr *natp;
    struct range absrange;
    struct range relrange;
    struct sctstr sect;
    coord cx, cy;
    int i;
    int y;
    char *pp, *p;
    size_t len;
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static char *mapbuf = NULL;
    static char **map = NULL;
    char buf[1024];

    if (!(p = getstarg(player->argp[1], "from sector : ", buf)) ||
	!sarg_xy(p, &cx, &cy) || !getsect(cx, cy, &sect))
	return RET_SYN;
    if ((sect.sct_own != player->cnum) && !player->god) {
	pr("Not yours\n");
	return RET_FAIL;
    }
    if (path_find(sect.sct_x, sect.sct_y, sect.sct_dist_x, sect.sct_dist_y,
		  player->cnum, MOB_MOVE) < 0) {
	pr("No path possible from %s to distribution sector %s\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum),
	   xyas(sect.sct_dist_x, sect.sct_dist_y, player->cnum));
	return RET_FAIL;
    }
    len = path_find_route(buf, sizeof(buf), sect.sct_x, sect.sct_y,
			  sect.sct_dist_x, sect.sct_dist_y);
    if (len >= sizeof(buf)) {
	pr("Can't handle path from %s to distribution sector %s, it's too long, sorry.\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum),
	   xyas(sect.sct_dist_x, sect.sct_dist_y, player->cnum));
	return RET_FAIL;
    }
    if (!mapbuf)
	mapbuf = malloc(WORLD_Y * MAPWIDTH(3));
    if (!map) {
	map = malloc(WORLD_Y * sizeof(char *));
	if (map && mapbuf) {
	    for (i = 0; i < WORLD_Y; i++)
		map[i] = &mapbuf[MAPWIDTH(3) * i];
	} else if (map) {
	    free(map);
	    map = NULL;
	}
    }
    if (!mapbuf || !map) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in path\n");
	return RET_FAIL;
    }
    pathrange(cx, cy, buf, 1, &absrange);
    snxtsct_area(&ns, &absrange);
    natp = getnatp(player->cnum);
    xyrelrange(natp, &absrange, &relrange);
    blankfill(mapbuf, &ns.range, 3);
    for (pp = buf; *pp; ++pp) {
	i = diridx(*pp);
	if (CANT_HAPPEN(i == DIR_STOP))
	    break;
	memcpy(&map[delty(&ns.range, cy)][deltx(&ns.range, cx) * 2],
	       routech[i], 3);
	cx = xnorm(cx + diroff[i][0]);
	cy = ynorm(cy + diroff[i][1]);
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
