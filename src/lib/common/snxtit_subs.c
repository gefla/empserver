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
 *  snxtit_subs.c: arrange item selection using one of many criteria.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include "misc.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "file.h"
#include "common.h"

void
snxtitem_area(register struct nstr_item *np, int type, struct range *range)
{
    bzero((s_char *)np, sizeof(*np));
    np->cur = -1;
    np->type = type;
    np->sel = NS_AREA;
    np->index = -1;
    np->range = *range;
    np->read = ef_read;
    np->flags = ef_flags(type);
    xysize_range(&np->range);
    ef_zapcache(type);
}

void
snxtitem_dist(register struct nstr_item *np, int type, int cx, int cy,
	      int dist)
{
    struct range range;

    bzero((s_char *)np, sizeof(*np));
    xydist_range(cx, cy, dist, &range);
    np->cur = -1;
    np->type = type;
    np->sel = NS_DIST;
    np->cx = cx;
    np->cy = cy;
    np->index = -1;
    np->range = range;
    np->dist = dist;
    np->read = ef_read;
    np->flags = ef_flags(type);
#if 0
    /* This is no longer proper. */
    /* It did the wrong thing for small, hitech worlds. */
    xysize_range(&np->range);
#endif
    ef_zapcache(type);
}

void
snxtitem_xy(register struct nstr_item *np, int type, coord x, coord y)
{
    bzero((s_char *)np, sizeof(*np));
    np->cur = -1;
    np->type = type;
    np->sel = NS_XY;
    np->cx = xnorm(x);
    np->cy = ynorm(y);
    np->index = -1;
    np->dist = 0;
    np->read = ef_read;
    np->flags = ef_flags(type);
    ef_zapcache(type);
}

void
snxtitem_all(register struct nstr_item *np, int type)
{
    bzero((s_char *)np, sizeof(*np));
    np->cur = -1;
    np->sel = NS_ALL;
    np->type = type;
    np->index = -1;
    np->read = ef_read;
    np->flags = ef_flags(type);
    xysize_range(&np->range);
    ef_zapcache(type);
}

void
snxtitem_group(register struct nstr_item *np, int type, s_char group)
{
    if (group == '~')
	group = ' ';
    bzero((s_char *)np, sizeof(*np));
    np->cur = -1;
    np->sel = NS_GROUP;
    np->group = group;
    np->type = type;
    np->index = -1;
    np->read = ef_read;
    np->flags = ef_flags(type);
    xysize_range(&np->range);
    ef_zapcache(type);
}

void
snxtitem_rewind(struct nstr_item *np)
{
    np->cur = -1;
    np->index = -1;
    ef_zapcache(np->type);
}

int
snxtitem_list(register struct nstr_item *np, int type, int *list, int len)
{
    int i;

    bzero((s_char *)np, sizeof(*np));
    np->cur = -1;
    np->type = type;
    np->sel = NS_LIST;
    np->index = -1;
    np->read = ef_read;
    np->flags = ef_flags(type);
    if (len <= 0 || len > NS_LSIZE)
	return 0;
    for (i = 0; i < len; i++)
	np->list[i] = list[i];
    np->size = len;
    ef_zapcache(type);
    return 1;
}
