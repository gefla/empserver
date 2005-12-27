/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ndump.c: Dump nuke information
 * 
 *  Known contributors to this file:
 *     John Yockey, 1997
 *     Steve McClure, 1998
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "nuke.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

int
ndump(void)
{
    int i;
    struct nstr_item nstr;
    struct nukstr nuk;
    time_t now;
    int nnukes;

    if (!snxtitem(&nstr, EF_NUKE, player->argp[1]))
	return RET_SYN;
    prdate();
    if (player->god)
	pr("   ");
    time(&now);
    pr("DUMP NUKES %ld\n", (long)now);
    if (player->god)
	pr("own ");
    pr("id x y num type\n");
    nnukes = 0;
    while (nxtitem(&nstr, &nuk)) {
	if (!player->god && !player->owner)
	    continue;
	if (nuk.nuk_own == 0)
	    continue;
	nnukes++;
	for (i = 0; i < N_MAXNUKE; i++) {
	    if (nuk.nuk_types[i] > 0) {
		if (player->god)
		    pr("%d ", nuk.nuk_own);
		pr("%d ", nuk.nuk_uid);
		prxy("%d %d", nuk.nuk_x, nuk.nuk_y, player->cnum);
		pr(" %d", nuk.nuk_types[i]);
		pr(" %.5s", nchr[i].n_name);
		pr("\n");
	    }
	}
    }
    if (nnukes == 0) {
	if (player->argp[1])
	    pr("%s: No nuke(s)\n", player->argp[1]);
	else
	    pr("%s: No nuke(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d nuke%s\n", nnukes, splur(nnukes));

    return RET_OK;
}
