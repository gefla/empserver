/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  hard.c: Increases the silo protection of a given missile
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "plane.h"

/*
 * harden <PLANE> <LEVEL>
 */
int
hard(void)
{
    struct plchrstr *pcp;
    struct plnstr pln;
    int level;
    char *p;
    int hcm;
    int n;
    struct nstr_item ni;
    struct sctstr sect;
    char buf[1024];
    long cash;
    struct natstr *natp;

    if (!snxtitem(&ni, EF_PLANE, player->argp[1], NULL))
	return RET_SYN;
    p = getstarg(player->argp[2], "Increase by? ", buf);
    if (!p || !*p)
	return RET_SYN;
    level = atoi(p);
    if (level < 0)
	return RET_SYN;
    natp = getnatp(player->cnum);
    cash = natp->nat_money;
    while (nxtitem(&ni, &pln)) {
	if (!player->owner)
	    continue;
	pcp = &plchr[(int)pln.pln_type];
	if ((pcp->pl_flags & P_M) == 0) {
	    pr("%s isn't a missile!\n", prplane(&pln));
	    continue;
	}
	if (pln.pln_ship >= 0 || pln.pln_land >= 0)
	    continue;
	n = level;
	if (level + pln.pln_harden > 127)
	    n = 127 - pln.pln_harden;
	if (n <= 0) {
	    pr("%s is already completely hardened!\n", prplane(&pln));
	    continue;
	}
	if (!getsect(pln.pln_x, pln.pln_y, &sect))
	    continue;
	if (sect.sct_own != player->cnum) {
	    pr("%s: you don't own %s!\n",
	       prplane(&pln), xyas(pln.pln_x, pln.pln_y, player->cnum));
	    continue;
	}
	hcm = sect.sct_item[I_HCM];
	if (hcm == 0) {
	    pr("No hcm in %s\n", xyas(pln.pln_x, pln.pln_y, player->cnum));
	    continue;
	}
	if (player->dolcost + 5.0 * n > cash) {
	    pr("You don't have enough money to harden %s!\n",
	       prplane(&pln));
	    continue;
	}
	if (hcm <= n)
	    n = hcm;
	pln.pln_harden += n;
	player->dolcost += 5.0 * n;
	putplane(pln.pln_uid, &pln);
	sect.sct_item[I_HCM] = hcm - n;
	putsect(&sect);
	pr("%s hardened to %d\n", prplane(&pln), pln.pln_harden);
    }
    return RET_OK;
}
