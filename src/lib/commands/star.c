/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  star.c: Start a sector producing
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "path.h"
#include "file.h"
#include "commands.h"

static void start_hdr(void);

int
start(void)
{
    struct sctstr sect;
    int nsect;
    struct nstr_sect nstr;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (nsect++ == 0)
	    start_hdr();
	if (player->god)
	    pr("%3d ", sect.sct_own);
	prxy("%4d,%-4d", nstr.x, nstr.y, player->cnum);
	pr(" %c", dchr[sect.sct_type].d_mnem);
	if (sect.sct_newtype != sect.sct_type)
	    pr("%c", dchr[sect.sct_newtype].d_mnem);
	else
	    pr(" ");
	pr("%4d%%", sect.sct_effic);

	pr(" will be updated normally.\n");
	if (sect.sct_off != 0) {
	    sect.sct_off = 0;
	    putsect(&sect);
	}
    }
    if (nsect == 0) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return 0;
}

static void
start_hdr(void)
{
    if (player->god)
	pr("    ");
    pr("PRODUCTION STARTING\n");
    if (player->god)
	pr("own ");
    pr("  sect        eff\n");
}
