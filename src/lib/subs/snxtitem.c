/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  snxtitem.c: Arrange item selection using one of many criteria.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Markus Armbruster, 2009-2020
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "xy.h"
#include "nsc.h"
#include "prototypes.h"
#include "unit.h"

/*
 * setup the nstr structure for sector selection.
 * can select on NS_ALL, NS_AREA, NS_DIST, and NS_LIST.
 * iterate thru the "condarg" string looking
 * for arguments to compile into the nstr.
 * Using this function for anything but command arguments is usually
 * incorrect, because it respects conditionals.  Use the snxtitem_FOO()
 * instead.
 */
int
snxtitem(struct nstr_item *np, int type, char *str, char *prompt)
{
    struct range range;
    int list[NS_LSIZE];
    int cnum, n;
    coord cx, cy;
    int dist;
    int flags;
    char promptbuf[128];
    char buf[1024];

    np->type = EF_BAD;
    np->sel = NS_UNDEF;
    if (!str) {
	if (!prompt) {
	    sprintf(promptbuf, "%s(s)? ", ef_nameof(type));
	    prompt = promptbuf;
	}
	str = getstring(prompt, buf);
	if (!str)
	    return 0;
    } else
	make_stale_if_command_arg(str);
    if (*str == 0) {
	return 0;
    }
    if (type == EF_NATION && isalpha(*str)) {
	cnum = natarg(str, NULL);
	if (cnum < 0)
	    return 0;
	sprintf(buf, "%d", cnum);
	str = buf;
    }
    flags = ef_flags(type);
    switch (sarg_type(str)) {
    case NS_AREA:
	if (!(flags & EFF_XY))
	    return 0;
	if (!sarg_area(str, &range))
	    return 0;
	snxtitem_area(np, type, &range);
	break;
    case NS_DIST:
	if (!(flags & EFF_XY))
	    return 0;
	if (!sarg_range(str, &cx, &cy, &dist))
	    return 0;
	snxtitem_dist(np, type, cx, cy, dist);
	break;
    case NS_ALL:
	snxtitem_all(np, type);
	break;
    case NS_LIST:
	if ((n = sarg_list(str, list, NS_LSIZE)) == 0)
	    return 0;
	if (!snxtitem_list(np, type, list, n))
	    return 0;
	break;
    case NS_XY:
	if (!(flags & EFF_XY))
	    return 0;
	if (!sarg_xy(str, &cx, &cy))
	    return 0;
	snxtitem_xy(np, type, cx, cy);
	break;
    case NS_GROUP:
	if (!(flags & EFF_GROUP))
	    return 0;
	snxtitem_group(np, type, *str);
	break;
    default:
	return 0;
    }
    return snxtitem_use_condarg(np);
}

void
snxtitem_area(struct nstr_item *np, int type, struct range *range)
{
    memset(np, 0, sizeof(*np));
    np->cur = -1;
    np->type = type;
    np->sel = NS_AREA;
    np->index = -1;
    np->range = *range;
    xysize_range(&np->range);
}

void
snxtitem_dist(struct nstr_item *np, int type, int cx, int cy, int dist)
{
    memset(np, 0, sizeof(*np));
    xydist_range(cx, cy, dist, &np->range);
    np->cur = -1;
    np->type = type;
    np->sel = NS_DIST;
    np->cx = cx;
    np->cy = cy;
    np->index = -1;
    np->dist = dist;
}

void
snxtitem_xy(struct nstr_item *np, int type, coord x, coord y)
{
    memset(np, 0, sizeof(*np));
    np->cur = -1;
    np->type = type;
    np->sel = NS_XY;
    np->cx = xnorm(x);
    np->cy = ynorm(y);
    np->index = -1;
    np->dist = 0;
}

void
snxtitem_all(struct nstr_item *np, int type)
{
    memset(np, 0, sizeof(*np));
    np->cur = -1;
    np->sel = NS_ALL;
    np->type = type;
    np->index = -1;
    xysize_range(&np->range);
}

void
snxtitem_group(struct nstr_item *np, int type, char group)
{
    if (group == '~')
	group = 0;
    memset(np, 0, sizeof(*np));
    np->cur = -1;
    np->sel = NS_GROUP;
    np->group = group;
    np->type = type;
    np->index = -1;
    xysize_range(&np->range);
}

void
snxtitem_rewind(struct nstr_item *np)
{
    np->cur = -1;
    np->index = -1;
}

int
snxtitem_list(struct nstr_item *np, int type, int *list, int len)
{
    int i;

    memset(np, 0, sizeof(*np));
    np->cur = -1;
    np->type = type;
    np->sel = NS_LIST;
    np->index = -1;
    if (len <= 0 || len > NS_LSIZE)
	return 0;
    for (i = 0; i < len; i++)
	np->list[i] = list[i];
    np->size = len;
    return 1;
}

/*
 * Initialize @np to iterate over the items of type @type in a carrier.
 * The carrier has file type @carrier_type and UID @carrier_uid.
 * Note: you can take an item gotten with nxtitem() off its carrier
 * without disturbing the iterator.  Whether the iterator will pick up
 * stuff you load onto the carrier during iteration is unspecified.
 */
void
snxtitem_cargo(struct nstr_item *np, int type,
	       int carrier_type, int carrier_uid)
{
    memset(np, 0, sizeof(*np));
    np->cur = -1;
    np->type = type;
    np->sel = NS_CARGO;
    np->next = unit_cargo_first(carrier_type, carrier_uid, type);
}

int
snxtitem_use_condarg(struct nstr_item *np)
{
    int n;

    if (!player->condarg)
	return 1;
    n = nstr_comp(np->cond, ARRAY_SIZE(np->cond),
		  np->type, player->condarg);
    if (n < 0)
	return 0;
    np->ncond = n;
    return 1;
}
