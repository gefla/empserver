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
 *  maps.c: Map routines
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "var.h"
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

static int bmnxtsct(register struct nstr_sect *);

int
draw_map(int bmap, s_char origin, int map_flags, struct nstr_sect *nsp,
	 int country)
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
    static u_char *bitmap = (u_char *)0;
    static s_char *wmapbuf = (s_char *)0;
    static s_char **wmap = (s_char **)0;

    if (!wmapbuf)
	wmapbuf =
	    (s_char *)malloc((WORLD_Y * MAPWIDTH(1)) * sizeof(s_char));
    if (!wmap) {
	wmap = (s_char **)malloc(WORLD_Y * sizeof(s_char *));
	if (wmap && wmapbuf) {
	    for (i = 0; i < WORLD_Y; i++)
		wmap[i] = &wmapbuf[MAPWIDTH(1) * i];
	} else if (wmap) {
	    free((s_char *)wmap);
	    wmap = (s_char **)0;
	}
    }
    if (!bitmap)
	bitmap = (u_char *)malloc((WORLD_X * WORLD_Y) / 8);
    if (!wmapbuf || !wmap || !bitmap) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in draw_map\n");
	return RET_FAIL;
    }

    if (bmap == EF_MAP + EF_BMAP) {
	if (!confirm("Are you sure you want to revert your bmap? "))
	    return RET_OK;
    }
    if (!player->command->c_flags & C_MOD) {
	logerror("%s command needs C_MOD flag set",
		 player->command->c_form);
	player->command->c_flags |= C_MOD;
    }
    np = getnatp(country);
    /* zap any conditionals */
    nsp->ncond = 0;
    xyrelrange(np, &nsp->range, &range);
    border(&range, "     ", "");
    blankfill((s_char *)wmapbuf, &nsp->range, 1);
    if (bmap) {
	register int c;
	switch (bmap) {
	case EF_BMAP:
	    while (bmnxtsct(nsp) && !player->aborted) {
		if (0 != (c = player->bmap[sctoff(nsp->x, nsp->y)]))
		    wmap[nsp->dy][nsp->dx] = c;
	    }
	    break;
	case EF_MAP:
	    while (bmnxtsct(nsp) && !player->aborted) {
		if (0 != (c = player->map[sctoff(nsp->x, nsp->y)]))
		    wmap[nsp->dy][nsp->dx] = c;
	    }
	    break;
	case (EF_MAP + EF_BMAP):
	    while (bmnxtsct(nsp) && !player->aborted) {
		player->bmap[sctoff(nsp->x, nsp->y)] =
		    player->map[sctoff(nsp->x, nsp->y)];
		if (0 != (c = player->bmap[sctoff(nsp->x, nsp->y)]))
		    wmap[nsp->dy][nsp->dx] = c;
	    }
	    ef_write(EF_BMAP, player->cnum, player->bmap);
	    break;
	case EF_NMAP:
	    do {
		register s_char *ptr;
		struct sctstr sect;

		if ((!player->god || country)) {
		    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
		    bitinit2(nsp, bitmap, country);
		}
		while (nxtsct(nsp, &sect) && !player->aborted) {
		    if ((!player->god || country) &&
			!emp_getbit(nsp->x, nsp->y, bitmap)) {
			if (!player->god)
			    continue;
		    }
		    ptr = &wmap[nsp->dy][nsp->dx];
		    if (sect.sct_newtype > SCT_MAXDEF) {
			*ptr = '?';
		    } else {
			*ptr = dchr[sect.sct_newtype].d_mnem;
			switch (sect.sct_newtype) {
			case SCT_WATER:
			case SCT_RURAL:
			case SCT_MOUNT:
			case SCT_WASTE:
			case SCT_PLAINS:
			    break;
			default:
			    if (sect.sct_own != country &&
				(!player->god || country)) {
				if (!player->god)
				    *ptr = '?';
			    }
			    break;
			}
		    }
		}
	    } while (0);
	    break;
	}
    } else {
	register s_char *ptr;
	struct sctstr sect;
	int changed = 0;

	if ((!player->god || country)) {
	    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
	    bitinit2(nsp, bitmap, country);
	}
	while (nxtsct(nsp, &sect) && !player->aborted) {
	    if ((!player->god || country)
		&& !emp_getbit(nsp->x, nsp->y, bitmap)) {
		if (!player->god)
		    continue;
	    }
	    ptr = &wmap[nsp->dy][nsp->dx];
	    if (sect.sct_type > SCT_MAXDEF) {
		*ptr = '?';
	    } else {
		*ptr = dchr[sect.sct_type].d_mnem;
		switch (sect.sct_type) {
		case SCT_WATER:
		case SCT_RURAL:
		case SCT_MOUNT:
		case SCT_WASTE:
		case SCT_PLAINS:
		    break;
		default:
		    if (sect.sct_own != country &&
			(!player->god || country)) {
			if (!player->god)
			    *ptr = '?';
		    }
		    break;
		}
		changed += map_set(player->cnum, nsp->x, nsp->y, *ptr, 0);
	    }
	}
	if (changed)
	    writemap(player->cnum);
    }
    if (player->aborted)
	return RET_OK;
    if (map_flags & MAP_PLANE) {
	snxtitem_all(&ni, EF_PLANE);
	while (nxtitem(&ni, (caddr_t)&plane)) {
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
	while (nxtitem(&ni, (caddr_t)&ship)) {
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
	while (nxtitem(&ni, (caddr_t)&land)) {
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
	register s_char *ptr;
	struct sctstr sect;

	snxtsct_rewind(nsp);
	if ((!player->god || country)) {
	    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
	    bitinit2(nsp, bitmap, country);
	}
	while (nxtsct(nsp, &sect) && !player->aborted) {
	    if ((!player->god || country) &&
		!emp_getbit(nsp->x, nsp->y, bitmap)) {
		if (!player->god)
		    continue;
	    }
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
bmnxtsct(register struct nstr_sect *np)
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

int
unit_map(int unit_type, int i, struct nstr_sect *nsp, s_char *originp)
{
    struct shpstr origs;
    struct lndstr origl;
    struct plnstr origp;
    s_char what[64];
    struct natstr *np;

    np = getnatp(player->cnum);
    if (unit_type == EF_LAND) {
	if (!getland(i, &origl) ||
	    (origl.lnd_own != player->cnum && !player->god) ||
	    (origl.lnd_own == 0))
	    return RET_FAIL;
	sprintf(what, "%d:%d,%d:%d", xrel(np, origl.lnd_x - 10),
		xrel(np, origl.lnd_x + 10),
		yrel(np, origl.lnd_y - 5), yrel(np, origl.lnd_y + 5));
	*originp = *lchr[(int)origl.lnd_type].l_name;
    } else if (unit_type == EF_PLANE) {
	if (!getplane(i, &origp) ||
	    (origp.pln_own != player->cnum && !player->god) ||
	    (origp.pln_own == 0))
	    return RET_FAIL;
	sprintf(what, "%d:%d,%d:%d", xrel(np, origp.pln_x - 10),
		xrel(np, origp.pln_x + 10),
		yrel(np, origp.pln_y - 5), yrel(np, origp.pln_y + 5));
	*originp = *plchr[(int)origp.pln_type].pl_name;
    } else {
	if (!getship(i, &origs) ||
	    (origs.shp_own != player->cnum && !player->god) ||
	    (origs.shp_own == 0))
	    return RET_FAIL;
	sprintf(what, "%d:%d,%d:%d", xrel(np, origs.shp_x - 10),
		xrel(np, origs.shp_x + 10),
		yrel(np, origs.shp_y - 5), yrel(np, origs.shp_y + 5));
	unit_type = EF_SHIP;
	*originp = *mchr[(int)origs.shp_type].m_name;
    }
    if (!snxtsct(nsp, what))
	return RET_FAIL;
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
share_bmap(natid from, natid to, struct nstr_sect *ns, s_char des,
	   s_char *from_name)
{
    s_char *from_bmap = ef_ptr(EF_BMAP, from);
    s_char *to_bmap = ef_ptr(EF_BMAP, to);
    int n = 0;
    struct sctstr sect;
    s_char fromdes;
    s_char todes;
    s_char from_des = *from_name;

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
