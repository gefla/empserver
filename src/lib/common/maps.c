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
 *  maps.c: Map routines
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "map.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "common.h"
#include "gen.h"
#include "subs.h"
#include "optlist.h"

static int bmnxtsct(struct nstr_sect *);
static char map_char(unsigned char type, natid own, int owner_or_god);

int
draw_map(int bmap, s_char origin, int map_flags, struct nstr_sect *nsp)
{
    struct natstr *np;
    struct range range;
    struct nstr_item ni;
    struct shpstr ship;
    struct lndstr land;
    struct plnstr plane;
    coord x, y;
    int i;
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static unsigned char *bitmap = NULL;
    static char *wmapbuf = NULL;
    static char **wmap = NULL;

    if (!wmapbuf)
	wmapbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!wmap) {
	wmap = malloc(WORLD_Y * sizeof(s_char *));
	if (wmap && wmapbuf) {
	    for (i = 0; i < WORLD_Y; i++)
		wmap[i] = &wmapbuf[MAPWIDTH(1) * i];
	} else if (wmap) {
	    free(wmap);
	    wmap = NULL;
	}
    }
    if (!bitmap)
	bitmap = malloc((WORLD_X * WORLD_Y) / 8);
    if (!wmapbuf || !wmap || !bitmap) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in draw_map\n");
	return RET_FAIL;
    }

    if (bmap == 'r') {
	if (!confirm("Are you sure you want to revert your bmap? "))
	    return RET_OK;
    }
    if (!(player->command->c_flags & C_MOD)) {
	logerror("%s command needs C_MOD flag set",
		 player->command->c_form);
	player->command->c_flags |= C_MOD;
    }
    np = getnatp(player->cnum);
    /* zap any conditionals */
    nsp->ncond = 0;
    xyrelrange(np, &nsp->range, &range);
    border(&range, "     ", "");
    blankfill(wmapbuf, &nsp->range, 1);
    if (bmap) {
	int c;
	switch (bmap) {
	default:
	    CANT_REACH();
	    bmap = 'b';
	    /* fall through */
	case 'b':
	    while (bmnxtsct(nsp) && !player->aborted) {
		if (0 != (c = player->bmap[sctoff(nsp->x, nsp->y)]))
		    wmap[nsp->dy][nsp->dx] = c;
	    }
	    break;
	case 't':
	    while (bmnxtsct(nsp) && !player->aborted) {
		if (0 != (c = player->map[sctoff(nsp->x, nsp->y)]))
		    wmap[nsp->dy][nsp->dx] = c;
	    }
	    break;
	case 'r':
	    while (bmnxtsct(nsp) && !player->aborted) {
		player->bmap[sctoff(nsp->x, nsp->y)] =
		    player->map[sctoff(nsp->x, nsp->y)];
		if (0 != (c = player->bmap[sctoff(nsp->x, nsp->y)]))
		    wmap[nsp->dy][nsp->dx] = c;
	    }
	    ef_write(EF_BMAP, player->cnum, player->bmap);
	    break;
	case 'n':
	    {
		struct sctstr sect;

		if (!player->god) {
		    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
		    bitinit2(nsp, bitmap, player->cnum);
		}
		while (nxtsct(nsp, &sect) && !player->aborted) {
		    if (!player->god && !emp_getbit(nsp->x, nsp->y, bitmap))
			continue;
		    wmap[nsp->dy][nsp->dx]
			= map_char(sect.sct_newtype, sect.sct_own,
				   player->owner);
		}
		break;
	    }
	}
    } else {
	struct sctstr sect;
	char mapch;
	int changed = 0;

	if (!player->god) {
	    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
	    bitinit2(nsp, bitmap, player->cnum);
	}
	while (nxtsct(nsp, &sect) && !player->aborted) {
	    if (!player->god && !emp_getbit(nsp->x, nsp->y, bitmap))
		continue;
	    mapch = map_char(sect.sct_type, sect.sct_own, player->owner);
	    wmap[nsp->dy][nsp->dx] = mapch;
	    changed |= map_set(player->cnum, nsp->x, nsp->y, mapch, 0);
	}
	if (changed)
	    writemap(player->cnum);
    }
    if (player->aborted)
	return RET_OK;
    if (map_flags & MAP_PLANE) {
	snxtitem_all(&ni, EF_PLANE);
	while (nxtitem(&ni, &plane)) {
	    if (plane.pln_own == 0)
		continue;
	    if (plane.pln_own != player->cnum && !player->god)
		continue;
	    if (!xyinrange(plane.pln_x, plane.pln_y, &nsp->range))
		continue;

	    x = xnorm(plane.pln_x - nsp->range.lx);
	    y = ynorm(plane.pln_y - nsp->range.ly);
	    wmap[y][x] = (*plchr[(int)plane.pln_type].pl_name) & ~0x20;
	}
    }
    if (map_flags & MAP_SHIP) {
	snxtitem_all(&ni, EF_SHIP);
	while (nxtitem(&ni, &ship)) {
	    if (ship.shp_own == 0)
		continue;
	    if (ship.shp_own != player->cnum && !player->god)
		continue;
	    if (!xyinrange(ship.shp_x, ship.shp_y, &nsp->range))
		continue;

	    x = xnorm(ship.shp_x - nsp->range.lx);
	    y = ynorm(ship.shp_y - nsp->range.ly);
	    wmap[y][x] = (*mchr[(int)ship.shp_type].m_name) & ~0x20;
	}
    }
    if (map_flags & MAP_LAND) {
	snxtitem_all(&ni, EF_LAND);
	while (nxtitem(&ni, &land)) {
	    if (land.lnd_own == 0)
		continue;
	    if (land.lnd_own != player->cnum && !player->god)
		continue;
	    if (!xyinrange(land.lnd_x, land.lnd_y, &nsp->range))
		continue;

	    x = xnorm(land.lnd_x - nsp->range.lx);
	    y = ynorm(land.lnd_y - nsp->range.ly);
	    wmap[y][x] = (*lchr[(int)land.lnd_type].l_name) & ~0x20;
	}
    }
    if (map_flags & MAP_HIGH) {
	char *ptr;
	struct sctstr sect;

	snxtsct_rewind(nsp);
	if (!player->god) {
	    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
	    bitinit2(nsp, bitmap, player->cnum);
	}
	while (nxtsct(nsp, &sect) && !player->aborted) {
	    if (!player->god && !emp_getbit(nsp->x, nsp->y, bitmap))
		continue;
	    ptr = &wmap[nsp->dy][nsp->dx];
	    if (sect.sct_own == player->cnum)
		 *ptr |= 0x80;
	}
    }
    if (origin)
	wmap[5][10] = origin & ~0x20;
    for (y = nsp->range.ly, i = 0; i < nsp->range.height; y++, i++) {
	int yval;

	yval = yrel(np, y);
	wmap[i][nsp->range.width] = '\0';
	pr("%4d %s %-4d\n", yval, wmap[i], yval);
	if (y >= WORLD_Y)
	    y -= WORLD_Y;
    }
    border(&range, "     ", "");
    return RET_OK;
}

