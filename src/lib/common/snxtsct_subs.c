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
 *  snxtsct_subs.c: arrange sector selection using either distance or area
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */
/*
 * XXX we can *almost* treat sectors as items, but not quite.
 * Items are retrieved using id's, and sectors still use x,y.
 */

#include "misc.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "file.h"
#include "common.h"
#include "optlist.h"

void
snxtsct_all(struct nstr_sect *np)
{
    struct range worldrange;

    worldrange.lx = -WORLD_X / 2;
    worldrange.ly = -WORLD_Y / 2;
    worldrange.hx = WORLD_X / 2;
    worldrange.hy = WORLD_Y / 2;
    worldrange.width = worldrange.height = 0;
    snxtsct_area(np, &worldrange);
}

void
snxtsct_area(register struct nstr_sect *np, struct range *range)
{
    memset(np, 0, sizeof(*np));
    np->range = *range;
    np->ncond = 0;
    np->type = NS_AREA;
    np->read = ef_read;
    np->x = np->range.lx - 1;
    np->y = np->range.ly;
    np->dx = -1;
    np->dy = 0;
    xysize_range(&np->range);
    ef_zapcache(EF_SECTOR);
}

void
snxtsct_rewind(struct nstr_sect *np)
{
    np->x = np->range.lx - 1;
    np->y = np->range.ly;
    np->dx = -1;
    np->dy = 0;
    np->id = -1;
    ef_zapcache(EF_SECTOR);
}

void
snxtsct_dist(register struct nstr_sect *np, coord cx, coord cy, int dist)
{
    memset(np, 0, sizeof(*np));
    xydist_range(cx, cy, dist, &np->range);
    np->cx = cx;
    np->cy = cy;
    np->ncond = 0;
    np->dist = dist;
    np->type = NS_DIST;
    np->read = ef_read;
    np->x = np->range.lx - 1;
    np->y = np->range.ly;
    np->dx = -1;
    np->dy = 0;
#if 0
    /* This function is now done elsewhere. */
    /* It was not doing the right thing when the world was small */
    xysize_range(&np->range);
#endif
    ef_zapcache(EF_SECTOR);
}

void
xysize_range(register struct range *rp)
{
    if (rp->lx >= rp->hx)
	rp->width = WORLD_X + rp->hx - rp->lx;
    else
	rp->width = rp->hx - rp->lx;
#ifndef HAY
    /* This is a necessary check for small, hitech worlds. */
    if (rp->width > WORLD_X)
	rp->width = WORLD_X;
#endif
    if (rp->ly >= rp->hy)
	rp->height = WORLD_Y + rp->hy - rp->ly;
    else
	rp->height = rp->hy - rp->ly;
#ifndef HAY
    /* This is a necessary check for small, hitech worlds. */
    if (rp->height > WORLD_Y)
	rp->height = WORLD_Y;
#endif
}

/* This is called also called in snxtitem.c */
void
xydist_range(coord x, coord y, register int dist, struct range *rp)
{
    if (dist < WORLD_X / 4) {
	rp->lx = xnorm((coord)(x - 2 * dist));
	rp->hx = xnorm((coord)(x + 2 * dist) + 1);
	rp->width = 4 * dist + 1;
    } else {
	/* Range is larger than the world */
	/* Make sure we get lx in the right place. */
	rp->lx = xnorm((coord)(x - WORLD_X / 2));
	rp->hx = xnorm((coord)(rp->lx + WORLD_X - 1));
	rp->width = WORLD_X;
    }

    if (dist < WORLD_Y / 2) {
	rp->ly = ynorm((coord)(y - dist));
	rp->hy = ynorm((coord)(y + dist) + 1);
	rp->height = 2 * dist + 1;
    } else {
	/* Range is larger than the world */
	rp->ly = ynorm((coord)(y - WORLD_Y / 2));
	rp->hy = ynorm((coord)(rp->ly + WORLD_Y - 1));
	rp->height = WORLD_Y;
    }
}
