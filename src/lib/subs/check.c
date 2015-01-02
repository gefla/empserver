/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  check.c: Check a sector, plane, land unit, ship or nuke
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2014
 */

#include <config.h>

#include <ctype.h>
#include "commodity.h"
#include "empobj.h"
#include "file.h"
#include "player.h"
#include "prototypes.h"
#include "xy.h"

static int
obj_changed(struct empobj *obj)
{
    union empobj_storage old;

    if (!get_empobj(obj->ef_type, obj->uid, &old))
	return 0;
    if (!ef_typedstr_eq((struct ef_typedstr *)&old,
			(struct ef_typedstr *)obj))
	return 1;
    ef_mark_fresh(obj->ef_type, obj);
    return 0;
}

int
check_obj_ok(struct empobj *obj)
{
    char *s;

    if (obj_changed(obj)) {
	if (obj->ef_type == EF_SECTOR)
	    pr("Sector %s has changed!\n",
	       xyas(obj->x, obj->y, player->cnum));
	else {
	    s = ef_nameof_pretty(obj->ef_type);
	    pr("%c%s %d has changed!\n", toupper(*s), s + 1, obj->uid);
	}
	return 0;
    }
    return 1;
}

int
check_sect_ok(struct sctstr *sectp)
{
    return check_obj_ok((struct empobj *)sectp);
}

int
check_ship_ok(struct shpstr *shipp)
{
    return check_obj_ok((struct empobj *)shipp);
}

int
check_plane_ok(struct plnstr *planep)
{
    return check_obj_ok((struct empobj *)planep);
}

int
check_land_ok(struct lndstr *landp)
{
    return check_obj_ok((struct empobj *)landp);
}

int
check_nuke_ok(struct nukstr *nukep)
{
    return check_obj_ok((struct empobj *)nukep);
}

int
check_loan_ok(struct lonstr *loanp)
{
    return check_obj_ok((struct empobj *)loanp);
}

int
check_comm_ok(struct comstr *commp)
{
    return check_obj_ok((struct empobj *)commp);
}

int
check_trade_ok(struct trdstr *tp)
{
    return check_obj_ok((struct empobj *)tp);
}
