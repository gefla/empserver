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
 *  navi.c: Navigate ships and such
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995 (rewritten)
 *     Ron Koenderink, 2006-2007
 *     Markus Armbruster, 2006-2014
 */

#include <config.h>

#include "commands.h"
#include "unit.h"

int
navi(void)
{
    struct nstr_item ni_ship;
    struct emp_qelem ship_list;
    double minmob, maxmob;

    if (!snxtitem(&ni_ship, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;
    shp_sel(&ni_ship, &ship_list);
    shp_nav(&ship_list, &minmob, &maxmob, player->cnum);
    if (QEMPTY(&ship_list)) {
	pr("No ships\n");
	return RET_FAIL;
    }
    return unit_move(&ship_list, &minmob, &maxmob);
}
