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
 *  brea.c: Break out of sanctuary
 *
 *  Known contributors to this file:
 *     Jeff Wallace, 1989
 */

#include <config.h>

#include "commands.h"

int
brea(void)
{
    struct natstr *natp;

    natp = getnatp(player->cnum);
    if (natp->nat_stat != STAT_SANCT) {
	pr("You aren't in sanctuary!\n");
	return RET_SYN;
    }
    bsanct();
    natp->nat_stat = STAT_ACTIVE;
    putnat(natp);
    return RET_OK;
}
