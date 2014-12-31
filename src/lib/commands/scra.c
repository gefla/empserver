/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  scra.c: Scrap ships, planes or land units
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 *     Markus Armbruster, 2004-2014
 */

#include <config.h>

#include <ctype.h>
#include "chance.h"
#include "commands.h"
#include "optlist.h"
#include "plague.h"
#include "unit.h"

int
scra(void)
{
    struct nstr_item ni;
    union empobj_storage item;
    int type, n;
    struct sctstr sect;
    struct mchrstr *mp;
    struct plchrstr *pp;
    struct lchrstr *lp;
    char *p;
    i_type i;
    char prompt[128];
    char buf[1024];
    float eff;

    if (!(p = getstarg(player->argp[1], "Ship, land, or plane? ", buf)))
	return RET_SYN;
    switch (*p) {
    case 's':
	type = EF_SHIP;
	break;
    case 'p':
	type = EF_PLANE;
	break;
    case 'l':
	type = EF_LAND;
	break;
    default:
	pr("Ships, land units, or planes only! (s, l, p)\n");
	return RET_SYN;
    }

    if (!snxtitem(&ni, type, player->argp[2], NULL))
	return RET_SYN;
    n = 0;
    while (nxtitem(&ni, &item)) {
	if (!player->owner)
	    continue;
	n++;
    }
    snprintf(prompt, sizeof(prompt), "Really scrap %d %s%s [n]? ",
			    n, ef_nameof(type), splur(n));
    if (!confirm(prompt))
	return RET_FAIL;

    snxtitem_rewind(&ni);
    while (nxtitem(&ni, &item)) {
	if (!player->owner)
	    continue;

	if (opt_MARKET) {
	    if (ontradingblock(type, &item.ship)) {
		pr("You cannot scrap an item on the trading block!\n");
		continue;
	    }
	}

	getsect(item.gen.x, item.gen.y, &sect);
	if (type == EF_SHIP) {
	    if (!player->owner
		&& relations_with(sect.sct_own, player->cnum) < FRIENDLY) {
		pr("%s is not in a friendly harbor!\n",
		   prship(&item.ship));
		continue;
	    }
	    if (sect.sct_type != SCT_HARBR || sect.sct_effic < 60) {
		pr("%s is not in a 60%% efficient harbor!\n",
		   prship(&item.ship));
		continue;
	    }
	    if (mchr[item.ship.shp_type].m_flags & M_TRADE) {
		pr("WARNING: You only collect money from trade ships if you \"scuttle\" them!\n");
		sprintf(prompt,
			"Are you really sure that you want to scrap %s (n)? ",
			prship(&item.ship));
		if (!confirm(prompt)) {
		    pr("%s not scrapped\n", prship(&item.ship));
		    continue;
		}
	    }
	} else {
	    if (!player->owner
		&& relations_with(sect.sct_own, player->cnum) != ALLIED) {
		pr("%s is not in an allied sector!\n",
		   unit_nameof(&item.gen));
		continue;
	    }
	    if (type == EF_PLANE
		&& (sect.sct_type != SCT_AIRPT || sect.sct_effic < 60)) {
		pr("%s is not in a 60%% efficient airfield!\n",
		   prplane(&item.plane));
		continue;
	    }
	}

	pr("%s scrapped in %s\n",
	   unit_nameof(&item.gen),
	   xyas(item.gen.x, item.gen.y, player->cnum));
	unit_drop_cargo(&item.gen, sect.sct_own);
	if (type == EF_SHIP) {
	    eff = item.ship.shp_effic / 100.0;
	    mp = &mchr[(int)item.ship.shp_type];
	    for (i = I_NONE + 1; i <= I_MAX; i++) {
		if (load_comm_ok(&sect, item.ship.shp_own, i,
				 -item.ship.shp_item[i]))
		    sect.sct_item[i] += item.ship.shp_item[i];
	    }
	    sect.sct_item[I_LCM] += mp->m_lcm * 2 / 3 * eff;
	    sect.sct_item[I_HCM] += mp->m_hcm * 2 / 3 * eff;
	    if (item.ship.shp_pstage == PLG_INFECT
		&& sect.sct_pstage == PLG_HEALTHY)
		sect.sct_pstage = PLG_EXPOSED;
	} else if (type == EF_LAND) {
	    eff = item.land.lnd_effic / 100.0;
	    lp = &lchr[(int)item.land.lnd_type];
	    for (i = I_NONE + 1; i <= I_MAX; i++) {
		if (load_comm_ok(&sect, item.land.lnd_own, i,
				 -item.land.lnd_item[i]))
		    sect.sct_item[i] += item.land.lnd_item[i];
	    }
	    sect.sct_item[I_LCM] += lp->l_lcm * 2 / 3 * eff;
	    sect.sct_item[I_HCM] += lp->l_hcm * 2 / 3 * eff;
	    if (item.land.lnd_pstage == PLG_INFECT
		&& sect.sct_pstage == PLG_HEALTHY)
		sect.sct_pstage = PLG_EXPOSED;
	} else {
	    eff = item.land.lnd_effic / 100.0;
	    pp = &plchr[(int)item.plane.pln_type];
	    sect.sct_item[I_LCM] += pp->pl_lcm * 2 / 3 * eff;
	    sect.sct_item[I_HCM] += pp->pl_hcm * 2 / 3 * eff;
	    sect.sct_item[I_MILIT] += roundavg(pp->pl_crew * eff);
	}
	item.gen.effic = 0;
	put_empobj(type, item.gen.uid, &item.gen);
	for (i = I_NONE + 1; i <= I_MAX; i++) {
	    if (sect.sct_item[i] > ITEM_MAX)
		sect.sct_item[i] = ITEM_MAX;
	}
	putsect(&sect);
    }
    return RET_OK;
}
