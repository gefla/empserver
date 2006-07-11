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

#include "config.h"
#include "prototypes.h"
#include "file.h"
#include "empobj.h"

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
