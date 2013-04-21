/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  maps.c: Map routines
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2011
 *     Ron Koenderink, 2006
 */

#include <config.h>

#include <ctype.h>
#include "com.h"
#include "empobj.h"
#include "file.h"
#include "land.h"
#include "map.h"
#include "match.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

/* Flags for draw_map() */
/* whether to put ships, planes, land units or nukes on the map */
#define MAP_SHIP	bit(0)
#define MAP_PLANE	bit(1)
#define MAP_LAND	bit(2)
#define MAP_NUKE	bit(3)
#define MAP_ALL		(MAP_SHIP | MAP_PLANE | MAP_LAND | MAP_NUKE)
/* whether to highlight own sectors */
#define MAP_HIGH	bit(4)
/* whether to draw a map or a bmap */
#define MAP_BMAP	bit(5)
/* whether to draw an alternate map: newdes for map, true bmap for bmap */
#define MAP_ALT		bit(6)
/* whether to revert bmap, internal to do_map() */
#define MAP_BMAP_REVERT	bit(7)

static int parse_map_arg(int, char *, struct nstr_sect *, char *);
static int parse_map_flags(int, char *);
static int revert_bmap(struct nstr_sect *);
static int draw_map(char, int, struct nstr_sect *);
static int bmnxtsct(struct nstr_sect *);
static char map_char(int, natid, int);
static int unit_map(int, int, struct nstr_sect *, char *);
static void snxtsct_around(struct nstr_sect *, coord, coord);

int
do_map(int bmap, int unit_type, char *arg1, char *arg2)
{
    struct nstr_sect ns;
    char origin;
    int res, map_flags;

    res = parse_map_arg(unit_type, arg1, &ns, &origin);
    if (res != RET_OK)
	return res;

    map_flags = parse_map_flags(bmap, arg2);
    if (map_flags < 0)
	return RET_SYN;

    if (map_flags & MAP_BMAP_REVERT)
	return revert_bmap(&ns);
    return draw_map(origin, map_flags, &ns);
}

static int
parse_map_arg(int unit_type, char *arg,
	      struct nstr_sect *nsp, char *originp)
{
    switch (sarg_type(arg)) {
    case NS_DIST:
    case NS_AREA:
    case NS_ALL:
	if (!snxtsct(nsp, arg))
	    return RET_SYN;
	*originp = 0;
	break;
    default:
	if (unit_map(unit_type, atoi(arg), nsp, originp) < 0) {
	    pr("No such %s\n", ef_nameof(unit_type));
	    return RET_FAIL;
	}
    }
    return RET_OK;
}

static void
warn_deprecated_arg(char *what, char *arg, char *use)
{
    pr("%s '%s' is deprecated and will go away in a future release.\n"
       "Use %s instead.\n",
       what, arg, use);
}

static int
parse_map_flags(int bmap, char *str)
{
    int map_flags;
    char *p;
    int tflags = 0;
    char *tp = NULL;

    switch (bmap) {
    default: CANT_REACH();
	/* fall through */
    case 'b': map_flags = MAP_BMAP; break;
    case 'n': map_flags = MAP_ALT; break;
    case 0:   map_flags = 0;
    }

    if (!str || !*str)
	return map_flags;

    /* special case "revert" */
    if (bmap == 'b' && mineq(str, "revert") != ME_MISMATCH)
	return MAP_BMAP_REVERT;

    for (p = str; *p; p++) {
	switch (*p) {
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
	    map_flags |= MAP_ALT;
	    /*
	     * Flags following 't' used to be ignored.  That breaks
	     * perfectly sensible "ts".  Try to continue, but save
	     * state for when a bad flag is found.
	     */
	    if (!tflags) {
		tflags = map_flags;
		tp = p;
	    }
	    break;
	case 'r':
	    if (bmap != 'b' || tflags)
		goto bad_flag;
	    warn_deprecated_arg("Map flag", "r", "argument 'revert'");
	    return MAP_BMAP_REVERT;
	default:
	bad_flag:
	    if (tflags) {
		/* ignore bad flags following 't' */
		warn_deprecated_arg("Argument", tp, "map flag 't'");
		return tflags;
	    }
	    pr("Bad flag %c!\n", *p);
	    return -1;
	}
    }

    return map_flags;
}

static int
revert_bmap(struct nstr_sect *nsp)
{
    if (!confirm("Are you sure you want to revert your bmap? "))
	return RET_FAIL;
    while (bmnxtsct(nsp))
	player->bmap[nsp->id] = player->map[nsp->id];
    ef_write(EF_BMAP, player->cnum, player->bmap);
    return RET_OK;
}

