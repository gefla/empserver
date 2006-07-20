/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  empobj.c: Common functions on struct empobj and
 *            union empobj_storage
 * 
 *  Known contributors to this file:
 *     Ron Koenderink, 2006
 *     Markus Armbruster, 2006
 */

#include <config.h>

#include "empobj.h"
#include "file.h"
#include "prototypes.h"

char *
obj_nameof(struct empobj *gp)
{
    switch (gp->ef_type) {
    case EF_SHIP:
	return prship((struct shpstr *)gp);
    case EF_PLANE:
	return prplane((struct plnstr *)gp);
    case EF_LAND:
	return prland((struct lndstr *)gp);
    case EF_NUKE:
	return prnuke((struct nukstr *)gp);
    }
    CANT_HAPPEN("unsupported TYPE");
    return "";
}


int
put_empobj(struct empobj *gp)
{
    switch (gp->ef_type)
    {
    case EF_SECTOR:
        return ef_write(gp->ef_type, sctoff(gp->x, gp->y), gp);
    case EF_NATION:
    case EF_BMAP:
    case EF_MAP:
	return ef_write(gp->ef_type, gp->own, gp);
    default:
	return ef_write(gp->ef_type, gp->uid, gp);
    }
}

void *
get_empobj_chr(struct empobj *gp)
{
    void *cp;

    switch (gp->ef_type) {
    case EF_LAND:
	cp = &lchr[(int)gp->type];
	break;
    case EF_SHIP:
	cp = &mchr[(int)gp->type];
	break;
    case EF_PLANE:
	cp = &plchr[(int)gp->type];
	break;
    case EF_NUKE:
	cp = &nchr[(int)gp->type];
	break;
    case EF_SECTOR:
	cp = &dchr[(int)gp->type];
	break;
    default:
        CANT_HAPPEN("unsupported TYPE");
        cp = NULL;
	break;
    }
    return cp;
}
