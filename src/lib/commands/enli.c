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
 *  enli.c: Enlist some military
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "chance.h"
#include "commands.h"

int
enli(void)
{
    struct nstr_sect nstr;
    struct sctstr sect;
    struct natstr *natp;
    int civ;
    int mil;
    int newmil;
    int milwant;
    int totalmil;
    int reserve;
    char *p;
    int quota;
    char prompt[128];
    char buf[1024];

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    natp = getnatp(player->cnum);
    newmil = 500;
    sprintf(prompt, "Number to enlist (max %d) : ", newmil);
    if (!(p = getstarg(player->argp[2], prompt, buf)))
	return RET_SYN;
    if ((milwant = atoi(p)) > newmil)
	milwant = newmil;
    if (0 != (quota = (milwant < 0)))
	milwant = -milwant;
    totalmil = 0;
    reserve = natp->nat_reserve;
    if (reserve <= 0) {
	pr("No military reserves left\n");
	return RET_OK;
    }
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (sect.sct_oldown != player->cnum)
	    continue;
	civ = sect.sct_item[I_CIVIL];
	if (civ == 0)
	    continue;
	if (sect.sct_loyal > 70) {
	    pr("civilians refuse to report in %s!\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	    continue;
	}
	if (sect.sct_mobil <= 0) {
	    pr("%s is out of mobility!\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	}
	mil = sect.sct_item[I_MILIT];
	newmil = civ * 0.5;
	if (quota) {
	    if (newmil > milwant - mil)
		newmil = milwant - mil;
	    if (newmil > 500)
		newmil = 500;
	} else if (newmil > milwant)
	    newmil = milwant;
	if (newmil > 999 - mil)
	    newmil = 999 - mil;
	if (newmil <= 0)
	    continue;
	if (newmil > reserve)
	    newmil = reserve;
	sect.sct_item[I_MILIT] = newmil + mil;
	reserve -= newmil;
	totalmil += newmil;
	sect.sct_item[I_CIVIL] = civ - newmil;
	pr("%3d enlisted in %s (%d)\n", newmil,
	   xyas(sect.sct_x, sect.sct_y, player->cnum), mil + newmil);
	if (sect.sct_mobil > 0)
	    sect.sct_mobil *= 1.0 - (double)newmil / (double)civ;
	putsect(&sect);
	if (totalmil >= 10000) {
	    pr("Rioting in induction center interrupts enlistment\n");
	    break;
	}
	if (reserve == 0) {
	    pr("Military reserve exhausted\n");
	    break;
	}
    }
    pr("Total new enlistment : %d\n", totalmil);
    pr("Military reserves stand at %d\n", reserve);
    if (totalmil) {
	natp->nat_reserve -= totalmil;
	putnat(natp);
    }
    if ((player->btused += roundavg(totalmil * 0.02)) > 0)
	pr("Paperwork at recruiting stations ... %d\n", player->btused);
    return RET_OK;
}
