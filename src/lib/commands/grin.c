/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  grin.c: Grind gold bars into dust
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "nat.h"
#include "nsc.h"
#include "file.h"
#include "product.h"
#include "commands.h"

int
grin(void)
{
    struct nstr_sect nstr;
    struct sctstr sect;
    char *p;
    int i, n, qty;
    int avail;
    char buf[1024];
    double grind_eff = 0.8;

    if ((p = getstarg(player->argp[1], "Sectors? ", buf)) == 0)
	return RET_SYN;
    if (!snxtsct(&nstr, p))
	return RET_SYN;
    if ((p = getstarg(player->argp[2], "amount :  ", buf)) == 0 || *p == 0)
	return RET_SYN;
    qty = atoi(p);
    if (qty < 0)
	return RET_SYN;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (sect.sct_effic < 60 || sect.sct_own != player->cnum)
	    continue;
	n = sect.sct_item[I_BAR] >= qty ? qty : sect.sct_item[I_BAR];
	/* work limit */
	avail = n * 5;
	if (avail > sect.sct_avail) {
	    n = sect.sct_avail / 5;
	    avail = sect.sct_avail;
	}
	/* space limit */
	for (i = 0; i < MAXPRCON; i++) {
	    if (!pchr[P_BAR].p_camt[i])
		continue;
	    if (CANT_HAPPEN(pchr[P_BAR].p_ctype[i] <= I_NONE ||
			    pchr[P_BAR].p_ctype[i] > I_MAX))
		continue;
	    n = MIN(n,
		    (double)(ITEM_MAX - sect.sct_item[pchr[P_BAR].p_ctype[i]])
		    / (pchr[P_BAR].p_camt[i] * grind_eff));
	}

	if (n > 0) {
	    pr("%d bars ground up in %s\n", n,
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	    sect.sct_item[I_BAR] -= n;
	    for (i = 0; i < MAXPRCON; i++) {
		if (!pchr[P_BAR].p_camt[i])
		    continue;
		if (CANT_HAPPEN(pchr[P_BAR].p_ctype[i] <= I_NONE ||
				pchr[P_BAR].p_ctype[i] > I_MAX))
		    continue;
		sect.sct_item[pchr[P_BAR].p_ctype[i]]
		    += n * pchr[P_BAR].p_camt[i] * grind_eff;
	    }
	    sect.sct_avail -= avail;
	    putsect(&sect);
	}
    }
    return RET_OK;
}
