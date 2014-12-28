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
 *  control.c: Military control functions
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include "file.h"
#include "land.h"
#include "nsc.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"

/*
 * Does the player->owner have military control of this sector?
 */
int
military_control(struct sctstr *sp)
{
    int tot_mil = 0;
    struct nstr_item ni;
    struct lndstr land;

    if (sp->sct_oldown != sp->sct_own) {
	snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
	while (nxtitem(&ni, &land)) {
	    if (land.lnd_own == sp->sct_own)
		tot_mil += land.lnd_item[I_MILIT];
	}
	if ((sp->sct_item[I_MILIT] + tot_mil) * 10 < sp->sct_item[I_CIVIL])
	    return 0;
    }

    return 1;
}

int
want_to_abandon(struct sctstr *sp, i_type vtype, int amnt, struct lndstr *lp)
{
    char prompt[80];

    /*
     * First, would we be abandoning it?  If not, just return that
     * it's ok to move out.
     */
    if (!would_abandon(sp, vtype, amnt, lp))
	return 1;

    sprintf(prompt, "Do you really want to abandon %s [yn]? ",
	    xyas(sp->sct_x, sp->sct_y, player->cnum));

    return askyn(prompt);
}

int
would_abandon(struct sctstr *sp, i_type vtype, int amnt, struct lndstr *lp)
{
    int mil, civs;

    if (vtype != I_CIVIL && vtype != I_MILIT)
	return 0;

    mil = sp->sct_item[I_MILIT];
    civs = sp->sct_item[I_CIVIL];

    if (vtype == I_MILIT)
	mil -= amnt;
    if (vtype == I_CIVIL)
	civs -= amnt;

    return sp->sct_own != 0 && civs <= 0 && mil <= 0
	&& !has_units(sp->sct_x, sp->sct_y, sp->sct_own, lp);
}
