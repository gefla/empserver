/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  newe.c: Show new sector efficiency (projected)
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1993
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "optlist.h"

int
newe(void)
{
    struct natstr *natp;
    struct sctstr sect;
    struct nstr_sect nstr;
    double work, lcms, hcms;
    int nsect;
    int bwork;
    int twork;
    int type;
    int eff;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    player->simulation = 1;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (!sect.sct_off) {
	    natp = getnatp(sect.sct_own);
	    work = do_feed(&sect, natp, etu_per_update, 1);
	    bwork = work / 2;

	    type = sect.sct_type;
	    eff = sect.sct_effic;
	    if (sect.sct_newtype != type) {
		twork = (eff + 3) / 4;
		if (twork > bwork) {
		    twork = bwork;
		}
		bwork -= twork;
		eff -= twork * 4;
		if (eff <= 0) {
		    type = sect.sct_newtype;
		    eff = 0;
		}

		twork = 100 - eff;
		if (twork > bwork) {
		    twork = bwork;
		}
		if (dchr[type].d_lcms > 0) {
		    lcms = sect.sct_item[I_LCM];
		    lcms = (int)(lcms / dchr[type].d_lcms);
		    if (twork > lcms)
			twork = lcms;
		}
		if (dchr[type].d_hcms > 0) {
		    hcms = sect.sct_item[I_HCM];
		    hcms = (int)(hcms / dchr[type].d_hcms);
		    if (twork > hcms)
			twork = hcms;
		}
		eff += twork;
	    } else if (eff < 100) {
		twork = 100 - eff;
		if (twork > bwork) {
		    twork = bwork;
		}
		if (dchr[type].d_lcms > 0) {
		    lcms = sect.sct_item[I_LCM];
		    lcms = (int)(lcms / dchr[type].d_lcms);
		    if (twork > lcms)
			twork = lcms;
		}
		if (dchr[type].d_hcms > 0) {
		    hcms = sect.sct_item[I_HCM];
		    hcms = (int)(hcms / dchr[type].d_hcms);
		    if (twork > hcms)
			twork = hcms;
		}
		eff += twork;
	    }
	} else {
	    eff = sect.sct_effic;
	    type = sect.sct_type;
	}
	if (nsect++ == 0) {
	    pr("EFFICIENCY SIMULATION\n");
	    pr("   sect  des    projected eff\n");
	}
	prxy("%4d,%-4d", nstr.x, nstr.y);
	pr(" %c", dchr[type].d_mnem);
	pr("    %3d%%\n", eff);
    }
    player->simulation = 0;
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
