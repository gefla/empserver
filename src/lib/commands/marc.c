/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  marc.c: March units around
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak
 *     Ken Stevens, 1995 (rewrite)
 *     Ron Koenderink, 2006-2007
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "file.h"
#include "map.h"
#include "path.h"
#include "empobj.h"
#include "unit.h"

int
march(void)
{
    struct nstr_item ni_land;
    struct emp_qelem land_list;
    double minmob, maxmob;
    int together;

    if (!snxtitem(&ni_land, EF_LAND, player->argp[1]))
	return RET_SYN;
    lnd_sel(&ni_land, &land_list);
    lnd_mar(&land_list, &minmob, &maxmob, &together, player->cnum);
    if (QEMPTY(&land_list)) {
	pr("No lands\n");
	return RET_FAIL;
    }
    return do_unit_move(&land_list, &together, &minmob, &maxmob);
}