/*
 * get the next sector in the range
 */
static int
bmnxtsct(struct nstr_sect *np)
{
    while (1) {
	np->dx++;
	np->x++;
	if (np->x >= WORLD_X)
	    np->x = 0;
	if (np->dx >= np->range.width) {
	    np->dx = 0;
	    np->x = np->range.lx;
	    np->dy++;
	    if (np->dy >= np->range.height)
		return 0;
	    np->y++;
	    if (np->y >= WORLD_Y)
		np->y = 0;
	}
	if ((np->y + np->x) & 01)
	    continue;
	if (np->type == NS_DIST) {
	    np->curdist = mapdist(np->x, np->y, np->cx, np->cy);
	    if (np->curdist > np->dist)
		continue;
	}
	np->id = sctoff(np->x, np->y);
	return 1;
    }
    /*NOTREACHED*/
}

/*
 * Return character to use in maps for sector type TYPE owned by OWN.
 * If OWNER_OR_GOD, the map is for the sector's owner or a deity.
 */
static char
map_char(unsigned char type, natid own, int owner_or_god)
{
    if (type > SCT_MAXDEF) {
	logerror("bad sector type %d\n", type);
	return '?';
    }
    if (owner_or_god
	|| type == SCT_WATER || type == SCT_MOUNT || type == SCT_WASTE
	|| (!own && (type == SCT_RURAL || type == SCT_PLAINS)))
	return dchr[type].d_mnem;
    return '?';
}

int
unit_map(int unit_type, int uid, struct nstr_sect *nsp, s_char *originp)
{
    struct shpstr origs;
    struct lndstr origl;
    struct plnstr origp;
    struct genitem *gp;
    struct range range;

    if (unit_type == EF_LAND) {
	if (!getland(uid, &origl) || !player->owner || origl.lnd_own == 0)
	    return RET_FAIL;
	gp = (struct genitem *)&origl;
	*originp = *lchr[(int)origl.lnd_type].l_name;
    } else if (unit_type == EF_PLANE) {
	if (!getplane(uid, &origp) || !player->owner || origp.pln_own == 0)
	    return RET_FAIL;
	gp = (struct genitem *)&origp;
	*originp = *plchr[(int)origp.pln_type].pl_name;
    } else {
	if (!getship(uid, &origs) || !player->owner || origs.shp_own == 0)
	    return RET_FAIL;
	gp = (struct genitem *)&origs;
	*originp = *mchr[(int)origs.shp_type].m_name;
    }

    range.lx = xnorm(gp->x - 10);
    range.hx = xnorm(gp->x + 11);
    range.ly = ynorm(gp->y - 5);
    range.hy = ynorm(gp->y + 6);
    xysize_range(&range);
    snxtsct_area(nsp, &range);
    return RET_OK;
}

int
bmaps_intersect(natid a, natid b)
{
    s_char *mapa = ef_ptr(EF_MAP, a);
    s_char *mapb = ef_ptr(EF_MAP, b);
    int i;

    for (i = 0; i < WORLD_X * WORLD_Y / 2; ++i, ++mapa, ++mapb)
	if (*mapa && *mapa != ' ' && *mapb && *mapb != ' ')
	    return 1;
    return 0;
}

/* Note that this requires that the BMAP is mapped into memory */

int
share_bmap(natid from, natid to, struct nstr_sect *ns, char des,
	   char *from_name)
{
    char *from_bmap = ef_ptr(EF_BMAP, from);
    char *to_bmap = ef_ptr(EF_BMAP, to);
    int n = 0;
    struct sctstr sect;
    char fromdes;
    char todes;
    char from_des = *from_name;

    if (isalpha(from_des))
	from_des &= ~0x20;

    while (nxtsct(ns, &sect)) {
	if (!(fromdes = from_bmap[sctoff(ns->x, ns->y)]))
	    continue;
	todes = to_bmap[sctoff(ns->x, ns->y)];
	if (todes &&
	    todes != '?' &&
	    todes != '.' && todes != ' ' && todes != from_des)
	    continue;
	if (sect.sct_own == from) {
	    if (fromdes != '=' && fromdes != 'h' && fromdes != des)
		fromdes = from_des;
	}
	if (todes == fromdes)
	    continue;
	n += map_set(to, ns->x, ns->y, fromdes, 1);
    }

    if (n)
	writebmap(to);
    return n;
}
