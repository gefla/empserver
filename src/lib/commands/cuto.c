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
 *  cuto.c: Do a delivery cutoff level report
 *
 *  Known contributors to this file:
 *     David Muir Sharnoff, 1987
 */

#include <config.h>

#include "commands.h"
#include "path.h"

int
cuto(void)
{
    struct sctstr sect;
    int nsect;
    int n;
    struct nstr_sect nstr;
    char dirstr[12];

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    prdate();
    (void)strcpy(dirstr, ".      $");
    for (n = 1; n <= 6; n++)
	dirstr[n] = dirch[n];
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (nsect++ == 0) {
	    pr("DELIVERY CUTOFF LEVELS\n");
	    if (player->god)
		pr("own");
	    pr("   sect   cmufsgpidbolhr civ mil  uw food sh gun pet irn dst bar oil lcm hcm rad\n");
	}
	if (player->god)
	    pr("%3d", sect.sct_own);
	prxy("%3d,%-3d", nstr.x, nstr.y);
	pr(" %c ", dchr[sect.sct_type].d_mnem);
	pr("%c", dirstr[sect.sct_del[I_CIVIL] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_MILIT] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_UW] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_FOOD] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_SHELL] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_GUN] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_PETROL] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_IRON] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_DUST] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_BAR] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_OIL] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_LCM] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_HCM] & 0x7]);
	pr("%c", dirstr[sect.sct_del[I_RAD] & 0x7]);
	pr("%4d", sect.sct_del[I_CIVIL] & ~0x7);
	pr("%4d", sect.sct_del[I_MILIT] & ~0x7);
	pr("%4d", sect.sct_del[I_UW] & ~0x7);
	pr("%4d", sect.sct_del[I_FOOD] & ~0x7);
	pr("%4d", sect.sct_del[I_SHELL] & ~0x7);
	pr("%4d", sect.sct_del[I_GUN] & ~0x7);
	pr("%4d", sect.sct_del[I_PETROL] & ~0x7);
	pr("%4d", sect.sct_del[I_IRON] & ~0x7);
	pr("%4d", sect.sct_del[I_DUST] & ~0x7);
	pr("%4d", sect.sct_del[I_BAR] & ~0x7);
	pr("%4d", sect.sct_del[I_OIL] & ~0x7);
	pr("%4d", sect.sct_del[I_LCM] & ~0x7);
	pr("%4d", sect.sct_del[I_HCM] & ~0x7);
	pr("%4d", sect.sct_del[I_RAD] & ~0x7);
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
    return RET_OK;
}
