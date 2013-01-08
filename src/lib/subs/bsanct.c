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
 *  bsanct.c: Break Sanctuary subroutines
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "file.h"
#include "game.h"
#include "news.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"

void
bsanct(void)
{
    int count;
    struct sctstr s;
    struct nstr_sect nstr;

    if (player->god)
	return;
    count = 0;
    snxtsct_all(&nstr);
    while (nxtsct(&nstr, &s)) {
	if (!player->owner)
	    continue;
	if (s.sct_type != SCT_SANCT)
	    continue;
	pr("%s is no longer a sanctuary.\n",
	   xyas(s.sct_x, s.sct_y, player->cnum));
	if (s.sct_effic == 100) {
	    s.sct_type = SCT_CAPIT;
	    s.sct_newtype = SCT_CAPIT;
	} else {
	    s.sct_type = SCT_HIWAY;
	    s.sct_newtype = SCT_HIWAY;
	}
	game_tick_to_now(&s.sct_access);
	(void)putsect(&s);
	count++;
    }
    if (count > 0) {
	game_note_bsanct();
	nreport(player->cnum, N_BROKE_SANCT, 0, 1);
    }
}
