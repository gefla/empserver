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
 *  nxtitemp.c: Get next item from list
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include "misc.h"
#include "var.h"
#include "xy.h"
#include "plane.h"
#include "ship.h"
#include "nuke.h"
#include "land.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "genitem.h"
#include "player.h"
#include "update.h"
#include "gen.h"
#include "common.h"

s_char *
nxtitemp(struct nstr_item *np, int owner)
{
    struct genitem *gp;
    int selected;

    if (np->sel == NS_UNDEF)
	return 0;
    do {
	if (np->sel == NS_LIST) {
	    np->index++;
	    if (np->index >= np->size)
		return 0;
	    np->cur = np->list[np->index];
	} else {
	    np->cur++;
	}
	gp = (struct genitem *)ef_ptr(np->type, np->cur);
	if (gp == (struct genitem *)0)
	    return 0;

	selected = 1;
	switch (np->sel) {
	    /*
	     * This one won't work unless you're running in emp_player
	     *
	     */
	case NS_LIST:
	    if ((np->flags & EFF_OWNER) && !owner)
		selected = 0;
	    break;
	case NS_ALL:
	    /* XXX maybe combine NS_LIST and NS_ALL later */
	    break;
	case NS_DIST:
	    if (!xyinrange(gp->x, gp->y, &np->range)) {
		selected = 0;
		break;
	    }
	    np->curdist = mapdist((int)gp->x, (int)gp->y,
				  (int)np->cx, (int)np->cy);
	    if (np->curdist > np->dist)
		selected = 0;
	    break;
	case NS_AREA:
	    if (!xyinrange(gp->x, gp->y, &np->range))
		selected = 0;
	    if (gp->x == np->range.hx || gp->y == np->range.hy)
		selected = 0;
	    break;
	case NS_XY:
	    if (xnorm(gp->x) != np->cx || ynorm(gp->y) != np->cy)
		selected = 0;
	    break;
	case NS_GROUP:
	    if (np->group != gp->group)
		selected = 0;
	    break;
	default:
	    logerror("nxtitemp: bad selector %d\n", np->sel);
	    return 0;
	}
	if (selected && np->ncond) {
	    /* nstr_exec is expensive, so we do it last */
	    if (!nstr_exec(np->cond, np->ncond, (s_char *)gp, np->type))
		selected = 0;
	}
    } while (!selected);
    return (s_char *)gp;
}
