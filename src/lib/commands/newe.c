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
 *  newe.c: Show new sector efficiency (projected)
 * 
 *  Known contributors to this file:
 *     Thomas Ruschak, 1993
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "nsc.h"
#include "sect.h"
#include "nat.h"
#include "item.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"

int
newe(void)
{
    struct natstr *natp;
    struct sctstr sect;
    struct nstr_sect nstr;
    double work, wforce, lcms, hcms;
    int nsect;
    int civs = 0;
    int uws = 0;
    int bwork;
    int twork;
    int type;
    int eff;
    int maxpop;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    player->simulation = 1;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (!sect.sct_off) {
	    civs = min(999, (int)((obrate * etu_per_update + 1.0)
				  * sect.sct_item[I_CIVIL]));
	    uws = min(999, (int)((uwbrate * etu_per_update + 1.0)
				 * sect.sct_item[I_UW]));
	    natp = getnatp(sect.sct_own);
	    maxpop = max_pop((float)natp->nat_level[NAT_RLEV], &sect);
	    civs = min(civs, maxpop);
	    uws = min(uws, maxpop);
	    /* This isn't quite right, since research might rise/fall */
	    /* during the update, but it's the best we can really do  */
	    wforce = (int)((civs * sect.sct_work) / 100.0
			   + uws + sect.sct_item[I_MILIT] * 2 / 5.0);

	    work = new_work(&sect, wforce * etu_per_update / 100);
	    bwork = work / 2;

	    type = sect.sct_type;
	    eff = sect.sct_effic;
	    if (sect.sct_newtype != type) {
		twork = (eff + 3) / 4;
		if (twork > bwork) {
		    twork = bwork;
		}
		work -= twork;
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
		work -= twork;
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
		work -= twork;
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
	prxy("%4d,%-4d", nstr.x, nstr.y, player->cnum);
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
