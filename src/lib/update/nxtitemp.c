/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nxtitemp.c: Get next item from list
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include <config.h>

#include "empobj.h"
#include "land.h"
#include "nsc.h"
#include "ship.h"
#include "unit.h"
#include "update.h"

void *
nxtitemp(struct nstr_item *np)
{
    struct empobj *gp;
    int selected;

    if (np->sel == NS_UNDEF)
	return NULL;
    do {
	if (np->sel == NS_LIST) {
	    np->index++;
	    if (np->index >= np->size)
		return NULL;
	    np->cur = np->list[np->index];
	} else if (np->sel == NS_CARGO) {
	    if (np->next < 0)
		return NULL;
	    np->cur = np->next;
	    np->next = unit_cargo_next(np->type, np->next);
	} else {
	    np->cur++;
	}
	gp = ef_ptr(np->type, np->cur);
	if (!gp)
	    return NULL;

	selected = 1;
	switch (np->sel) {
	case NS_LIST:
	case NS_CARGO:
	case NS_ALL:
	    break;
	case NS_DIST:
	    if (CANT_HAPPEN(!(ef_flags(np->type) & EFF_XY)))
		return NULL;
	    if (!xyinrange(gp->x, gp->y, &np->range)) {
		selected = 0;
		break;
	    }
	    np->curdist = mapdist(gp->x, gp->y, np->cx, np->cy);
	    if (np->curdist > np->dist)
		selected = 0;
	    break;
	case NS_AREA:
	    if (CANT_HAPPEN(!(ef_flags(np->type) & EFF_XY)))
		return NULL;
	    if (!xyinrange(gp->x, gp->y, &np->range))
		selected = 0;
	    break;
	case NS_XY:
	    if (CANT_HAPPEN(!(ef_flags(np->type) & EFF_XY)))
		return NULL;
	    if (xnorm(gp->x) != np->cx || ynorm(gp->y) != np->cy)
		selected = 0;
	    break;
	case NS_GROUP:
	    if (CANT_HAPPEN(!(ef_flags(np->type) & EFF_GROUP)))
		return NULL;
	    if (np->group != gp->group)
		selected = 0;
	    break;
	default:
	    CANT_REACH();
	    return NULL;
	}
	if (selected && np->ncond) {
	    /* nstr_exec is expensive, so we do it last */
	    if (!nstr_exec(np->cond, np->ncond, gp))
		selected = 0;
	}
    } while (!selected);
    return gp;
}
