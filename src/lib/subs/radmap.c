/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  radmap.c: Do a radar map given an x,y location, effic, and other
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include <config.h>

#include <stdlib.h>
#include "file.h"
#include "map.h"
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

static void radmap2(int, int, int, int, int, double, int);

void
radmap(int cx, int cy, int eff, int range, double seesub)
{
    radmap2(player->cnum, cx, cy, eff, range, seesub, 1);
}

/* More dynamic world sized buffers.  We create 'em once, and then
 * never again.  No need to keep creating/tearing apart.  We may
 * want to do this in other places too where it doesn't matter. */
static char **rad;
static char *radbuf;
static signed char **vis;
static signed char *visbuf;

static void
radmap2(int owner,
	int cx, int cy, int eff, int range, double seesub, int pr_flag)
{
    int visib, rng;
    struct sctstr sect;
    struct shpstr ship;
    struct plnstr plane;
    struct nstr_sect ns;
    struct nstr_item ni;
    int x, y;
    int row;
    int n;
    int changed = 0;

    if (!radbuf)
	radbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!visbuf)
	visbuf = malloc(WORLD_Y * MAPWIDTH(1));
    if (!rad) {
	rad = malloc(WORLD_Y * sizeof(char *));
	if (rad && radbuf) {
	    for (x = 0; x < WORLD_Y; x++)
		rad[x] = &radbuf[(WORLD_X + 1) * x];
	}
    }
    if (!vis) {
	vis = malloc(WORLD_Y * sizeof(signed char *));
	if (vis && visbuf) {
	    for (x = 0; x < WORLD_Y; x++)
		vis[x] = &visbuf[(WORLD_X + 1) * x];
	}
    }
    if (!radbuf || !visbuf || !rad || !vis) {
	pr("Memory error in radmap2, tell the deity.\n");
	return;
    }

    memset(visbuf, 0, (WORLD_Y * (WORLD_X + 1)));
    range = (int)(range * (eff / 100.0));
    if (range < 1)
	range = 1;
    if (pr_flag)
	pr("%s efficiency %d%%, max range %d\n",
	   xyas(cx, cy, owner), eff, range);
    snxtsct_dist(&ns, cx, cy, range);
    blankfill(radbuf, &ns.range, 1);
    while (nxtsct(&ns, &sect)) {
	if (sect.sct_own == owner
	    || sect.sct_type == SCT_WATER
	    || sect.sct_type == SCT_MOUNT
	    || sect.sct_type == SCT_WASTE
	    || ns.curdist <= range / 3)
	    rad[ns.dy][ns.dx] = dchr[sect.sct_type].d_mnem;
	else
	    rad[ns.dy][ns.dx] = '?';
	changed += map_set(owner, ns.x, ns.y, rad[ns.dy][ns.dx], 0);
    }
    if (changed)
	writemap(owner);
    if (!pr_flag)
	return;
    snxtitem_dist(&ni, EF_PLANE, cx, cy, range);
    while (nxtitem(&ni, &plane)) {
	if (plane.pln_own == 0)
	    continue;
	/* Used to have 'ghosts' when scanning whole world --ts */
	x = deltx(&ns.range, (int)plane.pln_x);
	y = delty(&ns.range, (int)plane.pln_y);

	if (pln_is_in_orbit(&plane) && plane.pln_own != owner) {
	    vis[y][x] = 100;
	    rad[y][x] = '$';
	}
    }
    snxtitem_dist(&ni, EF_SHIP, cx, cy, range);
    while (nxtitem(&ni, &ship)) {
	if (ship.shp_own == 0)
	    continue;
	/* Used to have 'ghosts' when scanning whole world --ts */
	x = deltx(&ns.range, (int)ship.shp_x);
	y = delty(&ns.range, (int)ship.shp_y);

	visib = shp_visib(&ship);
	rng = (int)(range * visib / 20.0);
	if (ni.curdist > rng)
	    continue;
	if ((mchr[(int)ship.shp_type].m_flags & M_SUB) &&
	    ni.curdist > rng * seesub)
	    continue;
	if (visib > vis[y][x]) {
	    vis[y][x] = visib;
	    /* &~0x20 makes it a cap letter */
	    rad[y][x] = (*mchr[(int)ship.shp_type].m_name) & ~0x20;
	}
    }
    /*
     * make the center of the display 0
     * so ve et al can find it.
     */
    rad[delty(&ns.range, cy)][deltx(&ns.range, cx)] = '0';

    n = ns.range.height;
    for (row = 0; row < n; row++)
	pr("%s\n", rad[row]);
    pr("\n");
}

/*
 * Return distance from left edge of R to X.
 * Value is between 0 (inclusive) and WORLD_X (exclusive).
 * X must be normalized.
 */
int
deltx(struct range *r, coord x)
{
    if (x >= r->lx)
	return x - r->lx;
    return x + WORLD_X - r->lx;
}

/*
 * Return distance from top edge of R to Y.
 * Value is between 0 (inclusive) and WORLD_Y (exclusive).
 * Y must be normalized.
 */
int
delty(struct range *r, coord y)
{
    if (y >= r->ly)
	return y - r->ly;
    return y + WORLD_Y - r->ly;
}

/*
 * Update OWNER's bmap for radar at CX,CY.
 * EFF is the radar's efficiency, and RANGE its range at 100%
 * efficiency.
 */
void
rad_map_set(natid owner, int cx, int cy, int eff, int range)
{
    struct nstr_sect ns;
    struct sctstr sect;
    int changed = 0;
    char ch;

    range = (int)(range * (eff / 100.0));
    if (range < 1)
	range = 1;
    snxtsct_dist(&ns, cx, cy, range);
    while (nxtsct(&ns, &sect)) {
	if (sect.sct_own == owner
	    || sect.sct_type == SCT_WATER
	    || sect.sct_type == SCT_MOUNT
	    || sect.sct_type == SCT_WASTE
	    || ns.curdist <= range / 3)
	    ch = dchr[sect.sct_type].d_mnem;
	else
	    ch = '?';
	changed += map_set(owner, ns.x, ns.y, ch, 0);
    }
    if (changed)
	writemap(owner);
}