static int
draw_map(char origin, int map_flags, struct nstr_sect *nsp)
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
	bitmap = malloc((WORLD_SZ() + 7) / 8);
    if (!wmapbuf || !wmap || !bitmap) {
	pr("Memory error, tell the deity.\n");
	logerror("malloc failed in draw_map\n");
	return RET_FAIL;
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

    if (map_flags & MAP_BMAP) {
	char *map = map_flags & MAP_ALT ? player->map : player->bmap;

	while (bmnxtsct(nsp)) {
	    if (map[nsp->id])
		wmap[nsp->dy][nsp->dx] = map[nsp->id];
	}
    } else {
	struct sctstr sect;
	char mapch;
	int changed = 0;

	if (!player->god) {
	    memset(bitmap, 0, (WORLD_SZ() + 7) / 8);
	    bitinit2(nsp, bitmap, player->cnum);
	}

	while (nxtsct(nsp, &sect)) {
	    if (!player->god && !emp_getbit(nsp->x, nsp->y, bitmap))
		continue;
	    mapch = map_char(map_flags & MAP_ALT
			     ? sect.sct_newtype : sect.sct_type,
			     sect.sct_own, player->owner);
	    wmap[nsp->dy][nsp->dx] = mapch;
	    if (!(map_flags & MAP_ALT))
		changed |= map_set(player->cnum, nsp->x, nsp->y, mapch, 0);
	}
	if (changed)
	    writemap(player->cnum);
    }

    i = 0;
    while (ef_mappable[i] != EF_BAD) {
	if (map_flags & ef_unit_map[i]) {
	    snxtitem_area(&ni, ef_mappable[i], &nsp->range);
	    while (nxtitem(&ni, &unit)) {
		if (unit.gen.own == 0)
		    continue;
		if (unit.gen.own != player->cnum && !player->god)
		    continue;

		x = deltx(&nsp->range, unit.gen.x);
		y = delty(&nsp->range, unit.gen.y);

		if (ef_mappable[i] == EF_NUKE)
		    wmap[y][x] = 'N';
		else {
		    name = empobj_chr_name(&unit.gen);
		    wmap[y][x] = *name & ~0x20;
		}
	    }
	}
	i++;
    }
    if (map_flags & MAP_HIGH) {
	struct sctstr sect;

	snxtsct_rewind(nsp);
	while (nxtsct(nsp, &sect)) {
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
    char *name;

    if (CANT_HAPPEN((ef_flags(unit_type) & (EFF_OWNER | EFF_XY))
		    != (EFF_OWNER | EFF_XY)))
	return -1;

    if (!get_empobj(unit_type, uid, &unit))
	return -1;
    if (!player->owner || unit.gen.own == 0)
	return -1;

    if (unit_type == EF_NUKE)
	*originp = 'n';
    else {
	name = empobj_chr_name(&unit.gen);
	*originp = *name;
    }

    snxtsct_around(nsp, unit.gen.x, unit.gen.y);
    return 0;
}

static void
snxtsct_around(struct nstr_sect *nsp, coord x, coord y)
{
    struct range range;

    range.lx = xnorm(x - 10);
    range.hx = xnorm(x + 10);
    range.ly = ynorm(y - 5);
    range.hy = ynorm(y + 5);
    xysize_range(&range);
    snxtsct_area(nsp, &range);
}

int
display_region_map(int bmap, int unit_type, coord curx, coord cury,
		   char *arg1, char *arg2)
{
    struct nstr_sect ns;
    char origin;
    int res, map_flags;

    if (arg1 && *arg1) {
	res = parse_map_arg(unit_type, arg1, &ns, &origin);
	if (res != RET_OK)
	    return res;

	map_flags = parse_map_flags(bmap, arg2);
	if (map_flags < 0)
	    return RET_SYN;
    } else {
	snxtsct_around(&ns, curx, cury);
	map_flags = 0;
	origin = 0;
    }

    if (map_flags & MAP_BMAP_REVERT)
	return revert_bmap(&ns);
    return draw_map(origin, map_flags, &ns);
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

    if (from == to)
	return 0;

    if (isalpha(from_des))
	from_des &= ~0x20;

    while (nxtsct(ns, &sect)) {
	if (!(fromdes = from_bmap[sect.sct_uid]))
	    continue;
	todes = to_bmap[sect.sct_uid];
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
