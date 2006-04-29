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
 *  rout.c: Show empire deliver routes
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "nat.h"
#include "nsc.h"
#include "sect.h"
#include "item.h"
#include "path.h"
#include "file.h"
#include "map.h"
#include "commands.h"
#include "optlist.h"

int
rout(void)
{
    struct ichrstr *ip;
    struct nstr_sect ns;
    struct natstr *natp;
    struct sctstr sect;
    struct nscstr cond[NS_NCOND];
    int ncond;
    struct range relrange;
    int row;
    int y;
    int ry;
    i_type i_del;
    int dir;
    char *p;
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static char *mapbuf = NULL;
    static char **map = NULL;
    static char *buf = NULL;
    int i;

    if ((ip = whatitem(player->argp[1], "What item? ")) == 0)
	return RET_SYN;
    i_del = ip->i_uid;;
    if (!snxtsct(&ns, player->argp[2]))
	return RET_FAIL;
    if (!mapbuf)
	mapbuf = malloc(WORLD_Y * MAPWIDTH(3));
    if (!map) {
	map = malloc(WORLD_Y * sizeof(s_char *));
	if (map && mapbuf) {
	    for (i = 0; i < WORLD_Y; i++)
		map[i] = &mapbuf[MAPWIDTH(3) * i];
	} else if (map) {
	    free(map);
	    map = NULL;
	}
    }
    if (!buf)
	buf = malloc(MAPWIDTH(3) + 12);
    if (!mapbuf || !map || !buf) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in rout\n");
	return RET_FAIL;
    }
    ncond = ns.ncond;
    memcpy(cond, ns.cond, sizeof(struct nscstr) * ncond);
    ns.ncond = 0;

    natp = getnatp(player->cnum);
    xyrelrange(natp, &ns.range, &relrange);
    memset(mapbuf, 0, ((WORLD_Y * MAPWIDTH(3))));
    blankfill(mapbuf, &ns.range, 3);
    border(&relrange, "     ", " ");

    while (nxtsct(&ns, &sect)) {
	if (!player->owner)
	    continue;
	p = &map[ns.dy][ns.dx * 2];
	if ((dir = sect.sct_del[i_del] & 0x7) &&
	    nstr_exec(cond, ncond, &sect))
	    memcpy(p, routech[dir][0], 3);
	p[1] = dchr[sect.sct_type].d_mnem;
    }
    for (row = 0, y = ns.range.ly; row < ns.range.height; y++, row++) {
	ry = yrel(natp, y);
	memset(buf, 0, (MAPWIDTH(3) + 10));
	sprintf(buf, "%4d ", ry);
	memcpy(buf + 5, map[row], ns.range.width * 2 + 1);
	sprintf(buf + 5 + ns.range.width * 2 + 1, " %-4d\n", ry);
	pr("%s", buf);
	if (y >= WORLD_Y)
	    y -= WORLD_Y;
    }
    border(&relrange, "     ", " ");
    return RET_OK;
}
