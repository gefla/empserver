/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  anti.c: Take action against che
 *
 *  Known contributors to this file:
 *     Pat Loney, 1992
 *     Steve McClure, 1997
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "optlist.h"

/*
 * format: anti <SECT>
 */
int
anti(void)
{
    struct sctstr sect;
    int nsect;
    struct nstr_sect nstr;
    int mil, che, target;
    int avail_mil;
    int amil, ache;
    int milkilled, chekilled;
    double odds, damil, dache;
    int mob;
    int n_cheleft;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (nsect++ == 0) {
	    pr("  sect    subversion activity report\n");
	    pr("  ----    --------------------------\n");
	}
	mil = sect.sct_item[I_MILIT];
	che = sect.sct_che;
	target = sect.sct_che_target;
	avail_mil = sect.sct_mobil / 2;
	if (mil <= avail_mil)
	    avail_mil = mil;
	prxy("%4d,%-4d ", sect.sct_x, sect.sct_y);
	if (avail_mil <= 0) {
	    pr("No available mil or mob in sector.\n");
	    continue;
	}
	pr("Sector mobility/troop strength will allow %d troops to engage.\n",
	   avail_mil);

	if (target == player->cnum) {
	    amil = mil;
	    ache = che;
	    milkilled = 0;
	    chekilled = 0;
	    mob = sect.sct_mobil;
	    while (amil != 0 && ache != 0 && mob > 1) {
		damil = amil;
		dache = ache;
		odds = (dache * 2.0 / (damil + dache));
		odds /= hap_fact(getnatp(sect.sct_own),
				 getnatp(sect.sct_oldown));
		mob = mob - 2;
		if (chance(odds)) {
		    amil = amil - 1;
		    milkilled = milkilled + 1;
		} else {
		    ache = ache - 1;
		    chekilled = chekilled + 1;
		}
	    }
	    if (mil - milkilled > 0) {
		sect.sct_mobil = sect.sct_mobil - chekilled - milkilled;
		sect.sct_item[I_MILIT] = mil - milkilled;
		if (ache == 0)
		    sect.sct_che_target = 0;
		sect.sct_che = ache;
		putsect(&sect);
		pr("          Body count:  Military %d - Guerillas %d.\n",
		   milkilled, chekilled);
		if (ache == 0) {
		    pr("          Partisans cleared out of this sector for now.\n");
		} else {
		    pr("          Partisans still active in this sector.\n");
		}
	    } else {
		if (opt_MOB_ACCESS) {
		    sect.sct_mobil =
			-(etu_per_update / sect_mob_neg_factor);
		} else {
		    sect.sct_mobil = 0;
		}
		sect.sct_loyal = sect.sct_loyal * 0.5;
		n_cheleft = (random() % 4);
		/* 75% chance some che will get left */
		if (n_cheleft) {
		    /* Ok, now leave anywhere from 16% to 25% of the che */
		    n_cheleft = (ache / (n_cheleft + 3));
		    ache -= n_cheleft;
		    sect.sct_che = n_cheleft;
		} else {
		    sect.sct_che = 0;
		    sect.sct_che_target = 0;
		}
		sect.sct_item[I_MILIT] = ache;
		if (sect.sct_own == sect.sct_oldown)
		    sect.sct_oldown = 0;
		sect.sct_own = sect.sct_oldown;
		sect.sct_off = 1;	/* Turn the sector off */
		putsect(&sect);
		pr("          Partisans take over the sector.  You blew it.\n");
		wu(0, sect.sct_oldown,
		   "Sector %s regained from partisan activity.\n",
		   xyas(nstr.x, nstr.y, sect.sct_oldown));
	    }
	} else {
	    pr("          Body count:  Military 0 - Guerillas 0.\n");
	    pr("          Partisans cleared out of this sector for now.\n");
	}
    }
    pr("%d sector%s\n", nsect, splur(nsect));
    return RET_OK;
}
