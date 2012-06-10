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
 *  grin.c: Grind gold bars into dust
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2009
 */

#include <config.h>

#include "commands.h"
#include "product.h"

int
grin(void)
{
    struct nstr_sect nstr;
    struct sctstr sect;
    char *p;
    int prd, i, n, qty;
    char buf[1024];
    double grind_eff = 0.8;
    struct pchrstr *pp;
    i_type ctype;
    unsigned camt;

    prd = dchr[SCT_BANK].d_prd;
    if (prd < 0 || pchr[prd].p_type < 0) {
	pr("Grinding is disabled.\n");
	return RET_FAIL;
    }
    pp = &pchr[prd];

    if (!(p = getstarg(player->argp[1], "Sectors? ", buf)))
	return RET_SYN;
    if (!snxtsct(&nstr, p))
	return RET_SYN;
    p = getstarg(player->argp[2], "amount :  ", buf);
    if (!p || !*p)
	return RET_SYN;
    qty = atoi(p);
    if (qty < 0)
	return RET_SYN;

    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (sect.sct_effic < 60 || sect.sct_own != player->cnum)
	    continue;

	/* materials limit */
	n = MIN(qty, sect.sct_item[pp->p_type]);
	/* work limit */
	n = MIN(n, sect.sct_avail / 5);
	/* space limit */
	for (i = 0; i < MAXPRCON; i++) {
	    ctype = pp->p_ctype[i];
	    camt = pp->p_camt[i];
	    if (!camt)
		continue;
	    if (CANT_HAPPEN(ctype <= I_NONE || ctype > I_MAX))
		continue;
	    n = MIN(n,
		    (double)(ITEM_MAX - sect.sct_item[ctype])
		    / (camt * grind_eff));
	}

	if (n > 0) {
	    pr("%d bars ground up in %s\n", n,
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	    sect.sct_item[I_BAR] -= n;
	    for (i = 0; i < MAXPRCON; i++) {
		ctype = pp->p_ctype[i];
		camt = pp->p_camt[i];
		if (!camt)
		    continue;
		if (CANT_HAPPEN(ctype <= I_NONE || ctype > I_MAX))
		    continue;
		sect.sct_item[ctype] += n * camt * grind_eff;
	    }
	    sect.sct_avail -= n * 5;
	    putsect(&sect);
	}
    }
    return RET_OK;
}
