/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  bsanct.c: Break Sanctuary subroutines
 * 
 *  Known contributors to this file:
 *  
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "news.h"
#include "path.h"
#include "file.h"
#include "deity.h"
#include "prototypes.h"

void
bsanct(void)
{
    register int count;
    struct sctstr s;
    struct nstr_sect nstr;

    if (player->god)
	return;
    count = 0;
    snxtsct(&nstr, "*");
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
	time(&s.sct_access);
	(void)putsect(&s);
	count++;
    }
    if (count > 0)
	nreport(player->cnum, N_BROKE_SANCT, 0, 1);
}
