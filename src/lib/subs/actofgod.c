/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  actofgod.c: Deity meddling subroutines
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2013
 */

#include <config.h>

#include "actofgod.h"
#include "file.h"
#include "news.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"

static void
nreport_divine_aid(natid whom, int goodness)
{
    if (opt_GODNEWS && getnatp(whom)->nat_stat != STAT_GOD && goodness)
	nreport(player->cnum, goodness > 0 ? N_AIDS : N_HURTS, whom, 1);
}

void
report_god_takes(char *prefix, char *what, natid from)
{
    if (from && from != player->cnum) {
	wu(0, from, "%s%s taken from you by an act of %s!\n",
	   prefix, what, cname(player->cnum));
	nreport_divine_aid(from, -1);
    }
}

void
report_god_gives(char *prefix, char *what, natid to)
{
    if (to && to != player->cnum) {
	wu(0, to, "%s%s given to you by an act of %s!\n",
	   prefix, what, cname(player->cnum));
	nreport_divine_aid(to, 1);
    }
}
