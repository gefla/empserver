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
 *  control.c: Military control functions
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "var.h"
#include "file.h"
#include "path.h"
#include "xy.h"
#include "land.h"
#include "nsc.h"
#include "item.h"
#include "prototypes.h"

/*
 * Does the player->owner have military control of this sector?
 *
 */
int
military_control(struct sctstr *sp)
{
    int tot_mil = 0;
    struct nstr_item ni;
    struct lndstr land;

    if (sp->sct_oldown != sp->sct_own) {
	snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
	while (nxtitem(&ni, (s_char *)&land)) {
	    if (land.lnd_own == sp->sct_own)
		tot_mil += total_mil(&land);
	}
	if ((sp->sct_item[I_MILIT] + tot_mil) * 10 < sp->sct_item[I_CIVIL])
	    return 0;
    }

    return 1;
}
