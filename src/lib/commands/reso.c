/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  reso.c: Display natural resources
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "path.h"
#include "commands.h"

static void reso_hdr(void);

int
reso(void)
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
	    reso_hdr();
	if (player->god)
	    pr("%3d ", sect.sct_own);
	prxy("%4d,%-4d", nstr.x, nstr.y, player->cnum);
	pr(" %c", dchr[sect.sct_type].d_mnem);
	if (sect.sct_newtype != sect.sct_type)
	    pr("%c", dchr[sect.sct_newtype].d_mnem);
	else
	    pr(" ");
	pr("%4d%%", sect.sct_effic);
	pr(" %4d", sect.sct_min);
	pr("%5d", sect.sct_gmin);
	pr("%5d", sect.sct_fertil);
	pr("%4d", sect.sct_oil);
	pr("%5d", sect.sct_uran);
/*		pr("%5d", sect.sct_elev);*/
	if (!player->god && sect.sct_terr)
	    pr("%4d", sect.sct_terr);
	pr("\n");
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
reso_hdr(void)
{
    if (player->god)
	pr("    ");
    pr("RESOURCE\n");
    if (player->god)
	pr("own ");
    pr("  sect        eff  min gold fert oil uran ");
    if (!player->god)
	pr("ter");
    pr("\n");
}
