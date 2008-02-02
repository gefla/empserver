/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
#include "optlist.h"
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
    CANT_REACH();
    return "The Beast #666";
}

struct empobj *
get_empobjp(int type, int id)
{
    if (CANT_HAPPEN(type == EF_SECTOR || type == EF_BAD))
	return NULL;
    return ef_ptr(type, id);
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

struct empobj_chr *
get_empobj_chr(struct empobj *gp)
{
    switch (gp->ef_type) {
    case EF_LAND:
	return (struct empobj_chr *)&lchr[(int)gp->type];
    case EF_SHIP:
	return (struct empobj_chr *)&mchr[(int)gp->type];
    case EF_PLANE:
	return (struct empobj_chr *)&plchr[(int)gp->type];
    case EF_NUKE:
	return (struct empobj_chr *)&nchr[(int)gp->type];
    case EF_SECTOR:
	return (struct empobj_chr *)&dchr[(int)gp->type];
    }
    CANT_REACH();
    return NULL;
}

char *
emp_obj_chr_name(struct empobj *gp)
{
    switch (gp->ef_type) {
    case EF_LAND:
	return lchr[(int)gp->type].l_name;
    case EF_SHIP:
	return mchr[(int)gp->type].m_name;
    case EF_PLANE:
	return plchr[(int)gp->type].pl_name;
    case EF_NUKE:
	return nchr[(int)gp->type].n_name;
    case EF_SECTOR:
	return dchr[(int)gp->type].d_name;
    }
    CANT_REACH();
    return NULL;
}

int
get_empobj_mob_max(int type)
{
    switch (type) {
    case EF_SHIP:
	return ship_mob_max;
    case EF_LAND:
	return land_mob_max;
    case EF_PLANE:
	return plane_mob_max;
    case EF_SECTOR:
	return sect_mob_max;
    }
    CANT_REACH();
    return -1;
}
