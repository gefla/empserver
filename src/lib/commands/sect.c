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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  sect.c: Show sectors in map-like format with conditionals.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "map.h"
#include "commands.h"
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
    s_char *ptr;
    coord y, yval;
    int i;
    s_char what[64];
    s_char *str;
    s_char buf[1024];
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static s_char *mapbuf = (s_char *)0;
    static s_char **map = (s_char **)0;

    nsect = 0;
    if (player->argp[1] == NULL) {
	if ((str = getstring("(sects)? ", buf)) == 0)
	    return RET_SYN;
    } else {
	str = player->argp[1];
    }

    if (*str == '*') {
	sprintf(what, "%d:%d,%d:%d",
		-WORLD_X / 2, WORLD_X / 2 - 1,
		-WORLD_Y / 2, WORLD_Y / 2 - 1);
	if (!snxtsct(&ns, what))
	    return RET_FAIL;
    } else if (!snxtsct(&ns, str))
	return RET_SYN;
    if (!mapbuf)
	mapbuf = malloc((WORLD_Y * MAPWIDTH(1)) * sizeof(s_char));
    if (!map) {
	map = malloc(WORLD_Y * sizeof(s_char *));
	if (map && mapbuf) {
	    for (i = 0; i < WORLD_Y; i++)
		map[i] = &mapbuf[MAPWIDTH(1) * i];
	} else if (map) {
	    free(map);
	    map = (s_char **)0;
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
    blankfill((s_char *)mapbuf, &ns.range, 1);
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
