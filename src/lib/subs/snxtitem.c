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
 *  snxtitem.c: Arrange item selection using one of many criteria.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "file.h"
#include "com.h"
#include "prototypes.h"

/*
 * setup the nstr structure for sector selection.
 * can select on NS_ALL, NS_AREA, NS_DIST, and NS_LIST.
 * iterate thru the "condarg" string looking
 * for arguments to compile into the nstr.
 */
int
snxtitem(register struct nstr_item *np, int type, s_char *str)
{
	register s_char *cp;
	struct	range range;
	int	list[NS_LSIZE];
	int	n;
	coord	cx, cy;
	int	dist;
	int	flags;
	s_char	natnumber[16];
	s_char	prompt[128];
	s_char	buf[1024];

	np->type = NS_UNDEF;
	np->sel = NS_UNDEF;
	if (str == 0) {
		sprintf(prompt, "%s(s)? ", ef_nameof(type));
		str = getstring(prompt, buf);
		if (str == 0)
			return 0;
	}
	if (*str == 0) {
		/* str present, but only <cr>: nil string passed by player */
		return 0;
	}
	if (type == EF_NATION && isalpha(*str)) {
		sprintf(natnumber, "%d", natarg(str, ""));
		str = natnumber;
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
	np->flags = flags;
	if (player->condarg == 0)
		return 1;
	cp = player->condarg;
	while ((cp = nstr_comp(np->cond, &np->ncond, type, cp)) && *cp)
		;
	if (cp == 0)
		return 0;
	return 1;
}

/*
 * The rest of these (snxtitem_area, snxtitem_dist, etc, have been moved
 * into the common lib, since they don't use condargs, and are useful
 * elsewhere (update, chiefly). ---ts
 *
 */
