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
 *  hidd.c: Do a hidden values report (deity)
 * 
 *  Known contributors to this file:
 *     Jeff Wallace, 1989
 *     Pat Loney, 1998
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "deity.h"
#include "file.h"
#include "commands.h"

int
hidd(void)
{
	struct	sctstr sect;
	int	nsect;
	struct	nstr_sect nstr;
	int	cond[I_MAX+1];

	if (!snxtsct(&nstr, player->argp[1]))
		return RET_SYN;
	prdate();
	nsect = 0;
	while (nxtsct(&nstr, &sect)) {
		if (!player->owner)
			continue;
		if (nsect++ == 0) {
			if (player->god)
				pr("    ");
			pr("HIDDEN VALUES     old        che-------  plague----\n");
			if (player->god)
				pr("own ");
			pr("  sect        eff own  loyal cnum value  stage time mines\n");
		}
		if (player->god)
			pr("%3d ", sect.sct_own);
		getvec(VT_COND, cond, (s_char *)&sect, EF_SECTOR);
		prxy("%4d,%-4d", nstr.x, nstr.y, player->cnum);
		pr(" %c  %3d%% %3d   %3d  %3d  %3d    %3d   %3d  %3d",
			dchr[sect.sct_type].d_mnem, sect.sct_effic,
			sect.sct_oldown, sect.sct_loyal,
			get_che_cnum(cond[C_CHE]),
			get_che_value(cond[C_CHE]),
			cond[C_PSTAGE], cond[C_PTIME],
			cond[C_MINE]);
		pr("\n");
	}
	if (nsect == 0) {
		if (player->argp[1])
			pr("%s: No sector(s)\n", player->argp[1]);
		else
			pr("%s: No sector(s)\n", "");
		return RET_FAIL;
	}else
		pr("%d sector%s\n", nsect, splur(nsect));
	return 0;
}
