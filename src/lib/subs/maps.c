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
 *  maps.c: Map routines
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 *     Ron Koenderink, 2006
 */

#include <config.h>

#include <ctype.h>
#include "com.h"
#include "empobj.h"
#include "file.h"
#include "land.h"
#include "map.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

static int bmnxtsct(struct nstr_sect *);
static char map_char(int, natid, int);
static int unit_map(int, int, struct nstr_sect *, char *);

int
do_map(int bmap, int unit_type, char *arg, char *map_flags_arg)
{
    struct nstr_sect ns;
    char origin = '\0';
    char *b;
    int map_flags = 0;

    if (!snxtsct(&ns, arg)) {
	if (unit_map(unit_type, atoi(arg), &ns, &origin))
	    return RET_FAIL;
    }
    for (b = map_flags_arg; b && *b; b++) {
	switch (*b) {
	case 's':
	case 'S':
	    map_flags |= MAP_SHIP;
	    break;
	case 'l':
	case 'L':
	    map_flags |= MAP_LAND;
	    break;
	case 'p':
	case 'P':
	    map_flags |= MAP_PLANE;
	    break;
	case 'n':
	case 'N':
	    map_flags |= MAP_NUKE;
	    break;
	case 'h':
	case 'H':
	    map_flags |= MAP_HIGH;
	    break;
	case '*':
	    map_flags |= MAP_ALL;
	    break;
	case 't':
	    if (bmap != 'b')
		goto bad_flag;
	    bmap = 't';
	    *(b + 1) = 0;
	    break;
	case 'r':
	    if (bmap != 'b')
		goto bad_flag;
	    bmap = 'r';
	    *(b + 1) = 0;
	    break;
	default:
	bad_flag:
	    pr("Bad flag %c!\n", *b);
	    break;
	}
    }
    return draw_map(bmap, origin, map_flags, &ns);
}

int
draw_map(int bmap, char origin, int map_flags, struct nstr_sect *nsp)
{
    struct natstr *np;
    struct range range;
    struct nstr_item ni;
    union empobj_storage unit;
    coord x, y;
    int i;
    /* Note this is not re-entrant anyway, so we keep the buffers
       around */
    static unsigned char *bitmap = NULL;
    static char *wmapbuf = NULL;
    static char **wmap = NULL;
    static int ef_mappable[] = { EF_PLANE, EF_SHIP, EF_LAND, EF_NUKE, EF_BAD };
    static int ef_unit_map[] = { MAP_PLANE, MAP_SHIP, MAP_LAND, MAP_NUKE };
    char *name;

    if (!wmapbuf)
	wmapbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!wmap) {
	wmap = malloc(WORLD_Y * sizeof(char *));
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

    i = 0;
    while (ef_mappable[i] != EF_BAD) {
	if (map_flags & ef_unit_map[i]) {
	    snxtitem_all(&ni, ef_mappable[i]);
	    while (nxtitem(&ni, &unit)) {
		if (unit.gen.own == 0)
		    continue;
		if (unit.gen.own != player->cnum && !player->god)
		    continue;
		if (!xyinrange(unit.gen.x, unit.gen.y, &nsp->range))
		    continue;

		x = xnorm(unit.gen.x - nsp->range.lx);
		y = ynorm(unit.gen.y - nsp->range.ly);

		if (ef_mappable[i] == EF_NUKE)
		    wmap[y][x] = 'N';
		else {
		    if ((name = emp_obj_chr_name(&unit.gen)) == NULL)
			return RET_FAIL;
		    wmap[y][x] = *name & ~0x20;
		}
	    }
	}
	i++;
    }
    if (map_flags & MAP_HIGH) {
	struct sctstr sect;

	snxtsct_rewind(nsp);
	if (!player->god) {
	    memset(bitmap, 0, (WORLD_X * WORLD_Y) / 8);
	    bitinit2(nsp, bitmap, player->cnum);
	}
	while (nxtsct(nsp, &sect) && !player->aborted) {
	    if (!player->god && !emp_getbit(nsp->x, nsp->y, bitmap))
		continue;
	    if (sect.sct_own == player->cnum)
		 wmap[nsp->dy][nsp->dx] |= 0x80;
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
map_char(int type, natid own, int owner_or_god)
{
    if (CANT_HAPPEN(type > SCT_TYPE_MAX || !dchr[type].d_mnem))
	return '?';
    if (owner_or_god
	|| type == SCT_WATER || type == SCT_MOUNT || type == SCT_WASTE
	|| (!own && (type == SCT_RURAL || type == SCT_PLAINS)))
	return dchr[type].d_mnem;
    return '?';
}

static int
unit_map(int unit_type, int uid, struct nstr_sect *nsp, char *originp)
{
    union empobj_storage unit;
    struct range range;
    char *name;

    if (!get_empobj(unit_type, uid, &unit))
	return RET_FAIL;
    if (!player->owner || unit.gen.own == 0)
	return RET_FAIL;

    if (unit_type == EF_NUKE)
	*originp = 'n';
    else {
	if ((name = emp_obj_chr_name(&unit.gen)) == NULL)
	    return RET_FAIL;
	*originp = *name;
    }

    range.lx = xnorm(unit.gen.x - 10);
    range.hx = xnorm(unit.gen.x + 11);
    range.ly = ynorm(unit.gen.y - 5);
    range.hy = ynorm(unit.gen.y + 6);
    xysize_range(&range);
    snxtsct_area(nsp, &range);
    return RET_OK;
}

int
display_region_map(int bmap, int unit_type, coord curx, coord cury,
		   char *arg)
{
    char coordinates[80];
    char *map_flag_arg;

    if (!arg || !*arg) {
	struct natstr *np;

	np = getnatp(player->cnum);
	sprintf(coordinates, "%d:%d,%d:%d",
	    xrel(np, curx - 10), xrel(np, curx + 11),
	    yrel(np, cury - 5), yrel(np, cury + 6));
	arg = coordinates;
	map_flag_arg = NULL;
    } else {
	map_flag_arg = strchr(arg, ' ');
	if (map_flag_arg != NULL) {
	    *map_flag_arg++  = '\0';
	    while (isspace(*map_flag_arg)) map_flag_arg++;
	}
    }
    player->condarg = NULL;
    return do_map(bmap, unit_type, arg, map_flag_arg);
}

int
bmaps_intersect(natid a, natid b)
{
    char *mapa = ef_ptr(EF_MAP, a);
    char *mapb = ef_ptr(EF_MAP, b);
    int i;

    for (i = 0; i < WORLD_SZ(); i++)
	if (mapa[i] && mapa[i] != ' ' && mapb[i] && mapb[i] != ' ')
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
