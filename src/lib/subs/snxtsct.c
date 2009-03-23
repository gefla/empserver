/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  snxtsct.c: Arrange sector selection using either distance or area
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Markus Armbruster, 2006-2008
 */

#include <config.h>

#include "file.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "xy.h"

/*
 * setup the nstr_sect structure for sector selection.
 * can select on either NS_ALL, NS_AREA, or NS_RANGE
 * iterate thru the "condarg" string looking
 * for arguments to compile into the nstr.
 * Using this function for anything but command arguments is usually
 * incorrect, because it respects conditionals.  Use the snxtsct_FOO()
 * instead.
 */
int
snxtsct(struct nstr_sect *np, char *str)
{
    struct range range;
    struct natstr *natp;
    coord cx, cy;
    int dist, n;
    char buf[1024];

    if (!str || !*str) {
	if (!(str = getstring("(sects)? ", buf)))
	    return 0;
    }
    switch (sarg_type(str)) {
    case NS_AREA:
	if (!sarg_area(str, &range))
	    return 0;
	snxtsct_area(np, &range);
	break;
    case NS_DIST:
	if (!sarg_range(str, &cx, &cy, &dist))
	    return 0;
	snxtsct_dist(np, cx, cy, dist);
	break;
    case NS_ALL:
	/*
	 * Can't use snxtsct_all(), as it would disclose the real
	 * origin.  Use a world-sized area instead.
	 */
	natp = getnatp(player->cnum);
	range.lx = xabs(natp, -WORLD_X / 2);
	range.ly = yabs(natp, -WORLD_Y / 2);
	range.hx = xnorm(range.lx + WORLD_X - 1);
	range.hy = ynorm(range.ly + WORLD_Y - 1);
	xysize_range(&range);
	snxtsct_area(np, &range);
	break;
    default:
	return 0;
    }
    if (!player->condarg)
	return 1;
    n = nstr_comp(np->cond, sizeof(np->cond) / sizeof(*np->cond),
		  EF_SECTOR, player->condarg);
    np->ncond = n >= 0 ? n : 0;
    return n >= 0;
}

void
snxtsct_all(struct nstr_sect *np)
{
    struct range worldrange;

    worldrange.lx = 0;
    worldrange.ly = 0;
    worldrange.hx = WORLD_X - 1;
    worldrange.hy = WORLD_Y - 1;
    xysize_range(&worldrange);
    snxtsct_area(np, &worldrange);
}

void
snxtsct_area(struct nstr_sect *np, struct range *range)
{
    memset(np, 0, sizeof(*np));
    np->range = *range;
    np->ncond = 0;
    np->type = NS_AREA;
    np->x = np->range.lx - 1;
    np->y = np->range.ly;
    np->dx = -1;
    np->dy = 0;
}

void
snxtsct_rewind(struct nstr_sect *np)
{
    np->x = np->range.lx - 1;
    np->y = np->range.ly;
    np->dx = -1;
    np->dy = 0;
    np->id = -1;
}

void
snxtsct_dist(struct nstr_sect *np, coord cx, coord cy, int dist)
{
    memset(np, 0, sizeof(*np));
    xydist_range(cx, cy, dist, &np->range);
    np->cx = cx;
    np->cy = cy;
    np->ncond = 0;
    np->dist = dist;
    np->type = NS_DIST;
    np->x = np->range.lx - 1;
    np->y = np->range.ly;
    np->dx = -1;
    np->dy = 0;
}

void
xysize_range(struct range *rp)
{
    if (rp->lx > rp->hx)
	rp->width = WORLD_X + rp->hx + 1 - rp->lx;
    else
	rp->width = rp->hx + 1 - rp->lx;
    if (CANT_HAPPEN(rp->width > WORLD_X))
	rp->width = WORLD_X;
    if (rp->ly > rp->hy)
	rp->height = WORLD_Y + rp->hy + 1 - rp->ly;
    else
	rp->height = rp->hy + 1 - rp->ly;
    if (CANT_HAPPEN(rp->height > WORLD_Y))
	rp->height = WORLD_Y;
}

void
xydist_range(coord x, coord y, int dist, struct range *rp)
{
    if (4 * dist + 1 < WORLD_X) {
	rp->lx = xnorm(x - 2 * dist);
	rp->hx = xnorm(x + 2 * dist);
	rp->width = 4 * dist + 1;
    } else {
	rp->lx = xnorm(x - WORLD_X / 2);
	rp->hx = xnorm(rp->lx + WORLD_X - 1);
	rp->width = WORLD_X;
    }

    if (2 * dist + 1 < WORLD_Y) {
	rp->ly = ynorm(y - dist);
	rp->hy = ynorm(y + dist);
	rp->height = 2 * dist + 1;
    } else {
	rp->ly = ynorm(y - WORLD_Y / 2);
	rp->hy = ynorm(rp->ly + WORLD_Y - 1);
	rp->height = WORLD_Y;
    }
}
