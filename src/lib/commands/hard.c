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
 *  hard.c: Increases the silo protection of a given missile
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "item.h"
#include "plane.h"
#include "nuke.h"
#include "ship.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "commands.h"

/*
 * harden <PLANE> <LEVEL>
 */
int
hard(void)
{
    struct plchrstr *pcp;
    struct plnstr pln;
    int level;
    s_char *p;
    int hcm;
    int n;
    struct nstr_item ni;
    struct sctstr sect;
    s_char buf[1024];
    long cash;
    struct natstr *natp;

    if (!snxtitem(&ni, EF_PLANE, player->argp[1]))
	return RET_SYN;
    if ((p = getstarg(player->argp[2], "Increase by? ", buf)) == 0
	|| *p == 0)
	return RET_SYN;
    level = atoi(p);
    if (level < 0)
	return RET_SYN;
    natp = getnatp(player->cnum);
    cash = natp->nat_money;
    while (nxtitem(&ni, (s_char *)&pln)) {
	if (!player->owner)
	    continue;
	pcp = &plchr[(int)pln.pln_type];
	if ((pcp->pl_flags & P_M) == 0) {
	    pr("%s isn't a missile!\n", prplane(&pln));
	    continue;
	}
	if (pln.pln_ship >= 0)	/* can't harden ssbns ... */
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
	hcm = getvar(V_HCM, (s_char *)&sect, EF_SECTOR);
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
	player->dolcost += (5.0 * n);
	putplane(pln.pln_uid, &pln);
	putvar(V_HCM, hcm - n, (s_char *)&sect, EF_SECTOR);
	putsect(&sect);
	pr("%s hardened to %d\n", prplane(&pln), pln.pln_harden);
    }
    return RET_OK;
}
