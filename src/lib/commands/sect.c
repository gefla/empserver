/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sect.c: Show sectors in map-like format with conditionals.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include "commands.h"
#include "map.h"
#include "optlist.h"

int
sct(void)
{
    struct nstr_sect ns;
    struct sctstr sect;
    struct nscstr cond[NS_NCOND];
    struct range range;
    struct natstr *np;
    int ncond;
    int nsect;
    char *ptr;
    coord y, yval;
    int i;
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static char *mapbuf = NULL;
    static char **map = NULL;

    nsect = 0;
    if (!snxtsct(&ns, player->argp[1]))
	return RET_SYN;
    if (!mapbuf)
	mapbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!map) {
	map = malloc(WORLD_Y * sizeof(char *));
	if (map && mapbuf) {
	    for (i = 0; i < WORLD_Y; i++)
		map[i] = &mapbuf[MAPWIDTH(1) * i];
	} else if (map) {
	    free(map);
	    map = NULL;
	}
    }
    if (!mapbuf || !map) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in sect\n");
	return RET_FAIL;
    }
    np = getnatp(player->cnum);
    ncond = ns.ncond;
    memcpy(cond, ns.cond, sizeof(*cond) * ncond);
    ns.ncond = 0;
    xyrelrange(getnatp(player->cnum), &ns.range, &range);
    border(&range, "    ", "");
    blankfill(mapbuf, &ns.range, 1);
    while (nxtsct(&ns, &sect)) {
	if (!player->owner)
	    continue;
	ptr = &map[ns.dy][ns.dx];
	*ptr = dchr[sect.sct_type].d_mnem;
	if (nstr_exec(cond, ncond, &sect)) {
	    ++nsect;
	    *ptr |= 0x80;
	}
    }
    for (i = 0, y = ns.range.ly; i < ns.range.height; y++, i++) {
	yval = yrel(np, y);
	pr("%3d %s %-3d\n", yval, map[i], yval);
	if (y >= WORLD_Y)
	    y -= WORLD_Y;
    }
    border(&range, "    ", "");
    if (nsect == 0) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return RET_OK;
}
